#ifndef EP_HH
#define EP_HH 1

#include <pthread.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <iostream>
#include <queue>

#include <set>
#include <queue>

#include "base-test.hh"
#include "locks.hh"

namespace kvtest {

    // Forward declaration for StoredValue
    class HashTable;

    class StoredValue {
    public:
        StoredValue() {
            next = NULL;
            value = NULL;
            dirty = false;
        }
        StoredValue(std::string &k, const char *v, StoredValue *n) {
            key = k;
            value = NULL;
            setValue(v);
            dirty = true;
            next = n;
        }
        ~StoredValue() {
            free((void*)value);
            value = NULL;
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
        bool isClean() {
            return !dirty;
        }
        const char* getValue() {
            return value;
        }
        void setValue(const char *v) {
            free((void*)value);
            value = strdup(v);
            markDirty();
        }
    private:

        friend class HashTable;

        bool dirty;
        std::string key;
        const char *value;
        StoredValue *next;
        DISALLOW_COPY_AND_ASSIGN(StoredValue);
    };

    typedef enum {
        NOT_FOUND, WAS_CLEAN, WAS_DIRTY
    } mutation_type_t;

    class HashTable {
    public:

        HashTable(size_t s = 196613) {
            size = s;
            active = true;
            values = (StoredValue**)calloc(s, sizeof(StoredValue**));
            mutexes = (pthread_mutex_t*)calloc(s, sizeof(pthread_mutex_t));
            for (int i = 0; i < (int)size; i++) {
                pthread_mutex_init(&mutexes[i], NULL);
            }
        }

        ~HashTable() {
            clear();
            for (int i = 0; i < (int)size; i++) {
                pthread_mutex_destroy(&mutexes[i]);
            }
            free(mutexes);
            free(values);
            mutexes = NULL;
            values = NULL;
            active = false;
        }

        void clear() {
            assert(active);
            for (int i = 0; i < (int)size; i++) {
                LockHolder lh(&mutexes[i]);
                while (values[i]) {
                    StoredValue *v = values[i];
                    values[i] = v->next;
                    delete v;
                }
            }
        }

        StoredValue *find(std::string &key) {
            assert(active);
            int bucket_num = bucket(key);
            LockHolder(getMutex(bucket_num));
            return unlocked_find(key, bucket_num);
        }

        // True if this existed and was clean
        mutation_type_t set(std::string &key, std::string &val) {
            return set(key, val.c_str());
        }

        mutation_type_t set(std::string &key, const char *val) {
            assert(active);
            mutation_type_t rv = NOT_FOUND;
            int bucket_num = bucket(key);
            LockHolder(getMutex(bucket_num));
            StoredValue *v = unlocked_find(key, bucket_num);
            if (v) {
                rv = v->isClean() ? WAS_CLEAN : WAS_DIRTY;
                v->setValue(val);
            } else {
                v = new StoredValue(key, val, values[bucket_num]);
                values[bucket_num] = v;
            }
            return rv;
        }

        StoredValue *unlocked_find(std::string &key, int bucket_num) {
            StoredValue *v = values[bucket_num];
            while (v) {
                if (key.compare(v->key) == 0) {
                    return v;
                }
                v = v->next;
            }
            return NULL;
        }

        inline int bucket(std::string &key) {
            assert(active);
            int h=5381;
            int i=0;
            const char *str=key.c_str();

            for(i=0; str[i] != 0x00; i++) {
                h = ((h << 5) + h) ^ str[i];
            }

            return abs(h) % (int)size;
        }

        // Get the mutex for a bucket (for doing your own lock management)
        inline pthread_mutex_t *getMutex(int bucket_num) {
            assert(active);
            assert(bucket_num < (int)size);
            assert(bucket_num >= 0);
            return &mutexes[bucket_num];
        }

        // True if it existed
        bool del(std::string &key) {
            assert(active);
            int bucket_num = bucket(key);
            LockHolder(getMutex(bucket_num));

            StoredValue *v = values[bucket_num];

            // Special case empty bucket.
            if (!v) {
                return false;
            }

            // Special case the first one
            if (key.compare(v->key) == 0) {
                values[bucket_num] = v->next;
                delete v;
                return true;
            }

            while (v->next) {
                if (key.compare(v->next->key) == 0) {
                    StoredValue *tmp = v->next;
                    v->next = v->next->next;
                    delete tmp;
                    return true;
                }
            }

            return false;
        }

    private:
        size_t            size;
        bool              active;
        StoredValue     **values;
        pthread_mutex_t  *mutexes;

        DISALLOW_COPY_AND_ASSIGN(HashTable);
    };

    // Forward declaration
    class Flusher;

    class EventuallyPersistentStore : public KVStore {
    public:

        EventuallyPersistentStore(KVStore *t, size_t est=32768);

        ~EventuallyPersistentStore();

        void set(std::string &key, std::string &val,
                 Callback<bool> &cb);

        void set(std::string &key, const char *val,
                 Callback<bool> &cb);

        void get(std::string &key, Callback<GetValue> &cb);

        void del(std::string &key, Callback<bool> &cb);

        void reset();

    private:

        void queueDirty(std::string &key);
        void flush(bool shouldWait);
        void flushSome(std::queue<std::string> *q, Callback<bool> &cb);
        void initQueue();

        friend class Flusher;

        KVStore                 *underlying;
        size_t                   est_size;
        Flusher                 *flusher;
        HashTable                storage;
        pthread_mutex_t          mutex;
        pthread_cond_t           cond;
        std::queue<std::string> *towrite;
        pthread_t                thread;
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
