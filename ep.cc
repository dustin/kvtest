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

    EventuallyPersistentStore::EventuallyPersistentStore(KVStore *t) {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
        towrite = new std::set<std::string>;
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
        pthread_mutex_destroy(&mutex);
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
        towrite = new std::set<std::string>;
        storage.clear();
    }

    void EventuallyPersistentStore::get(std::string &key,
                                        Callback<kvtest::GetValue> &cb) {
        LockHolder lh(&mutex);
        std::map<std::string, std::string>::iterator it = storage.find(key);
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
        towrite->insert(key);
        if(pthread_cond_signal(&cond) != 0) {
            throw std::runtime_error("Error signaling change.");
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
            std::set<std::string> *q = towrite;
            towrite = new std::set<std::string>;
            lh.unlock();

            RememberingCallback<bool> cb;
            assert(underlying);
            underlying->begin();
            for (std::set<std::string>::iterator it = q->begin();
                 it != q->end(); it++) {
                std::string key = *it;
                flushOne(key, cb);
            }
            underlying->commit();
        }
    }

    void EventuallyPersistentStore::flushOne(std::string &key,
                                             Callback<bool> &cb) {
        LockHolder lh(&mutex);
        std::map<std::string, std::string>::iterator it = storage.find(key);
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
