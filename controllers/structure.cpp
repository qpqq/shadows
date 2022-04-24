#include "structure.hpp"

void buildings_receive(DataBase &database, std::string lat_low, std::string lon_left, std::string lat_up, std::string lon_right, std::vector<way> &build) {

    std::string query, query_tags, query_nodes, pattern_tags, pattern_nodes;
    std::string between, tag_key, tag_val;
    way mid_way;
    node mid_node;
    unsigned int i, counter = 0;

    between = "(lat BETWEEN " + lat_low + " AND " + lat_up + ")" + " AND (lon BETWEEN " + lon_left + " AND " + lon_right + ")";

    query = "SELECT way_id "
            "FROM WayBuildings "
            "WHERE tag_key = 'building' AND " + between + " GROUP BY way_id;";

    pattern_tags = "SELECT way_tags.tag_key, way_tags.tag_val "
                   "FROM way_tags "
                   "WHERE way_tags.tag_key = 'building:levels' AND way_tags.way_id = ";
    
    pattern_nodes = "SELECT nodes.node_id, nodes.lat, nodes.lon "
                    "FROM nodes "
                    "JOIN ways ON ways.node_id = nodes.node_id "
                    "WHERE ways.way_id = ";

    Request req_ways(database, query);

    std::cout << "selecting ways by coordinates: ";

	while(req_ways.step() != SQLITE_DONE) {
        req_ways.data(mid_way.id, 0);
        build.push_back(mid_way);
        counter++;
	}

    std::cout << "total " + std::to_string(counter - 1) << std::endl;

    counter = 0;

    std::cout << "selecting building:levels by way.id: ";

    for(i = 0; i < build.size(); ++i) {
        query_tags  = pattern_tags  + std::to_string(build[i].id) + ";";
        query_nodes = pattern_nodes + std::to_string(build[i].id) + ";";

        Request req_tags(database, query_tags);
        Request req_nodes(database, query_nodes);

        while (req_tags.step() != SQLITE_DONE) {
            req_tags.data(tag_key, 0);
            req_tags.data(tag_val, 1);
            build[i].tags.insert(std::pair<std::string, std::string>(tag_key, tag_val));
        }

        while(req_nodes.step() != SQLITE_DONE) {
            req_nodes.data(mid_node.id, 0);
            req_nodes.data(mid_node.lat, 1);
            req_nodes.data(mid_node.lon, 2);
            build[i].seq.push_back(mid_node);
            counter++;
        }
    }

    std::cout << "total " + std::to_string(i - 1) << std::endl;
    std::cout << "total nodes " + std::to_string(counter - 1) << std::endl;

}

void neighbours_receive(DataBase &database, std::string node_id, std::vector<mate> &mates) {

    std::string query, withas, mid_select, query_tag;
    std::vector<std::string> ways;
    std::string mid_way;
    unsigned int i;
    mate mid_mate;

    query = "SELECT way_id FROM ways WHERE node_id = " + node_id + ";";

    Request req_ways(database, query);
    
    while(req_ways.step() != SQLITE_DONE) {
        req_ways.data(mid_way, 0);
        ways.push_back(mid_way);
	}

    for(i = 0; i < ways.size(); ++i) {
        mid_mate.prev = 0;
        mid_mate.next = 0;
        mid_mate.path_type = "";

        withas = "WITH mid AS ( "
                 "SELECT node_id, " 
                 "LAG(node_id, 1, 0) OVER (ORDER BY seq_id) pv, "
                 "LEAD(node_id, 1, 0) OVER (ORDER BY seq_id) nt "
                 "FROM ways "
                 "WHERE way_id = " + ways[i] + ") ";
                    
        mid_select = "SELECT pv, nt "
                     "FROM mid "
                     "WHERE node_id = " + node_id + ";";

        query = withas + mid_select;

        query_tag = "SELECT tag_val "
                    "FROM way_tags "
                    "WHERE tag_key = 'highway' AND way_id = " + ways[i] + ";";

        Request req_tag(database, query_tag);
        Request req_neighbour(database, query);
        
        while(req_tag.step() != SQLITE_DONE) {
            req_tag.data(mid_mate.path_type, 0);
        }
        
        while(req_neighbour.step() != SQLITE_DONE) {
            req_neighbour.data(mid_mate.prev, 0);
            req_neighbour.data(mid_mate.next, 1);
            mates.push_back(mid_mate);
        }   
    }
}

int define_fine(std::string path_type) {
    if(price_list.count(path_type) > 0) {
        return price_list[path_type];
    }
    return 0;
}

std::vector<graph::weightNode> getadjacencyMatrix(uint64_t Node) {

    std::vector<graph::weightNode> nodes;
    std::string node_id = std::to_string(Node);
    std::vector<mate> mates;
    graph::weightNode mid_node;
    int i, fine;

    neighbours_receive(graph::database, node_id, mates);

    for(i = 0; i < mates.size(); ++i) {
        fine = define_fine(mates[i].path_type);
        if(mates[i].prev != 0) {
            mid_node.index = mates[i].prev;
            mid_node.fineness = fine;
            nodes.push_back(mid_node);
        }
        if(mates[i].next != 0) {
            mid_node.index = mates[i].next;
            mid_node.fineness = fine;
            nodes.push_back(mid_node);
        }
    }
    return nodes;
}

void node_coord(DataBase &database, std::string node_id, node &ret) {

    std::string query;
    node mid_node;

    query = "SELECT nodes.lat, nodes.lon "
            "FROM nodes "
            "WHERE node_id = " + node_id + ";";

    Request req_node(database, query);

    while(req_node.step() != SQLITE_DONE) {
        req_node.data(ret.lat, 0);
        req_node.data(ret.lon, 1);
    }
}

graph::graphNode getNode(uint64_t Node) {
    std::string node_id = std::to_string(Node);
    graph::graphNode gr_node;
    node mid_node;
    node_coord(graph::database, node_id, mid_node);
    gr_node.x = mid_node.lat;
    gr_node.y = mid_node.lon;
    return gr_node;
}

void buildings_receive_test() {
    std::string lat_low, lon_left, lat_up, lon_right;

    lat_low = "55.7274074";
    lon_left = "37.5765893";
    lat_up = "55.8322596";
    lon_right = "37.7582195";

    std::vector<way> a;

    DataBase db("../database/shadow.db");

    buildings_receive(db, lat_low, lon_left, lat_up, lon_right, a);
    std::cout.precision(8);
    for(int i = 0; i < a.size(); ++i) {
        for(int j = 0; j < a[i].seq.size(); ++j) {
            std::cout << "id: " << a[i].seq[j].id << " ";
            std::cout << "lat: " << a[i].seq[j].lat << " ";
            std::cout << "lon: " << a[i].seq[j].lon << " ";
            std::cout << "way_tag_value: " << a[i].tags.begin()->second << std::endl;
        } 
        if(i == 1) {
            break;
        }
    }
}

void getadjacencyMatrix_test() {
    std::cout << "getadjacencyMatrix_test() called" << std::endl;
    uint64_t node_id = 1540431697;
    std::cout << "calling getadjacencyMatrix(node_id) with node_id = " + std::to_string(node_id) << std::endl;
    std::vector<graph::weightNode> test = getadjacencyMatrix(node_id);
    for(auto& x: test) {
        std::cout << "index: " << x.index << " " << "fineness: " << x.fineness << std::endl;
    }
}

void getNode_test() {
    std::cout << "getNode_test() called" << std::endl;
    uint64_t node_id = 1540431697;
    std::cout << "calling getNode(node_id) with node_id = " + std::to_string(node_id) << std::endl;
    graph::graphNode test = getNode(node_id);
    std::cout.precision(10);

    std::cout << "x: " << test.x << " " << "y: " << test.y << std::endl;
}

int main() {

    getadjacencyMatrix_test();
    getNode_test();
    buildings_receive_test();

	return 0;
}

/*
    5. map переписать в json?
*/
