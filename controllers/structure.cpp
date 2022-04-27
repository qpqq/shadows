//#include "structure.hpp"
//
//void buildings_receive(DataBase &database, std::string &lat_low, std::string &lon_left, std::string &lat_up, std::string &lon_right, std::vector<way> &build) {
//
//    std::string query, query_tags, query_nodes, pattern_tags, pattern_nodes;
//    std::string between, tag_key, tag_val;
//    way mid_way;
//    node mid_node;
//    unsigned int i, counter = 0;
//
//    between = "(lat BETWEEN " + lat_low + " AND " + lat_up + ")" + " AND (lon BETWEEN " + lon_left + " AND " + lon_right + ")";
//
//    query = "SELECT way_id "
//            "FROM WayBuildings "
//            "WHERE tag_key = 'building' AND " + between + " GROUP BY way_id;";
//
//    pattern_tags = "SELECT way_tags.tag_key, way_tags.tag_val "
//                   "FROM way_tags "
//                   "WHERE way_tags.tag_key = 'building:levels' AND way_tags.way_id = ";
//
//    pattern_nodes = "SELECT nodes.node_id, nodes.lat, nodes.lon "
//                    "FROM nodes "
//                    "JOIN ways ON ways.node_id = nodes.node_id "
//                    "WHERE ways.way_id = ";
//
//    Request req_ways(database, query);
//
//    std::cout << "selecting ways by coordinates: ";
//
//    while (req_ways.step() != SQLITE_DONE) {
//        req_ways.data(mid_way.id, 0);
//        build.push_back(mid_way);
//        counter++;
//    }
//
//    std::cout << "total " + std::to_string(counter - 1) << std::endl;
//
//    counter = 0;
//
//    std::cout << "selecting building:levels by way.id: ";
//
//    for (i = 0; i < build.size(); ++i) {
//        query_tags  = pattern_tags  + std::to_string(build[i].id) + ";";
//        query_nodes = pattern_nodes + std::to_string(build[i].id) + ";";
//
//        Request req_tags(database, query_tags);
//        Request req_nodes(database, query_nodes);
//
//        while (req_tags.step() != SQLITE_DONE) {
//            req_tags.data(tag_key, 0);
//            req_tags.data(tag_val, 1);
//            build[i].tags.insert(std::pair<std::string, std::string>(tag_key, tag_val));
//        }
//
//        while (req_nodes.step() != SQLITE_DONE) {
//            req_nodes.data(mid_node.id, 0);
//            req_nodes.data(mid_node.lat, 1);
//            req_nodes.data(mid_node.lon, 2);
//            build[i].seq.push_back(mid_node);
//            counter++;
//        }
//    }
//
//    std::cout << "total " + std::to_string(i - 1) << std::endl;
//    std::cout << "total nodes " + std::to_string(counter - 1) << std::endl;
//
//}
//
//void neighbours_receive(DataBase &database, const std::string &node_id, std::vector<mate> &mates) {
//
//    std::string query, withas, mid_select, query_tag;
//    std::vector<std::string> ways;
//    std::string mid_way;
//    unsigned int i;
//    mate mid_mate;
//
//    query = "SELECT way_id FROM ways WHERE node_id = " + node_id + ";";
//
//    Request req_ways(database, query);
//
//    while (req_ways.step() != SQLITE_DONE) {
//        req_ways.data(mid_way, 0);
//        ways.push_back(mid_way);
//    }
//
//    for (i = 0; i < ways.size(); ++i) {
//        mid_mate.prev = 0;
//        mid_mate.next = 0;
//        mid_mate.path_type = "";
//
//        withas = "WITH mid AS ( "
//                 "SELECT node_id, "
//                 "LAG(node_id, 1, 0) OVER (ORDER BY seq_id) pv, "
//                 "LEAD(node_id, 1, 0) OVER (ORDER BY seq_id) nt "
//                 "FROM ways "
//                 "WHERE way_id = " + ways[i] + ") ";
//
//        mid_select = "SELECT pv, nt "
//                     "FROM mid "
//                     "WHERE node_id = " + node_id + ";";
//
//        query = withas + mid_select;
//
//        query_tag = "SELECT tag_val "
//                    "FROM way_tags "
//                    "WHERE tag_key = 'highway' AND way_id = " + ways[i] + ";";
//
//        Request req_tag(database, query_tag);
//        Request req_neighbour(database, query);
//
//        while (req_tag.step() != SQLITE_DONE) {
//            req_tag.data(mid_mate.path_type, 0);
//        }
//
//        while (req_neighbour.step() != SQLITE_DONE) {
//            req_neighbour.data(mid_mate.prev, 0);
//            req_neighbour.data(mid_mate.next, 1);
//            mates.push_back(mid_mate);
//        }
//    }
//}
//
//int define_fine(const std::string &path_type) {
//    if (price_list.count(path_type) > 0) {
//        return price_list[path_type];
//    }
//    return 0;
//}
//
////std::vector<Graph::weightNode> getAdjacencyMatrix(uint64_t Node) {
////
////    std::vector<Graph::weightNode> nodes;
////    std::string node_id = std::to_string(Node);
////    std::vector<mate> mates;
////    Graph::weightNode mid_node;
////    int i, fine;
////
////    neighbours_receive(Graph::database, node_id, mates);
////
////    for (i = 0; i < mates.size(); ++i) {
////        fine = define_fine(mates[i].path_type);
////        if (mates[i].prev != 0) {
////            mid_node.index = mates[i].prev;
////            mid_node.fineness = fine;
////            nodes.push_back(mid_node);
////        }
////        if (mates[i].next != 0) {
////            mid_node.index = mates[i].next;
////            mid_node.fineness = fine;
////            nodes.push_back(mid_node);
////        }
////    }
////    return nodes;
////}
//
//void node_coord(DataBase &database, const std::string &node_id, node &ret) {
//
//    std::string query;
//    node mid_node;
//
//    query = "SELECT nodes.lat, nodes.lon "
//            "FROM nodes "
//            "WHERE node_id = " + node_id + ";";
//
//    Request req_node(database, query);
//
//    while (req_node.step() != SQLITE_DONE) {
//        req_node.data(ret.lat, 0);
//        req_node.data(ret.lon, 1);
//    }
//}
//
////Graph::graphNode getNode(uint64_t Node) {
////    std::string node_id = std::to_string(Node);
////    Graph::graphNode gr_node;
////    node mid_node;
////    node_coord(Graph::database, node_id, mid_node);
////    gr_node.x = mid_node.lat;
////    gr_node.y = mid_node.lon;
////    return gr_node;
////}
//
//void buildings_receive_test() {
//    std::string lat_low, lon_left, lat_up, lon_right;
//
//    lat_low = "55.7274074";
//    lon_left = "37.5765893";
//    lat_up = "55.8322596";
//    lon_right = "37.7582195";
//
//    std::vector<way> a;
//
//    DataBase db("shadow.db");
//
//    buildings_receive(db, lat_low, lon_left, lat_up, lon_right, a);
//    std::cout.precision(8);
//    for (int i = 0; i < a.size(); ++i) {
//        for (int j = 0; j < a[i].seq.size(); ++j) {
//            std::cout << "id: " << a[i].seq[j].id << " ";
//            std::cout << "lat: " << a[i].seq[j].lat << " ";
//            std::cout << "lon: " << a[i].seq[j].lon << " ";
//            std::cout << "way_tag_value: " << a[i].tags.begin()->second << std::endl;
//        }
//        if (i == 1) {
//            break;
//        }
//    }
//}
//
//node closestNode(DataBase &database, const std::string &lat, const std::string &lon) {
//    std::string query, sq, dlat_plus, dlat_minus, dlon_plus, dlon_minus, _radius;
//    double radius = 0.0003125; // 0.125 km
//    unsigned int i;
//
//    node ret;
//    ret.id = 0;
//    ret.lat = -1.0;
//    ret.lon = -1.0;
//
//    sq = "sqrt((lat - " + lat + ")*(lat - " + lat + ") + (lon - " + lon + ")*(lon - " + lon + "))";
//
//    // searching for the closest node by accumulating the radius
//    for (i = 1; i < 9; ++i) {
//
//        radius += radius;
//        _radius = std::to_string(radius);
//        dlat_minus =    lat + " - " + _radius;
//        dlat_plus  =    lat + " + " + _radius;
//        dlon_minus =    lon + " - " + _radius;
//        dlon_plus =     lon + " + " + _radius;
//
//        query = "SELECT node_id, lat, lon, " + sq + " AS rho "
//                "FROM nodes "
//                "WHERE lat BETWEEN " + dlat_minus + " AND " + dlat_plus + " AND lon BETWEEN " + dlon_minus + " AND " + dlon_plus + " AND rho != 0 "
//                "ORDER BY rho ASC "
//                "LIMIT 1;";
//
//        Request req(database, query);
//
//        while (req.step() != SQLITE_DONE) {
//            req.data(ret.id, 0);
//            req.data(ret.lat, 1);
//            req.data(ret.lon, 2);
//        }
//
//        if (ret.lat != -1.0 && ret.lon != -1.0) {
//            break;
//        }
//
//    }
//    // RunTimeError assertion
//    assert(ret.lat != -1.0 && ret.lon != -1.0);
//
//    return ret;
//}
//
///*
//
//    Исправления
//
//        1. closestNode в structure.cpp
//        2. closestNode в structure.hpp
//
//    Получение объектных файлов
//
//    gcc -c sqlite/sqlite3.c -lsqlite3 -DSQLITE_ENABLE_MATH_FUNCTIONS
//    с++ -с database.cpp
//    c++ -c structure.cpp
//
//    Получение исполняемого
//
//    c++ structure.o database.o sqlite3.o
//
//*/
//
///*
//    5. map переписать в json?
//*/
