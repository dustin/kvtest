#include "ep.hh"

namespace kvtest {

    EventuallyPersistentStore::EventuallyPersistentStore(KVStore *t) {
        underlying = t;
    }

    EventuallyPersistentStore::~EventuallyPersistentStore() {
        for (std::map<std::string, StoredValue*>::iterator it=storage.begin();
             it != storage.end(); it++ ) {
            delete (*it).second;
        }
    }

    void EventuallyPersistentStore::set(std::string &key, std::string &val,
                                        Callback<bool> &cb) {
        std::map<std::string, StoredValue*>::iterator it = storage.find(key);
        if (it != storage.end()) {
            delete it->second;
        }

        storage[key] = new StoredValue(val);
        bool rv = true;
        cb.callback(rv);
    }

    void EventuallyPersistentStore::get(std::string &key,
                                        Callback<kvtest::GetValue> &cb) {
        std::map<std::string, StoredValue*>::iterator it = storage.find(key);
        bool success = it != storage.end();
        kvtest::GetValue rv(success ? it->second->getValue() : std::string(":("),
                            success);
        cb.callback(rv);
    }

    void EventuallyPersistentStore::del(std::string &key, Callback<bool> &cb) {
        bool rv = true;
        if(storage.find(key) == storage.end()) {
            rv = false;
        }
        storage.erase(key);
        cb.callback(rv);
    }

}
