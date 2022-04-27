#include "graph.hpp"
#include "path.cpp"

Graph::Graph() = default;

Graph::Graph(const std::string &path) : Graph() {
    database = DataBase(path);
};

Graph::~Graph() = default;

Graph::graphNode Graph::getNode(uint64_t Node) {
    return Nodes[Node];
}

double Graph::getShading(Graph::graphNode Node1, Graph::graphNode Node2) {
    return getLength2(Node1, Node2);
}

Graph::graphShadingEdge Graph::getShadingEdge(uint64_t fineness, uint64_t Node1, uint64_t Node2) {
    graphShadingEdge gse = {1,
                            getShading(getNode(Node1), getNode(Node2)),
                            getLength2(getNode(Node1), getNode(Node2)),
                            Node1,
                            Node2};
    return gse;
}

std::vector<Graph::weightNode> Graph::getAdjacencyMatrix(uint64_t Node) {
    return {};
}

double Graph::getLength2(Graph::graphNode Node1, Graph::graphNode Node2) {
    return 111200 * acos(sin(Node1.x) * sin(Node2.x) + cos(Node1.x) * cos(Node2.x) * cos(Node2.x - Node2.y));
}

double Graph::getRemotenessWeight(uint64_t startNode, uint64_t endNode, uint64_t EdgeNode, double fineness) {
    // -O(exp(x^2))
    return std::min(std::exp(getLength2(getNode(startNode), getNode(EdgeNode)) / fineness),
                    std::exp(getLength2(getNode(endNode), getNode(EdgeNode))) / fineness);
}

double Graph::getEdgeWeight(double shading, double length, uint64_t startNode, uint64_t endNode, uint64_t EdgeNode, double fineness) {
    return shading + 0.1 * length + getRemotenessWeight(startNode, endNode, EdgeNode, fineness);
}

Graph::graphRoute Graph::getRoute(uint64_t startNode, uint64_t endNode) {
    minimumsSet minSet;
    usedSet usedSet;
    valueSet valueSet;
    Graph::graphRoute ans;
    bool getans = false;
    minSet.update(startNode, 0, 0, 0);
    valueSet.update(startNode, 0, 0, 0);
    usedSet.update(0, 0);
    while (getans == 0 || minSet.is_empty() == 0) {
        minimumsSet::minimumsSetElement el = minSet.getMinimum();
        if (el.nodeIndex == endNode) {
            getans = true;
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
            Graph::graphShadingEdge curEdge = getShadingEdge(e.fineness, el.nodeIndex, e.index);
            double new_length = el.key +
                                getEdgeWeight(curEdge.shading, curEdge.length, startNode, endNode, curEdge.node,
                                              trans_finesness[curEdge.fineness]);
            double new_shading = el.value + curEdge.shading;
            if (usedSet.get(curEdge.node) == 0 &&
                (minSet.get(curEdge.node).inf || minSet.get(curEdge.node).key > new_length)) {
                minSet.update(curEdge.node, new_length, e.index, new_shading);
                valueSet.update(curEdge.node, new_shading, e.index, el.nodeIndex);
            }
        }
        minSet.erase(el.nodeIndex);
        usedSet.update(el.nodeIndex, 1);
    }
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