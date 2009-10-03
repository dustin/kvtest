#ifndef TOKYO_BASE_H
#define TOKYO_BASE_H 1

#include "base-test.hh"

#include <stdlib.h>
#include <stdint.h>
#include <tcutil.h>
#include <tchdb.h>
#define ERRSTR_SIZE 64

namespace kvtest {

    /**
     * A Tokyo Cabinet DB store.
     */
    class TokyoStore : public KVStore {
    public:

        /**
         * Get a TokyoStore.
         *
         * @param p the path to the file holding the db
         * @param should_autocommit if true, sync after every set
         */
        TokyoStore(char const *p, bool should_autocommit=true);

        ~TokyoStore() {
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
        TCHDB *hdb;
        int ecode;
        const char *path;
        bool autocommit;

        void open();
        void close();
    };

}

#endif /* TOKYO_BASE_H */
