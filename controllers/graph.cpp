#include "graph.hpp"
#include "path.cpp"


Graph::Graph(DataBase &db) : db(db) {}

Graph::~Graph() = default;

graphNode Graph::getNode(uint64_t Node) {
    return db.getNode(Node);
}

double Graph::getShading(graphNode Node1, graphNode Node2) {
    return getLength2(Node1, Node2);
}

graphShadingEdge Graph::getShadingEdge(uint64_t fineness, uint64_t Node1, uint64_t Node2) {
    graphShadingEdge gse = {1,
                            getShading(getNode(Node1), getNode(Node2)),
                            getLength2(getNode(Node1), getNode(Node2)),
                            Node2,
                            Node1};
    return gse;
}

std::vector<weightNode> Graph::getAdjacencyMatrix(uint64_t Node) {
    std::vector<weightNode> weightNodeArr;
    for (auto adjacentNode: adjacencyMatrix[Node]) {
        weightNodeArr.push_back({adjacentNode, 1});
    }

    return weightNodeArr;
}

double Graph::getLength2(graphNode Node1, graphNode Node2) {
    return 111200 * 180 / M_PI *
           acos(sin(Node1.x * M_PI / 180) * sin(Node2.x * M_PI / 180) +
                cos(Node1.x * M_PI / 180) * cos(Node2.x * M_PI / 180) *
                cos(Node2.y * M_PI / 180 - Node1.y * M_PI / 180));
}

double
Graph::getRemotenessWeight(uint64_t startNode, uint64_t endNode, uint64_t EdgeNode, double fineness) {
    // -O(exp(x^2))
    double ans = std::min(std::exp(getLength2(getNode(startNode), getNode(EdgeNode)) / fineness),
                          std::exp(getLength2(getNode(endNode), getNode(EdgeNode))) / fineness);
    //return std::min(ans, 100.0);
    return 1.0;
}

double Graph::getEdgeWeight(double shading, double length,
                            uint64_t startNode, uint64_t endNode, uint64_t EdgeNode, double fineness) {
    return (shading + 0.1 * length) * getRemotenessWeight(startNode, endNode, EdgeNode, fineness);
}


graphRoute Graph::getRoute(std::vector<std::string> &fromLocation, std::vector<std::string> &toLocation) {
    uint64_t startNode = db.closestNode(fromLocation);
    uint64_t endNode = db.closestNode(toLocation);

    adjacencyMatrix = db.getAdjacencyMatrixFull(startNode, endNode);
//    grid = Grid(db.buildings_receive())

    std::cout << "Making the route... ";

    minimumsSet minSet;
    usedSet usedSet;
    valueSet valueSet;
    graphRoute ans;
    minSet.update(startNode, 0, 0, 0);
    valueSet.update(startNode, 0, 0, 0);
    usedSet.update(0, 0);
    int cnt = 0;
    while (minSet.is_empty() == 0) {
        minimumsSet::minimumsSetElement el = minSet.getMinimum();

//        std::cout << el.nodeIndex << " node cnt:" << cnt << " length: " << el.key << std::endl;

        if (el.nodeIndex == endNode) {
            uint64_t node = endNode;
            ans.shading = el.value;
            while (node != startNode) {
                valueSet::valueSetElement route_el = valueSet.get(node);
                ans.Nodes.push_back(getNode(route_el.nodeIndex));
                node = route_el.prevNodeIndex;
            }
            break;
        }
        for (auto &e: getAdjacencyMatrix(el.nodeIndex)) {
            graphShadingEdge curEdge = getShadingEdge(e.fineness, el.nodeIndex, e.index);
            double new_length = el.key +
                                getEdgeWeight(curEdge.shading, curEdge.length, startNode, endNode, curEdge.node,
                                              trans_finesness[curEdge.fineness]);
            double new_shading = el.value + curEdge.shading;
            if (usedSet.get(curEdge.node) == 0 &&
                (minSet.get(curEdge.node).inf || minSet.get(curEdge.node).key > new_length)) {
                minSet.update(curEdge.node, new_length, e.index, new_shading);
                cnt++;
                valueSet.update(curEdge.node, new_shading, e.index, el.nodeIndex);
            }
        }
        minSet.erase(el.nodeIndex);
        cnt--;
        usedSet.update(el.nodeIndex, 1);
    }

    std::cout << "done: ";
    std::cout << "number of vertices: " << ans.Nodes.size() << std::endl;

    return ans;
}

//int pathTest() {
//    Graph plGraph;
//    uint64_t node1, node2;
//    std::cin >> node1 >> node2;
//    Graph::graphRoute route = plGraph.getRoute(node1, node2);
//    std::cout << route.shading << std::endl;
//    for (auto &e: route.Nodes) {
//        std::cout << e.x << e.y << " ";
//    }
//    std::cout << std::endl;
//
//    return 0;
//}
