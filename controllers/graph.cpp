#include "graph.hpp"

Graph::Graph(DataBase &db) : db(db) {}

Graph::~Graph() = default;

GraphNode Graph::getNode(uint64_t node) {
    if (nodes[node] != GraphNode()) {
        return nodes[node];
    }
    return nodes[node] = db.getNode(node);
}

double Graph::getShading(GraphNode node1, GraphNode node2) {
    return (1 - grid.shadowPerc(node1, node2)) * getLength2(node1, node2);
}

GraphShadingEdge Graph::getShadingEdge(uint64_t fineness, uint64_t node1, uint64_t node2) {
    GraphShadingEdge gse = {fineness,
                            getShading(getNode(node1), getNode(node2)),
                            getLength2(getNode(node1), getNode(node2)),
                            node2,
                            node1};
    return gse;
}

std::vector<WeightNode> Graph::getAdjacencyMatrix(uint64_t node) {
    std::vector<WeightNode> weightNodeArr;
    for (auto adjacentNode: adjacencyMatrix[node]) {
        weightNodeArr.push_back({adjacentNode, 1});
    }

    return weightNodeArr;
}

double Graph::getLength2(GraphNode node1, GraphNode node2) {
    return EarthRadius *
           acos(sin(node1.x * M_PI / 180) * sin(node2.x * M_PI / 180) +
                cos(node1.x * M_PI / 180) * cos(node2.x * M_PI / 180) *
                cos(node2.y * M_PI / 180 - node1.y * M_PI / 180));
}

double Graph::getRemotenessWeight(uint64_t startNode, uint64_t endNode, uint64_t edgeNode, double fineness) {
    // -O(exp(x^2))
    double ans = std::min(std::exp(getLength2(getNode(startNode), getNode(edgeNode)) / fineness),
                          std::exp(getLength2(getNode(endNode), getNode(edgeNode))) / fineness);
    return std::min(ans, MAX_REMOTENESS_FINE*1.0);
    //return 1.0;
}

double Graph::getEdgeWeight(double shading, double length,
                            uint64_t startNode, uint64_t endNode, uint64_t edgeNode, double fineness) {
    return (shading + 0.1 * length) * getRemotenessWeight(startNode, endNode, edgeNode, fineness);
}


GraphRoute
Graph::getRoute(std::vector<std::string> &fromLocation, std::vector<std::string> &toLocation) {
    uint64_t startNode = db.closestNode(fromLocation);
    uint64_t endNode = db.closestNode(toLocation);

    double offset = 5000;
    offset /= EarthRadius;
    adjacencyMatrix = db.getAdjacencyMatrixFull(fromLocation, toLocation, offset);
    grid = Grid({fromLocation, toLocation}, offset, db.buildingsReceive(fromLocation, toLocation, offset), 3);

    std::cout << "Making the route... ";
    std::cout.flush();

    minimumsSet minSet;
    usedSet usedSet;
    valueSet valueSet;
    GraphRoute ans;
    minSet.update(startNode, 0, 0, 0);
    valueSet.update(startNode, 0, 0, 0);
    usedSet.update(0, 0);
    int cnt = 0;
    while (minSet.is_empty() == 0) {
        minimumsSet::minimumsSetElement el = minSet.getMinimum();

//        std::cout << el.nodeIndex << " node cnt:" << cnt << " length: " << el.key << std::endl;

        if (el.nodeIndex == endNode) {
            uint64_t node = endNode;
//            ans.shading = el.value;
            while (node != startNode) {
                valueSet::valueSetElement route_el = valueSet.get(node);
                ans.Nodes.push_back(getNode(route_el.nodeIndex));
                node = route_el.prevNodeIndex;
            }
            break;
        }
        for (auto &e: getAdjacencyMatrix(el.nodeIndex)) {
            GraphShadingEdge curEdge = getShadingEdge(e.fineness, el.nodeIndex, e.index);
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
    std::cout << std::endl;

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
