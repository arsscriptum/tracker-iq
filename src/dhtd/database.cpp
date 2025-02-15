
//==============================================================================
//
// database.cpp :  a simple C++ class that wraps SQLite functionality
//
//==============================================================================
//  Copyright (C) Guilaume Plante 2020 <cybercastor@icloud.com>
//==============================================================================



#include "stdafx.h"
#include "Database.h"

#ifndef NO_DATABASE_IMPL

#include <iostream>

Database::Database(const std::string &dbPath)
    : m_dbPath(dbPath), m_db(nullptr) {
}

Database::~Database() {
    disconnect();
}

bool Database::connect() {
    int rc = sqlite3_open(m_dbPath.c_str(), &m_db);
    if (rc != SQLITE_OK) {
        std::cerr << "Can't open database: " 
                  << (m_db ? sqlite3_errmsg(m_db) : "unknown error") << std::endl;
        if(m_db)
            sqlite3_close(m_db);
        m_db = nullptr;
        return false;
    }
    return true;
}

void Database::disconnect() {
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

bool Database::execute(const std::string &sql) {
    char *errMsg = nullptr;
    int rc = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (execute): " 
                  << (errMsg ? errMsg : "unknown error") << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Database::query(const std::string &sql,
                     std::function<int(int, char **, char **)> callback,
                     void *data) {
    // We need to wrap our std::function into a C callback pointer.
    struct CallbackData {
        std::function<int(int, char **, char **)> *cb;
    };

    CallbackData cbData { &callback };

    auto c_callback = [](void *cbData, int argc, char **argv, char **azColName) -> int {
        CallbackData *data = static_cast<CallbackData*>(cbData);
        return (*(data->cb))(argc, argv, azColName);
    };

    char *errMsg = nullptr;
    int rc = sqlite3_exec(m_db, sql.c_str(), c_callback, &cbData, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (query): " 
                  << (errMsg ? errMsg : "unknown error") << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

#endif