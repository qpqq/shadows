#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstdio>
#include <cassert>
#include <algorithm>
#include <sstream>

#include "sqlite/sqlite3.h"

#define UNDEFINED_PATH  0 // empty string received
#define ABANDONED      1
#define BRIDLEWAY       2
#define BUS_STOP        0
#define CONSTRUCTION    0
#define CORRIDOR        2
#define CYCLEWAY        0
#define ELEVATOR        4
#define FOOTWAY         4
#define LIVING_STREET   3
#define MOTORWAY        1
#define PATH            2
#define PEDESTRIAN      3
#define PLATFORM        1
#define PRIMARY         0
#define PRIMARY_LINK    0
#define PROPOSED        0
#define RACEWAY         0
#define RESIDENTIAL     3
#define REST_AREA       1
#define ROAD            0
#define SECONDARY       5
#define SECONDARY_LINK  0
#define SERVICE         7
#define STEPS           7
#define STREET_LAMP     0
#define TERTIARY        6
#define TERTIARY_LINK   0
#define TRACK           1
#define TRUNK           6
#define TRUNK_LINK      0
#define UNCLASSIFIED    5
#define VIA_FERRATA     3

struct Node {
    unsigned long long int id;
    double lat;
    double lon;
    std::map<std::string, std::string> tags;
};

struct Way {
    unsigned long long int id;
    std::vector<Node> seq;
    std::map<std::string, std::string> tags;
};

struct Mate {
    unsigned long long int id;
    unsigned long long int prev;           // previous node in the array
    unsigned long long int next;           // next node in the array
    std::string path_type;      // way_tag for classifying roads
};

struct GraphShadingEdge {
    uint64_t fineness; // Крупность дороги, т.е. чем более крупная дорога тем больше эта величина
    double shading; // Затененность дороги выраженная в длине незатененной части
    double length; // Длина дороги
    uint64_t node; // Вершина конца ребра
    uint64_t prevNode; // Вершина начала ребра
};

struct GraphNode {
    double x;
    double y;
    friend bool operator== (const GraphNode& a, const GraphNode& b);

    friend bool operator!= (const GraphNode& a, const GraphNode& b);
};

struct GraphRoute {
    std::vector<GraphNode> Nodes;
    double shading;
};

struct WeightNode {
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
     * Single-argument constructors must be marked explicit to avoid unintentional implicit conversions
     */
    explicit DataBase(const std::string &path);

    /**
    * Default destructor
    */
    ~DataBase();

    const char *getPath();

    sqlite3 *getPointer();

    /**
     * Converts doubles to string with the specified precision.
     * @param x double
     * @param n number of digits after the decimal iPnt
     * @return string
     */
    static std::string toStringWithPrecision(double x, int n = 15);

    /**
     * Calculates the boundary coordinates
     * @param coords coordinate vector {latitude, longitude}
     * @param offset boundary offset. In radians
     * @return vector of border coordinates {lower, upper, left, right}
     */
    static std::vector<std::string> boundaries(const std::vector<std::vector<std::string>> &coords, double offset = 0);

    /**
     * Returns building data
     * @param coords string vector {latitude, longitude}
     * @param offset boundary offset. In radians
     * @return vector of ways
     */
    std::vector<Way>
    buildingsReceive(std::vector<std::string> &coords1, std::vector<std::string> &coords2, double offset);

    /**
     * Test for buildingsReceive
     */
    [[maybe_unused]] void buildingsReceiveTest();

    void neighboursReceive(const std::string &node_id, std::vector<Mate> &mates);

    int define_fine(const std::string &path_type);

    std::vector<WeightNode> getAdjacencyMatrix(uint64_t node);

    std::map<uint64_t, std::vector<uint64_t>>
    getAdjacencyMatrixFull(std::vector<std::string> &fromLocation, std::vector<std::string> &toLocation,
                           double offset = 0);

    Node nodeCoord(const std::string &node_id);

    GraphNode getNode(uint64_t node);

    uint64_t closestNode(const std::vector<std::string> &coords);
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


class Closest {

private:

    std::vector<Node> quickSelect(std::vector<Node> &points, int k);

    int partition(std::vector<Node> &points, int left, int right);

    Node &choosePivot(std::vector<Node> &points, int left, int right);

    double squaredDistance(Node &point);

public:

    Closest();

    ~Closest();

    std::vector<Node> kClosest(std::vector<Node> &points, int k);

    void shift(Node &zero, std::vector<Node> &point);
};
