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

    std::vector<graphShadingEdge> Edges;
    std::vector<graphNode> Nodes;
    std::vector<std::vector<weightNode>> adjacencyMatrix;

public:

    DataBase database;

    /**
    * Default constructor
    */
    Graph();

    /**
    * Constructor with a database path
    * @param path to database
    *
    * Single-argument constructors must be marked explicit to avoid unintentional implicit conversions
    */
    explicit Graph(const std::string &path);

    ~Graph();

    graphNode getNode(uint64_t Node); // -map

    double getShading(graphNode Node1, graphNode Node2); // -shading в метрах

    graphShadingEdge getShadingEdge(uint64_t fineness, uint64_t Node1, uint64_t Node2);

    std::vector<weightNode> getAdjacencyMatrix(uint64_t Node); // -map

    double getLength2(graphNode Node1, graphNode Node2);

    double getRemotenessWeight(uint64_t startNode, uint64_t endNode, uint64_t EdgeNode, double fineness);

    double getEdgeWeight(double shading, double length,
                         uint64_t startNode, uint64_t endNode, uint64_t EdgeNode, double fineness);

    graphRoute getRoute(uint64_t startNode, uint64_t endNode); // -output
};
