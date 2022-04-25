#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <stdio.h>

#include "sqlite/sqlite3.h"
#include "database.hpp"

struct Graph {
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

    static DataBase database;

    std::vector<graphShadingEdge> Edges;
    std::vector<graphNode> Nodes;
    std::vector<std::vector<weightNode>> adjacencyMatrix;

    graphShadingEdge getShadingEdge(uint64_t fineness, uint64_t Node1, uint64_t Node2);

    graphNode getNode(uint64_t Node); // --map
    double getShading(graphNode Node1, graphNode Node2); // -shading в метрах
    graphRoute getRoute(uint64_t startNode, uint64_t endNode); // ---output
    std::vector<weightNode> getAdjacencyMatrix(uint64_t Node); // -map
    double getLength2(graphNode Node1, graphNode Node2);

    Graph();

    ~Graph();
};
