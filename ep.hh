#ifndef EP_HH
#define EP_HH 1

#include <assert.h>
#include <stdbool.h>
#include <stdexcept>
#include <iostream>
#include <map>
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
        std::string getValue() {
            return value;
        }
    private:
        bool dirty;
        std::string value;
    };

    class EventuallyPersistentStore : public KVStore {
    public:

        EventuallyPersistentStore(KVStore *t);

        ~EventuallyPersistentStore();

        void set(std::string &key, std::string &val,
                 Callback<bool> &cb);

        void get(std::string &key, Callback<GetValue> &cb);

        void del(std::string &key, Callback<bool> &cb);

    private:
        KVStore *underlying;
        std::map<std::string, StoredValue*> storage;
        DISALLOW_COPY_AND_ASSIGN(EventuallyPersistentStore);
    };
}

#endif /* EP_HH */
