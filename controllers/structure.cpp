#include "structure.hpp"

/*
Used functions from sqlite3.h:

1. sqlite3_open() --- open connection with db;

2. sqlite3_prepare_v2() --- transforms sql querry to bin querry;

3. typedef struct sqlite3_stmt sqlite3_stmt;

An instance of this object represents a single SQL statement that has been compiled into binary form and is ready to be evaluated.

Think of each SQL statement as a separate computer program. The original SQL text is source code. A prepared statement object is the compiled object code. All SQL must be converted into a prepared statement before it can be run.

4. sqlite3_step(stmt) --- execute bin querry (step by step)

5. sqlite3_column_type(stmt, i) --- defines i column's type on any step

6. sqlite3_column_text(stmt, i) --- returns text from i col on any step

7. sqlite3_column_int(stmt, i) --- returns int from i col on any step

8. sqlite3_column_double(stmt, i) --- returns double from i col on any step

if the query returns an integer but the sqlite3_column_text() interface is used to extract the value, then an automatic type conversion is performed.

*/


int resp::buildings_receive(const char *db_path, std::string lat_low, std::string lon_left, std::string lat_up, std::string lon_right, std::vector<resp::way> &build) {

    sqlite3 *db;
    sqlite3_stmt *stmt, *stmt1, *stmt2;
    std::string query, query_tags, query_nodes, pattern_tags, pattern_nodes;
    std::string between, tag_key, tag_val;
    resp::way mid_way;
    resp::node mid_node;
    unsigned int i, counter = 0;
    int flag;

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

    std::cout << "Openning db " + std::string(db_path) + ": ";

    flag = sqlite3_open(db_path, &db);

    if (flag != SQLITE_OK) {
        std::cerr << "failed"<< std::endl;
        return 1;
    }

    std::cout << "done" << std::endl;

    flag = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);

    if (flag != SQLITE_OK) {
        std::cerr << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    std::cout << "selecting ways by coordinates: ";

    while(sqlite3_step(stmt) != SQLITE_DONE) {
        mid_way.id = sqlite3_column_int64(stmt, 0);
        build.push_back(mid_way);
        counter++;
    }

    std::cout << "total " + std::to_string(counter - 1) << std::endl;

    sqlite3_finalize(stmt);

    counter = 0;

    std::cout << "selecting building:levels by way.id: ";

    for(i = 0; i < build.size(); ++i) {
        query_tags  = pattern_tags  + std::to_string(build[i].id) + ";";
        query_nodes = pattern_nodes + std::to_string(build[i].id) + ";";

        flag = sqlite3_prepare_v2(db, query_tags.c_str(), -1, &stmt1, NULL);

        if (flag != SQLITE_OK) {
            std::cerr << "failed preparation by tag" << std::endl;
            return 1;
        }

        while(sqlite3_step(stmt1) != SQLITE_DONE) {
            tag_key = std::string((char *) sqlite3_column_text(stmt1, 0));
            tag_val = std::string((char *) sqlite3_column_text(stmt1, 1));
            build[i].tags.insert(std::pair<std::string, std::string>(tag_key, tag_val));
        }

        flag = sqlite3_prepare_v2(db, query_nodes.c_str(), -1, &stmt2, NULL);

        if (flag != SQLITE_OK) {
            std::cerr << "failed preparation by node" << std::endl;
            return 1;
        }

        while(sqlite3_step(stmt2) != SQLITE_DONE) {
            mid_node.id  =  sqlite3_column_int64(stmt2, 0);
            mid_node.lat =  sqlite3_column_double(stmt2, 1);
            mid_node.lon =  sqlite3_column_double(stmt2, 2);
            build[i].seq.push_back(mid_node);
            counter++;
        }

        sqlite3_finalize(stmt1);
        sqlite3_finalize(stmt2);
    }

    std::cout << "total " + std::to_string(i - 1) << std::endl;
    std::cout << "total nodes " + std::to_string(counter - 1) << std::endl;


    sqlite3_close(db);
    return 0;
}

int resp::neighbours_receive(const char *db_path, std::string node_id, std::vector<resp::mate> &mates) {

    sqlite3 *db;
    sqlite3_stmt *stmt, *stmt1;
    std::string query, withas, mid_select, query_tag;
    std::vector<std::string> ways;
    std::string mid_way;
    int i, flag;
    resp::mate mid_mate;

    query = "SELECT way_id FROM ways WHERE node_id = " + node_id + ";";

    std::cout << "Openning db " + std::string(db_path) + ": ";

    flag = sqlite3_open(db_path, &db);

    if (flag != SQLITE_OK) {
        std::cerr << "failed" << std::endl;
        return 1;
    }

    std::cout << "done" << std::endl;

    flag = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);

    if (flag != SQLITE_OK) {
        std::cerr << "failed preparation sql data to binary" << std::endl;
        return 1;
    }

    std::cout << "selecting ways by node_id: ";

    while(sqlite3_step(stmt) != SQLITE_DONE) {
        mid_way = std::to_string(sqlite3_column_int64(stmt, 0));
        ways.push_back(mid_way);
    }

    std::cout << "done" << std::endl;

    sqlite3_finalize(stmt);

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

        flag = sqlite3_prepare_v2(db, query_tag.c_str(), -1, &stmt1, NULL);

        if (flag != SQLITE_OK) {
            std::cerr << "failed preparation sql data to binary" << std::endl;
            return 1;
        }

        while(sqlite3_step(stmt1) != SQLITE_DONE) {
            mid_mate.path_type = std::string((char *) sqlite3_column_text(stmt1, 0));
        }

        flag = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);

        if (flag != SQLITE_OK) {
            std::cerr << "failed preparation sql data to binary" << std::endl;
            return 1;
        }

        while(sqlite3_step(stmt) != SQLITE_DONE) {
            mid_mate.prev = sqlite3_column_int64(stmt, 0);
            mid_mate.next = sqlite3_column_int64(stmt, 1);
            mates.push_back(mid_mate);
        }
        sqlite3_finalize(stmt);
        sqlite3_finalize(stmt1);
    }
    sqlite3_close(db);
    return 0;
}

int resp::define_fine(std::string path_type) {
    if(price_list.count(path_type) > 0) {
        return price_list[path_type];
    }
    return 0;
}

std::vector<graph::weightNode> getadjacencyMatrix(uint64_t Node) {

    std::vector<graph::weightNode> nodes;
    std::string node_id = std::to_string(Node);
    std::vector<resp::mate> mates;
    graph::weightNode mid_node;
    int i, fine;

    neighbours_receive("../database/shadow.db", node_id, mates);

    for(i = 0; i < mates.size(); ++i) {

        fine = resp::define_fine(mates[i].path_type);
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

int resp::node_coord(const char *db_path, std::string node_id, resp::node &ret) {

    sqlite3 *db;
    sqlite3_stmt *stmt;
    std::string query;
    resp::node mid_node;
    int i, flag;

    query = "SELECT nodes.lat, nodes.lon "
            "FROM nodes "
            "WHERE node_id = " + node_id + ";";

    std::cout << "Openning db " + std::string(db_path) + ": ";

    flag = sqlite3_open(db_path, &db);

    if (flag != SQLITE_OK) {
        std::cerr << "failed"<< std::endl;
        return 1;
    }

    std::cout << "done" << std::endl;

    flag = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);

    if (flag != SQLITE_OK) {
        std::cerr << "failed preparation sql data to binary" << std::endl;
        return 1;
    }

    while(sqlite3_step(stmt) != SQLITE_DONE) {
        ret.lat =  sqlite3_column_double(stmt, 0);
        ret.lon =  sqlite3_column_double(stmt, 1);
    }

    sqlite3_finalize(stmt);

    sqlite3_close(db);
    return 0;
}

graph::graphNode getNode(uint64_t Node) {
    std::string node_id = std::to_string(Node);
    graph::graphNode gr_node;
    resp::node mid_node;
    node_coord("../database/shadow.db", node_id, mid_node);
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

    std::vector<resp::way> a;

    buildings_receive("../database/shadow.db", lat_low, lon_left, lat_up, lon_right, a);
    std::cout.precision(8);
    for(int i = 0; i < a.size(); ++i) {
        for(int j = 0; j < a[i].seq.size(); ++j) {
            std::cout << "id: " << a[i].seq[j].id << " ";
            std::cout << "lat: " << a[i].seq[j].lat << " ";
            std::cout << "lon: " << a[i].seq[j].lon << std::endl;
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

int structureTest() {

    getadjacencyMatrix_test();
    getNode_test();

    return 0;
}

/*
    1. Добавить sqlite3_errmsg в sqlite3_prepare_v2().
    2. Добавить обработку ошибок с помощью кода возврата функций.
    2. Добавить функцию печати запроса (гитхаб, ссылку скидывал Максиму).
    3. Класс БД.
    4. не создавать бд при открытии
    5. map переписать в json?
    6. пространство имён - проверить
*/
