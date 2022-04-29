#include "graph.hpp"
#include "path.cpp"

Graph::Graph() = default;

Graph::~Graph() = default;

graphNode Graph::getNode(DataBase &db, uint64_t Node) {
    return db.getNode(Node);
}

double Graph::getShading(graphNode Node1, graphNode Node2) {
    return getLength2(Node1, Node2);
}

graphShadingEdge Graph::getShadingEdge(DataBase &db, uint64_t fineness, uint64_t Node1, uint64_t Node2) {
    graphShadingEdge gse = {1,
                            getShading(getNode(db, Node1), getNode(db, Node2)),
                            getLength2(getNode(db, Node1), getNode(db, Node2)),
                            Node2,
                            Node1};
    return gse;
}

std::vector<weightNode> Graph::getAdjacencyMatrix(DataBase &db, uint64_t Node) {
    return db.getAdjacencyMatrix(Node);
}

double Graph::getLength2(graphNode Node1, graphNode Node2) {
    return 111200 * acos(sin(Node1.x) * sin(Node2.x) + cos(Node1.x) * cos(Node2.x) * cos(Node2.x - Node2.y));
}

double
Graph::getRemotenessWeight(DataBase &db, uint64_t startNode, uint64_t endNode, uint64_t EdgeNode, double fineness) {
    // -O(exp(x^2))
	double ans = std::min(std::exp(getLength2(getNode(startNode), getNode(EdgeNode)) / fineness), std::exp(getLength2(getNode(endNode), getNode(EdgeNode))) / fineness);
	//return std::min(ans, 100.0);
	return 1.0;
}

double Graph::getEdgeWeight(DataBase &db, double shading, double length, uint64_t startNode, uint64_t endNode,
                            uint64_t EdgeNode, double fineness) {
    return (shading + 0.1 * length )*getRemotennesWeight(db, startNode, endNode, EdgeNode, fineness);
}


graphRoute Graph::getRoute(DataBase &db, uint64_t startNode, uint64_t endNode) {
    minimumsSet minSet;
    usedSet usedSet;
    valueSet valueSet;
    graphRoute ans;
    bool getans = false;
    minSet.update(startNode, 0, 0, 0);
    valueSet.update(startNode, 0, 0, 0);
    usedSet.update(0, 0);
	int cnt = 0;
    while (minSet.is_empty() == 0) {
        minimumsSet::minimumsSetElement el = minSet.getMinimum();
	std::cout << el.nodeIndex << " node cnt:" << cnt << std::endl;
        if (el.nodeIndex == endNode) {
            getans = true;
            uint64_t node = endNode;
            ans.shading = el.value;
            while (node != startNode) {
                valueSet::valueSetElement route_el = valueSet.get(node);
                ans.Nodes.push_back(getNode(db, route_el.nodeIndex));
                node = route_el.prevNodeIndex;
            }
            break;
        }
        for (auto &e: getAdjacencyMatrix(db, el.nodeIndex)) {
            graphShadingEdge curEdge = getShadingEdge(db, e.fineness, el.nodeIndex, e.index);
            double new_length = el.key +
                                getEdgeWeight(db, curEdge.shading, curEdge.length, startNode, endNode, curEdge.node,
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
