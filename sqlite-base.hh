#ifndef SQLITE_BASE_H
#define SQLITE_BASE_H 1

#include <sqlite3.h>

#include "base-test.hh"
#include "suite.hh"

namespace kvtest {

    /**
     * A sqlite prepared statement.
     */
    class PreparedStatement {
    public:

        /**
         * Construct a prepared statement.
         *
         * @param d the DB where the prepared statement will execute
         * @param query the query to prepare
         */
        PreparedStatement(sqlite3 *d, const char *query);

        /**
         * Clean up.
         */
        ~PreparedStatement();

        /**
         * Bind a string parameter to a binding in this statement.
         *
         * @param pos the binding position (starting at 1)
         * @param s the value to bind
         */
        void bind(int pos, const char *s);

        /**
         * Execute a prepared statement that does not return results.
         *
         * @return how many rows were affected
         */
        int execute();

        /**
         * Execute a prepared statement that does return results
         * and/or return the next row.
         *
         * @return true if there are more rows after this one
         */
        bool fetch();

        /**
         * Reset the bindings.
         *
         * Call this before reusing a prepared statement.
         */
        void reset();

        /**
         * Get the value at a given column in the current row.
         *
         * Use this along with fetch.
         *
         * @param x the column number (starting at 1)
         * @return the value
         */
        const char *column(int x);

    private:
        sqlite3      *db;
        sqlite3_stmt *st;
    };

    /**
     * The sqlite driver.
     */
    class Sqlite3 : public ThingUnderTest {
    public:

        /**
         * Construct an instance of sqlite with the given database name.
         */
        Sqlite3(const char *fn);

        /**
         * Cleanup.
         */
        ~Sqlite3();

        /**
         * Reset database to a clean state.
         */
        void reset();

        /**
         * Begin a transaction (if not already in one).
         */
        void begin();

        /**
         * Commit a transaction (unless not currently in one).
         */
        void commit();

        /**
         * Rollback a transaction (unless not currently in one).
         */
        void rollback();

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

    protected:

        /**
         * Shortcut to execute a simple query.
         *
         * @param query a simple query with no bindings to execute directly
         */
        void execute(const char *query);

    private:

        const char *filename;
        sqlite3 *db;
        bool intransaction;

        PreparedStatement *ins_stmt;
        PreparedStatement *sel_stmt;
        PreparedStatement *del_stmt;

        void open();
        void close();
        void initTables();
        void destroyTables();
    };

}

#endif /* SQLITE_BASE_H */
