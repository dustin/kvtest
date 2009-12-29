#ifndef EP_HH
#define EP_HH 1

#include <pthread.h>
#include <assert.h>
#include <stdbool.h>
#include <stdexcept>
#include <iostream>
#include <queue>

#include <map>
#include <set>
#include <queue>

#include "base-test.hh"

namespace kvtest {

    class StoredValue {
    public:
        StoredValue(std::string &v) {
            value = v;
            markClean();
        }
        void markDirty() {
            dirty = true;
        }
        void markClean() {
            dirty = false;
        }
        bool isDirty() {
            return dirty;
        }
        std::string getValue() {
            return value;
        }
    private:
        bool dirty;
        std::string value;
    };

    // Forward declaration
    class Flusher;

    class EventuallyPersistentStore : public KVStore {
    public:

        EventuallyPersistentStore(KVStore *t, size_t est=32768);

        ~EventuallyPersistentStore();

        void set(std::string &key, std::string &val,
                 Callback<bool> &cb);

        void get(std::string &key, Callback<GetValue> &cb);

        void del(std::string &key, Callback<bool> &cb);

        void reset();

    private:

        void markDirty(std::string &key);
        void flush(bool shouldWait);
        void flushSome(std::queue<std::string> *q, Callback<bool> &cb);
        void initQueue();

        friend class Flusher;

        KVStore                             *underlying;
        size_t                               est_size;
        Flusher                             *flusher;
        std::map<std::string, StoredValue*>  storage;
        pthread_mutex_t                      mutex;
        pthread_cond_t                       cond;
        std::queue<std::string>             *towrite;
        pthread_t                            thread;
        DISALLOW_COPY_AND_ASSIGN(EventuallyPersistentStore);
    };

    class Flusher {
    public:
        Flusher(EventuallyPersistentStore *st) {
            store = st;
            running = true;
        }
        ~Flusher() {
            stop();
        }
        void stop() {
            running = false;
        }
        void run() {
            try {
                while(running) {
                    store->flush(true);
                }
                std::cout << "Shutting down flusher." << std::endl;
            } catch(std::runtime_error &e) {
                std::cerr << "Exception in executor loop: "
                          << e.what() << std::endl;
                assert(false);
            }
        }
    private:
        EventuallyPersistentStore *store;
        volatile bool running;
    };

}

#endif /* EP_HH */
