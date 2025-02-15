
//==============================================================================
//
// database.h :  a simple C++ class that wraps SQLite functionality
//
//==============================================================================
//  Copyright (C) Guilaume Plante 2020 <cybercastor@icloud.com>
//==============================================================================


#ifndef __SQLITE_DATABASE_H__
#define __SQLITE_DATABASE_H__

#ifndef NO_DATABASE_IMPL

#include <string>
#include <sqlite3.h>
#include <functional>



class Database {
public:
    // Constructor: takes the path to the database file
    explicit Database(const std::string &dbPath);

    // Destructor: disconnects from the database if needed
    ~Database();

    // Opens the connection. Returns true on success.
    bool connect();

    // Closes the connection
    void disconnect();

    // Executes a non-select SQL statement (INSERT, UPDATE, DELETE, etc.)
    // Returns true on success.
    bool execute(const std::string &sql);

    // Executes a SELECT query.
    // The 'callback' is called for each row returned by the query.
    // The callback signature is the same as for sqlite3_exec:
    //     int callback(void *data, int argc, char **argv, char **azColName)
    // 'data' is an optional pointer you can use to pass context to your callback.
    // Returns true on success.
    bool query(const std::string &sql,
               std::function<int(int, char **, char **)> callback,
               void *data = nullptr);

private:
    std::string m_dbPath;  // Path to the SQLite database file
    sqlite3 *m_db;         // SQLite database handle
};

#endif 

#endif//__SQLITE_DATABASE_H__