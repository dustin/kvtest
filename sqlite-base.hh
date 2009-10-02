#ifndef SQLITE_BASE_H
#define SQLITE_BASE_H 1

#include <sqlite3.h>

#include "base-test.hh"
#include "suite.hh"

namespace kvtest {

    class PreparedStatement {
    public:
        PreparedStatement(sqlite3 *d, const char *query);

        ~PreparedStatement();

        void bind(int pos, const char *s);

        int execute();

        bool fetch();

        void reset();

        const char *column(int x);

    private:
        sqlite3      *db;
        sqlite3_stmt *st;
    };

    class Sqlite3 : public ThingUnderTest {
    public:

        Sqlite3(const char *fn);

        ~Sqlite3();

        void reset();

        void begin();

        void commit();

        void rollback();

        void set(std::string &key, std::string &val, Callback<bool> &cb);

        void get(std::string &key, Callback<GetValue> &cb);

        void del(std::string &key, Callback<bool> &cb);

    protected:

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
