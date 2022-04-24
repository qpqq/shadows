#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sqlite3.h>
#include <stdio.h>
#include <assert.h>

class DataBase {

private:
    const char  *path;
    sqlite3     *db;

public:
    DataBase(std::string path);
    const char *get_path();
    sqlite3 *get_pointer();
    ~DataBase();
};

class Request {
    
private:
    const char *query;
    sqlite3_stmt *stmt;

public:
    Request(DataBase &database, std::string query);
    int step();
    void data(unsigned long long int &ret, int col_id);
    void data(double &ret, int col_id);
    void data(std::string &ret, int col_id);
    ~Request();
};
