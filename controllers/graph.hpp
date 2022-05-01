#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstdio>
#include <cmath>

#include "database.hpp"

class Graph {

private:

    DataBase db;

public:

    std::vector<graphShadingEdge> edges;
    std::vector<graphNode> nodes;
    std::map<uint64_t, std::vector<uint64_t>> adjacencyMatrix;

    /**
    * Default constructor
    */
    Graph();

    explicit Graph(DataBase &db_);

    ~Graph();

    graphNode getNode(uint64_t Node); // -map

    double getShading(graphNode Node1, graphNode Node2); // -shading в метрах

    graphShadingEdge getShadingEdge(uint64_t fineness, uint64_t Node1, uint64_t Node2);

    std::vector<weightNode> getAdjacencyMatrix(uint64_t Node); // -map

    double getLength2(graphNode Node1, graphNode Node2);

    double getRemotenessWeight(uint64_t startNode, uint64_t endNode, uint64_t EdgeNode, double fineness);

    double getEdgeWeight(double shading, double length,
                         uint64_t startNode, uint64_t endNode, uint64_t EdgeNode, double fineness);

    graphRoute getRoute(std::vector<std::string> &fromLocation, std::vector<std::string> &toLocation); // -output
};
