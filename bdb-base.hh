#ifndef BDB_BASE_H
#define BDB_BASE_H 1

#include "base-test.hh"

#include <db.h>

namespace kvtest {

    class BDBStore : public KVStore {
    public:

        BDBStore(char const *p, bool should_autocommit=true);

        ~BDBStore() {
            close();
        }

        /**
         * Overrides reset().
         */
        void reset();

        /**
         * Commit a transaction (unless not currently in one).
         */
        void commit();

        /**
         * Overrides set().
         */
        void set(std::string &key, std::string &val, Callback<bool> &cb);

        /**
         * Overrides get().
         */
        void get(std::string &key, Callback<GetValue> &cb);

        /**
         * Overrides del().
         */
        void del(std::string &key, Callback<bool> &cb);


    private:
        DB *db;
        const char *path;
        bool autocommit;

        void open();
        void close();
    };

}

#endif /* BDB_BASE_H */
