#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstdio>
#include <cassert>

#include "sqlite/sqlite3.h"

struct node {
    unsigned long long int                  id;
    double                                  lat;
    double                                  lon;
    std::map<std::string, std::string>      tags;
};

struct way {
    unsigned long long int                  id;
    std::vector<node>                       seq;
    std::map<std::string, std::string>      tags;
};

struct mate {
    unsigned long long int      prev;           // previous node in the array
    unsigned long long int      next;           // next node in the array
    std::string                 path_type;      // way_tag for classifying roads
};

class DataBase {

private:
    const char  *path{};
    sqlite3     *db{};

public:

    /**
     * Default constructor. Initializes private variables
     */
    DataBase();

    /**
     * Constructor with a database path
     * @param path to database
     *
     * Single-argument constructors must be marked explicit to avoid unintentional implicit conversions
     */
    explicit DataBase(const std::string &path);

    ~DataBase();

    const char *get_path();

    sqlite3 *get_pointer();

    /**
     * Returns building data
     * @param lat_low lower bound
     * @param lon_left left bound
     * @param lat_up upper bound
     * @param lon_right right bound
     * @return vector of ways
     */
    std::vector<way>
    buildings_receive(std::string &lat_low, std::string &lon_left, std::string &lat_up, std::string &lon_right);

    unsigned long long int closestNode(const std::string &lat, const std::string &lon);
};

class Request {

private:
    const char *query{};
    sqlite3_stmt *stmt{};

public:
    Request(DataBase &database, const std::string &query);
    int step();
    void data(unsigned long long int &ret, int col_id);
    void data(double &ret, int col_id);
    void data(std::string &ret, int col_id);
    ~Request();
};
