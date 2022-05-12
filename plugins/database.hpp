#pragma once

#include "drogon/lib/inc/drogon/plugins/Plugin.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cstdio>
#include <cassert>
#include <algorithm>

#include "sqlite/sqlite3.h"

#define UNDEFINED_PATH  0 // empty string received
#define ABANDONED       1
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


/////////////////////////////////////////////////////////////////////////////////////
#define BRIDGE          5
/////////////////////////////////////////////////////////////////////////////////////


/**
 * @brief The same node as in OSM database.
 * 
 */
struct Node {
    uint64_t id; // node id
    double lat;  // node latitude
    double lon;  // node longitude
    std::map<std::string, std::string> tags;  // node tags
};


/**
 * @brief The same way as in OSM database.
 * 
 */
struct Way {
    uint64_t id; // way id
    std::vector<Node> seq;  // sequence of nodes
    std::map<std::string, std::string> tags;  // way tags
};

struct GraphNode {
    uint64_t id{};
    double x{};
    double y{};
    int fineness{};

    GraphNode() = default;

    explicit GraphNode(int id) { this->id = id; };

    friend bool operator==(const GraphNode &a, const GraphNode &b) { return a.id == b.id; };

    friend bool operator!=(const GraphNode &a, const GraphNode &b) { return !(a == b); };

    friend bool operator<(const GraphNode &a, const GraphNode &b) { return a.id < b.id; };
};

struct GraphShadingEdge {
    uint64_t fineness{}; // Крупность дороги, т.е. чем более крупная дорога тем больше эта величина
    double shading{}; // Затененность дороги выраженная в длине незатененной части
    double length{}; // Длина дороги
    GraphNode node; // Вершина конца ребра
    GraphNode prevNode; // Вершина начала ребра
};

struct GraphRoute {
    std::vector<GraphNode> nodes;
    std::vector<double> shading;
};

/**
 * @brief Used to build AdjacencyMatrix.
 * 
 */
struct Mate {
    GraphNode curr;
    uint64_t prev; // previous node in the array
    uint64_t next; // next node in the array
    std::string path_type; // way_tag for classifying roads
};

class DataBase : public drogon::Plugin<DataBase> {

private:

    const char *path{};
    sqlite3 *db{};

    std::map<uint64_t, GraphNode> nodes;
    std::map<GraphNode, std::set<GraphNode>> adjacencyMatrix;
    std::vector<Way> buildings;

    /**
     * @brief Road's fines dictionary. Used in AdjacencyMatrix.
     * 
     */
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
            {"",               UNDEFINED_PATH},
            {"bridge",         BRIDGE}
    };

public:

    /**
     * Default constructor. Initializes private variables
     */
    DataBase();

    /**
     * Default destructor
     */
    ~DataBase() override;

    /**
     * @brief Opens a database connection.
     * 
     */
    void open();

    void initNodes();

    void initAdjacencyMatrix();

    void initBuildings();

    /// This method must be called by drogon to initialize and start the plugin.
    /// It must be implemented by the user.
    void initAndStart(const Json::Value &config) override;

    /// This method must be called by drogon to shutdown the plugin.
    /// It must be implemented by the user.
    void shutdown() override;

    /**
     * @brief Gets the path to the database.
     * 
     * @return path to the database 
     */
    const char *getPath();

    /**
     * @brief Gets the pointer to the database.
     * 
     * @return pointer to the database 
     */
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

    /**
     * @brief Builds the AdjacencyMatrix by the coordinates.
     * 
     * @param coords1 string vector {latitude, longitude} of the first point
     * @param coords2 string vector {latitude, longitude} of the second point
     * @param offset boundary offset. In radians
     * @return AdjacancyMatrix
     */
    std::map<GraphNode, std::set<GraphNode>>
    getAdjacencyMatrix(std::vector<std::string> &coords1, std::vector<std::string> &coords2,
                       double offset = 0);
    
    Node nodeCoord(const std::string &node_id);

    /**
     * @brief Returns node's coordinates.
     * 
     * @param node input node
     * @return node's coordinates by the structure GraphNode.
     */
    GraphNode getNode(uint64_t node);

    /**
     * @brief Returns the closestNode that has been found by coordinates.
     * 
     * @param coords string vector {latitude, longitude}
     * @return information about node by the structure GraphNode.
     */
    GraphNode closestNode(const std::vector<std::string> &coords);
};


class Request {

private:
    const char *query{};
    sqlite3_stmt *stmt{};

public:

    /**
     * @brief Prepares SQL statement.
     * 
     * @param database DataBase object
     * @param query query to be executed
     * @details See sqlite3_prepare_v2 for more information.
     */
    Request(DataBase &database, const std::string &query);

    /**
     * @brief Makes a step.
     * @details See sqlite3_step for more information.
     * 
     * @return error code 
     */
    int step();

    /**
     * @brief Receives data from col_id and stores it in ret.
     * 
     * @param ret link to parameter to be returned
     * @param col_id column number to be read
     */
    void data(uint64_t &ret, int col_id);

    /**
     * @brief Receives data from col_id and stores it in ret.
     * 
     * @param ret link to parameter to be returned
     * @param col_id column number to be read
     */
    void data(double &ret, int col_id);

    /**
     * @brief Receives data from col_id and stores it in ret.
     * 
     * @param ret link to parameter to be returned
     * @param col_id column number to be read
     */
    void data(std::string &ret, int col_id);

    ~Request();
};

/**
 * @brief Fast searching for the closest node. Based on binary search.
 * 
 */
class Closest {

private:

    GraphNode zero;

    /**
     * @brief Selects k closest nodes from points vector.
     * 
     * @param points vector of nodes
     * @param k a number of nodes to be searched for
     * @return k closest nodes
     */
    std::vector<GraphNode> quickSelect(std::vector<GraphNode> &points, int k);

    /**
     * @brief Standard patrition function.
     * 
     * @param points vector of nodes
     * @param left left border of the vector
     * @param right right border of the vector
     * @return new left border of the vector 
     */
    int partition(std::vector<GraphNode> &points, int left, int right);

    GraphNode &choosePivot(std::vector<GraphNode> &points, int left, int right);

    double squaredDistance(GraphNode &point);

public:

    Closest();

    Closest(GraphNode zero);

    ~Closest();

    std::vector<GraphNode> kClosest(std::vector<GraphNode> &points, int k);

    void shift(std::vector<GraphNode> &point);

    void antiShift(std::vector<GraphNode> &point);
};
