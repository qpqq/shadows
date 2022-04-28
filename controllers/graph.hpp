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

public:

    std::vector<graphShadingEdge> Edges;
    std::vector<graphNode> Nodes;
    std::vector<std::vector<weightNode>> adjacencyMatrix;

    /**
    * Default constructor
    */
    Graph();

    ~Graph();

    graphNode getNode(DataBase &db, uint64_t Node); // -map

    double getShading(graphNode Node1, graphNode Node2); // -shading в метрах

    graphShadingEdge getShadingEdge(DataBase &db, uint64_t fineness, uint64_t Node1, uint64_t Node2);

    std::vector<weightNode> getAdjacencyMatrix(DataBase &db, uint64_t Node); // -map

    double getLength2(graphNode Node1, graphNode Node2);

    double getRemotenessWeight(DataBase &db, uint64_t startNode, uint64_t endNode, uint64_t EdgeNode, double fineness);

    double getEdgeWeight(DataBase &db, double shading, double length, uint64_t startNode, uint64_t endNode,
                         uint64_t EdgeNode, double fineness);

    graphRoute getRoute(DataBase &db, uint64_t startNode, uint64_t endNode); // -output
};
