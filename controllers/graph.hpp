#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstdio>
#include <cmath>

#include "path.cpp"
#include "database.hpp"
#include "grid.hpp"

class Graph {

private:

    DataBase &db;
    Grid grid;

    std::vector<GraphShadingEdge> edges;
    std::vector<GraphNode> nodes;
    std::map<uint64_t, std::vector<uint64_t>> adjacencyMatrix;

    double EarthRadius = 6378100;

public:

    explicit Graph(DataBase &db);

    ~Graph();

    GraphNode getNode(uint64_t node); // -map

    double getShading(GraphNode Node1, GraphNode Node2); // -shading в метрах

    GraphShadingEdge getShadingEdge(uint64_t fineness, uint64_t node1, uint64_t node2);

    std::vector<WeightNode> getAdjacencyMatrix(uint64_t node); // -map

    double getLength2(GraphNode node1, GraphNode node2);

    double getRemotenessWeight(uint64_t startNode, uint64_t endNode, uint64_t edgeNode, double fineness);

    double getEdgeWeight(double shading, double length,
                         uint64_t startNode, uint64_t endNode, uint64_t edgeNode, double fineness);

    GraphRoute getRoute(std::vector<std::string> &fromLocation, std::vector<std::string> &toLocation); // -output
};
