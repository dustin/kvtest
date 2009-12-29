#include "ep.hh"
#include "locks.hh"

namespace kvtest {

    static void* launch_flusher_thread(void* arg) {
        Flusher *flusher = (Flusher*) arg;
        try {
            flusher->run();
        } catch(...) {
            std::cerr << "Caught a fatal exception in the thread" << std::endl;
        }
        return NULL;
    }

    EventuallyPersistentStore::EventuallyPersistentStore(KVStore *t,
                                                         size_t est) : storage(est) {

        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
        est_size = est;
        initQueue();
        flusher = new Flusher(this);

        // Run in a thread...
        if(pthread_create(&thread, NULL, launch_flusher_thread, flusher)
           != 0) {
            throw std::runtime_error("Error initializing queue thread");
        }

        underlying = t;
        assert(underlying);
    }

    EventuallyPersistentStore::~EventuallyPersistentStore() {
        LockHolder lh(&mutex);
        flusher->stop();
        if(pthread_cond_signal(&cond) != 0) {
            throw std::runtime_error("Error signaling change.");
        }
        lh.unlock();
        pthread_join(thread, NULL);
        delete flusher;
        delete towrite;
        delete towrite_q;
        pthread_mutex_destroy(&mutex);
    }

    void EventuallyPersistentStore::initQueue() {
        towrite = new google::sparse_hash_set<std::string>(est_size);
        towrite_q = new std::queue<std::string>;
    }

    void EventuallyPersistentStore::set(std::string &key, std::string &val,
                                        Callback<bool> &cb) {
        LockHolder lh(&mutex);

        storage[key] = val;
        bool rv = true;
        markDirty(key);
        lh.unlock();
        cb.callback(rv);
    }

    void EventuallyPersistentStore::reset() {
        flush(false);
        LockHolder lh(&mutex);
        underlying->reset();
        delete towrite;
        delete towrite_q;
        initQueue();
        storage.clear();
    }

    void EventuallyPersistentStore::get(std::string &key,
                                        Callback<kvtest::GetValue> &cb) {
        LockHolder lh(&mutex);
        google::sparse_hash_map<std::string, std::string>::iterator it = storage.find(key);
        bool success = it != storage.end();
        kvtest::GetValue rv(success ? it->second : std::string(":("),
                            success);
        lh.unlock();
        cb.callback(rv);
    }

    void EventuallyPersistentStore::del(std::string &key, Callback<bool> &cb) {
        bool rv = true;
        LockHolder lh(&mutex);
        if(storage.find(key) == storage.end()) {
            rv = false;
        } else {
            markDirty(key);
            storage.erase(key);
        }
        lh.unlock();
        cb.callback(rv);
    }

    void EventuallyPersistentStore::markDirty(std::string &key) {
        // Assumed locked
        if (towrite->find(key) == towrite->end()) {
            // This is only called for missing keys.
            towrite->insert(key);
            towrite_q->push(key);
            if(pthread_cond_signal(&cond) != 0) {
                throw std::runtime_error("Error signaling change.");
            }
        }
    }

    void EventuallyPersistentStore::flush(bool shouldWait) {
        LockHolder lh(&mutex);

        if (towrite->empty()) {
            if (shouldWait) {
                if(pthread_cond_wait(&cond, &mutex) != 0) {
                    throw std::runtime_error("Error waiting for signal.");
                }
            }
        } else {
            std::queue<std::string> *q = towrite_q;
            google::sparse_hash_set<std::string> *s = towrite;
            initQueue();
            lh.unlock();

            RememberingCallback<bool> cb;
            assert(underlying);
            underlying->begin();
            while (!q->empty()) {
                std::string key = q->front();
                flushOne(key, cb);
                q->pop();
            }
            underlying->commit();

            delete q;
            delete s;
        }
    }

    void EventuallyPersistentStore::flushOne(std::string &key,
                                             Callback<bool> &cb) {
        LockHolder lh(&mutex);
        google::sparse_hash_map<std::string, std::string>::iterator it = storage.find(key);
        bool found = it != storage.end();
        std::string val = it->second;
        lh.unlock();

        if (found) {
            // should set
            underlying->set(key, val, cb);
        } else {
            // Should delete
            underlying->del(key, cb);
        }
    }

}
