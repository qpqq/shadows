#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstdio>
#include <cassert>

#include "sqlite/sqlite3.h"

#define UNDEFINED_PATH  120 // empty string received
#define ABANDONED       1
#define BRIDLEWAY       2
#define BUS_STOP        3
#define CONSTRUCTION    4
#define CORRIDOR        5
#define CYCLEWAY        6
#define ELEVATOR        7
#define FOOTWAY         8
#define LIVING_STREET   9
#define MOTORWAY        10
#define PATH            11
#define PEDESTRIAN      12
#define PLATFORM        13
#define PRIMARY         14
#define PRIMARY_LINK    15
#define PROPOSED        16
#define RACEWAY         17
#define RESIDENTIAL     18
#define REST_AREA       19
#define ROAD            20
#define SECONDARY       21
#define SECONDARY_LINK  22
#define SERVICE         23
#define STEPS           24
#define STREET_LAMP     25
#define TERTIARY        26
#define TERTIARY_LINK   27
#define TRACK           28
#define TRUNK           29
#define TRUNK_LINK      30
#define UNCLASSIFIED    31
#define VIA_FERRATA     32

struct node {
    unsigned long long int id;
    double lat;
    double lon;
    std::map<std::string, std::string> tags;
};

struct way {
    unsigned long long int id;
    std::vector<node> seq;
    std::map<std::string, std::string> tags;
};

struct mate {
    unsigned long long int prev;           // previous node in the array
    unsigned long long int next;           // next node in the array
    std::string path_type;      // way_tag for classifying roads
};

struct mate_matrix {
    unsigned long long int id;
    unsigned long long int prev;           // previous node in the array
    unsigned long long int next;           // next node in the array
};

struct graphShadingEdge {
    uint64_t fineness; // Крупность дороги, т.е. чем более крупная дорога тем больше эта величина
    double shading; // Затененность дороги выраженная в длине незатененной части
    double length; // Длина дороги
    uint64_t node; // Вершина конца ребра
    uint64_t prevNode; // Вершина начала ребра
};

struct graphNode {
    double x;
    double y;
};

struct graphRoute {
    std::vector<graphNode> Nodes;
    double shading;
};

struct weightNode {
    uint64_t index;
    int fineness;
};

class DataBase {

private:
    const char *path{};
    sqlite3 *db{};

    std::map<std::string, int> price_list = {
            {"abandoned",      ABANDONED},
            {"bridleway",      BRIDLEWAY},
            {"bus_stop",       BUS_STOP},
            {"construction",   CONSTRUCTION},
            {"corridor",       CORRIDOR},
            {"cycleway",       CYCLEWAY},
            {"elevator",       ELEVATOR},
            {"footway",        FOOTWAY},
            {"living_street",  LIVING_STREET},
            {"motorway",       MOTORWAY},
            {"path",           PATH},
            {"pedestrian",     PEDESTRIAN},
            {"platform",       PLATFORM},
            {"primary",        PRIMARY},
            {"primary_link",   PRIMARY_LINK},
            {"proposed",       PROPOSED},
            {"raceway",        RACEWAY},
            {"residential",    RESIDENTIAL},
            {"rest_area",      REST_AREA},
            {"road",           ROAD},
            {"secondary",      SECONDARY},
            {"secondary_link", SECONDARY_LINK},
            {"service",        SERVICE},
            {"steps",          STEPS},
            {"street_lamp",    STREET_LAMP},
            {"tertiary",       TERTIARY},
            {"tertiary_link",  TERTIARY_LINK},
            {"track",          TRACK},
            {"trunk",          TRUNK},
            {"trunk_link",     TRUNK_LINK},
            {"unclassified",   UNCLASSIFIED},
            {"via_ferrata",    VIA_FERRATA},
            {"",               UNDEFINED_PATH}
    };

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

    void buildings_receive_test();

    void neighbours_receive(const std::string &node_id, std::vector<mate> &mates);

    int define_fine(const std::string &path_type);

    std::vector<weightNode> getAdjacencyMatrix(uint64_t Node);

    std::map<uint64_t, std::vector<uint64_t>>
    getAdjacencyMatrixFull(uint64_t startNode, uint64_t endNode);

    void node_coord(const std::string &node_id, node &ret);

    graphNode getNode(uint64_t Node);

    unsigned long long int closestNode(const std::vector<std::string> &coords);
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
