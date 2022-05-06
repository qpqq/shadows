#include "graph.hpp"

Graph::Graph(DataBase &db) : db(db) {}

Graph::~Graph() = default;

double Graph::getShading(GraphNode &Node1, GraphNode &Node2) {
    return (1 - grid.shadowPerc(Node1, Node2)) * getLength2(Node1, Node2);
}

GraphShadingEdge Graph::getShadingEdge(uint64_t fineness, GraphNode &node1, GraphNode &node2) {
    GraphShadingEdge gse = {fineness,
                            getShading(node1, node2),
                            getLength2(node1, node2),
                            node2,
                            node1};
    return gse;
}

std::vector<GraphNode> Graph::getAdjacencyMatrix(GraphNode &node) {
    std::vector<GraphNode> graphNodeArr;
    for (auto adjacentNode: adjacencyMatrix[node]) {
        graphNodeArr.push_back(adjacentNode);
    }

    return graphNodeArr;
}

double Graph::getLength2(GraphNode &node1, GraphNode &node2) {
    return EarthRadius *
           acos(sin(node1.x * M_PI / 180) * sin(node2.x * M_PI / 180) +
                cos(node1.x * M_PI / 180) * cos(node2.x * M_PI / 180) *
                cos(node2.y * M_PI / 180 - node1.y * M_PI / 180));
}

double Graph::getRemotenessWeight(GraphNode &startNode, GraphNode &endNode, GraphNode &edgeNode, double fineness) {
    // -O(exp(x^2))
    double ans = std::min(std::exp(getLength2(startNode, edgeNode) / fineness),
                          std::exp(getLength2(endNode, edgeNode)) / fineness);
    return std::min(ans, MAX_REMOTENESS_FINE * 1.0);
    //return 1.0;
}

double Graph::getEdgeWeight(double shading, double length,
                            GraphNode &startNode, GraphNode &endNode, GraphNode &edgeNode, double fineness) {
    return (shading + 0.1 * length) * getRemotenessWeight(startNode, endNode, edgeNode, fineness);
}


GraphRoute
Graph::getRoute(std::vector<std::string> &fromLocation, std::vector<std::string> &toLocation) {
    GraphNode startNode = db.closestNode(fromLocation);
    GraphNode endNode = db.closestNode(toLocation);

    double offset = 1000;
    offset *= 180 / M_PI / EarthRadius;
    adjacencyMatrix = db.getAdjacencyMatrixFull(fromLocation, toLocation, offset);
    grid = Grid({fromLocation, toLocation}, offset, db.buildingsReceive(fromLocation, toLocation, offset), 1.5);
//    grid.print_grid();

    std::cout << "Making the route... ";
    std::cout.flush();

    minimumsSet minSet;
    usedSet usedSet;
    valueSet valueSet;
    GraphRoute ans;
    minSet.update(startNode, 0, GraphNode(0), 0);
    valueSet.update(startNode, 0, GraphNode(0), GraphNode(0));
    usedSet.update(GraphNode(0), 0);
    int cnt = 0;
    while (minSet.is_empty() == 0) {
        minimumsSet::minimumsSetElement el = minSet.getMinimum();

//        std::cout << el.nodeIndex << " node cnt:" << cnt << " length: " << el.key << std::endl;

        if (el.nodeIndex == endNode) {
            GraphNode node = endNode;

            while (node != startNode) {
                valueSet::valueSetElement route_el = valueSet.get(node);
                ans.nodes.push_back(route_el.nodeIndex);
                node = route_el.prevNodeIndex;
            }

            for (int i = 1; i < ans.nodes.size(); i++) {
                ans.shading.push_back(1 - grid.shadowPerc(ans.nodes[i - 1], ans.nodes[i]));
            }

            break;
        }

        for (auto &e: getAdjacencyMatrix(el.nodeIndex)) {

            GraphShadingEdge curEdge = getShadingEdge(e.fineness, el.nodeIndex, e);
            double new_length = el.key +
                                getEdgeWeight(curEdge.shading, curEdge.length, startNode, endNode, curEdge.node,
                                              trans_finesness[curEdge.fineness]);
            double new_shading = el.value + curEdge.shading;

            if (usedSet.get(curEdge.node) == 0 &&
                (minSet.get(curEdge.node).inf || minSet.get(curEdge.node).key > new_length)) {
                minSet.update(curEdge.node, new_length, e, new_shading);
                cnt++;
                valueSet.update(curEdge.node, new_shading, e, el.nodeIndex);
            }
        }
        minSet.erase(el.nodeIndex);
        cnt--;
        usedSet.update(el.nodeIndex, 1);
    }

    std::cout << "done: ";
    std::cout << "number of vertices: " << ans.nodes.size() << std::endl;

    return ans;
}
