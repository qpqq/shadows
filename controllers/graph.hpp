#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <cstdio>
#include <cmath>

#include "path.cpp"
#include "database.hpp"
#include "grid.hpp"

#define MAX_REMOTENESS_FINE 1000

class Graph {

private:

    DataBase &db;
    Grid grid;

    std::vector<GraphShadingEdge> edges;
    std::map<GraphNode, GraphNode> nodes;
    std::map<GraphNode, std::set<GraphNode>> adjacencyMatrix;

    double EarthRadius = 6378100;

public:

    explicit Graph(DataBase &db);

    ~Graph();

    double getShading(GraphNode &Node1, GraphNode &Node2); // -shading в метрах

    GraphShadingEdge getShadingEdge(uint64_t fineness, GraphNode &node1, GraphNode &node2);

    std::vector<GraphNode> getAdjacencyMatrix(GraphNode &node); // -map

    double getLength2(GraphNode &node1, GraphNode &node2);

    double getRemotenessWeight(GraphNode &startNode, GraphNode &endNode, GraphNode &edgeNode, double fineness);

    double getEdgeWeight(double shading, double length,
                         GraphNode &startNode, GraphNode &endNode, GraphNode &edgeNode, double fineness);

    GraphRoute getRoute(std::vector<std::string> &fromLocation, std::vector<std::string> &toLocation); // -output
};
