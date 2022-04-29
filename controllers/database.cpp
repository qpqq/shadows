#include "database.hpp"

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

DataBase::DataBase() = default;

DataBase::DataBase(const std::string &path) {
    int flag;
    this->path = path.c_str();
    std::cout << "Opening " << path << std::endl;
    flag = sqlite3_open_v2(this->path, &db, SQLITE_OPEN_READWRITE, nullptr);

    if (flag != SQLITE_OK) {
        std::cout << "failed" << std::endl;
        std::cerr << "response: " << sqlite3_errmsg(db) << std::endl;
        assert(flag == SQLITE_OK);
    } else {
        std::cout << "Opening " << path << " done" << std::endl;
    }
}

const char *DataBase::get_path() {
    return path;
}

sqlite3 *DataBase::get_pointer() {
    return db;
}

DataBase::~DataBase() {
    sqlite3_close(db);
}

std::vector<way>
DataBase::buildings_receive(std::string &lat_low, std::string &lon_left, std::string &lat_up, std::string &lon_right) {
    std::vector<way> build;

    std::string query, query_tags, query_nodes, pattern_tags, pattern_nodes;
    std::string between, tag_key, tag_val;
    way mid_way;
    node mid_node;
    unsigned int i, counter = 0;

    between =
            "(lat BETWEEN " + lat_low + " AND " + lat_up + ")" + " AND (lon BETWEEN " + lon_left + " AND " + lon_right +
            ")";

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

    Request req_ways(*this, query);

    std::cout << "selecting ways by coordinates: ";

    while (req_ways.step() != SQLITE_DONE) {
        req_ways.data(mid_way.id, 0);
        build.push_back(mid_way);
        counter++;
    }

    std::cout << "total " + std::to_string(counter - 1) << std::endl;

    counter = 0;

    std::cout << "selecting building:levels by way.id: ";

    for (i = 0; i < build.size(); ++i) {
        query_tags = pattern_tags + std::to_string(build[i].id) + ";";
        query_nodes = pattern_nodes + std::to_string(build[i].id) + ";";

        Request req_tags(*this, query_tags);
        Request req_nodes(*this, query_nodes);

        while (req_tags.step() != SQLITE_DONE) {
            req_tags.data(tag_key, 0);
            req_tags.data(tag_val, 1);
            build[i].tags.insert(std::pair<std::string, std::string>(tag_key, tag_val));
        }

        while (req_nodes.step() != SQLITE_DONE) {
            req_nodes.data(mid_node.id, 0);
            req_nodes.data(mid_node.lat, 1);
            req_nodes.data(mid_node.lon, 2);
            build[i].seq.push_back(mid_node);
            counter++;
        }
    }

    std::cout << "total " + std::to_string(i - 1) << std::endl;
    std::cout << "total nodes " + std::to_string(counter - 1) << std::endl;

    return build;
}

void DataBase::buildings_receive_test() {
    std::string lat_low, lon_left, lat_up, lon_right;

    lat_low = "55.7274074";
    lon_left = "37.5765893";
    lat_up = "55.8322596";
    lon_right = "37.7582195";

    std::vector<way> a;

    a = buildings_receive(lat_low, lon_left, lat_up, lon_right);
    std::cout.precision(8);
    for (int i = 0; i < a.size(); ++i) {
        for (int j = 0; j < a[i].seq.size(); ++j) {
            std::cout << "id: " << a[i].seq[j].id << " ";
            std::cout << "lat: " << a[i].seq[j].lat << " ";
            std::cout << "lon: " << a[i].seq[j].lon << " ";
            std::cout << "way_tag_value: " << a[i].tags.begin()->second << std::endl;
        }
        if (i == 1) {
            break;
        }
    }
}

void DataBase::neighbours_receive(const std::string &node_id, std::vector<mate> &mates) {

    std::string query, withas, mid_select, query_tag;
    std::vector<std::string> ways;
    std::string mid_way;
    unsigned int i;
    mate mid_mate;

    query = "SELECT way_id FROM ways WHERE node_id = " + node_id + ";";

    Request req_ways(*this, query);

    while (req_ways.step() != SQLITE_DONE) {
        req_ways.data(mid_way, 0);
        ways.push_back(mid_way);
    }

    for (i = 0; i < ways.size(); ++i) {
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

        Request req_tag(*this, query_tag);
        Request req_neighbour(*this, query);

        while (req_tag.step() != SQLITE_DONE) {
            req_tag.data(mid_mate.path_type, 0);
        }

        while (req_neighbour.step() != SQLITE_DONE) {
            req_neighbour.data(mid_mate.prev, 0);
            req_neighbour.data(mid_mate.next, 1);
            mates.push_back(mid_mate);
        }
    }
}

int DataBase::define_fine(const std::string &path_type) {
    if (price_list.count(path_type) > 0) {
        return price_list[path_type];
    }
    return 0;
}

std::vector<weightNode> DataBase::getAdjacencyMatrix(uint64_t Node) {

    std::vector<weightNode> nodes;
    std::string node_id = std::to_string(Node);
    std::vector<mate> mates;
    weightNode mid_node{};
    int i, fine;

    neighbours_receive(node_id, mates);

    for (i = 0; i < mates.size(); ++i) {
        fine = define_fine(mates[i].path_type);
        if (mates[i].prev != 0) {
            mid_node.index = mates[i].prev;
            mid_node.fineness = fine;
            nodes.push_back(mid_node);
        }
        if (mates[i].next != 0) {
            mid_node.index = mates[i].next;
            mid_node.fineness = fine;
            nodes.push_back(mid_node);
        }
    }
    return nodes;
}

void DataBase::node_coord(const std::string &node_id, node &ret) {

    std::string query;
    node mid_node;

    query = "SELECT nodes.lat, nodes.lon "
            "FROM nodes "
            "WHERE node_id = " + node_id + ";";

    Request req_node(*this, query);

    while (req_node.step() != SQLITE_DONE) {
        req_node.data(ret.lat, 0);
        req_node.data(ret.lon, 1);
    }
}

graphNode DataBase::getNode(uint64_t Node) {
    std::string node_id = std::to_string(Node);
    graphNode gr_node{};
    node mid_node;
    node_coord(node_id, mid_node);
    gr_node.x = mid_node.lat;
    gr_node.y = mid_node.lon;
    return gr_node;
}

unsigned long long int DataBase::closestNode(const std::string &lat, const std::string &lon) {
    std::string query, sq, dlat_plus, dlat_minus, dlon_plus, dlon_minus, _radius;
    double radius = 0.0003125; // 0.125 km
    unsigned int i;

    node ret;
    ret.id = 0;
    ret.lat = -1.0;
    ret.lon = -1.0;

    sq = "sqrt((lat - " + lat + ")*(lat - " + lat + ") + (lon - " + lon + ")*(lon - " + lon + "))";

    // searching for the closest node by accumulating the radius
    for (i = 1; i < 9; ++i) {

        radius += radius;
        _radius = std::to_string(radius);
        dlat_minus =    lat + " - " + _radius;
        dlat_plus  =    lat + " + " + _radius;
        dlon_minus =    lon + " - " + _radius;
        dlon_plus =     lon + " + " + _radius;

        query = "SELECT nodes.node_id, lat, lon, " + sq + " AS rho "
                "FROM nodes "
                "JOIN ways ON ways.node_id = nodes.node_id "
                "JOIN way_tags ON way_tags.way_id = ways.way_id "
                "WHERE way_tags.tag_key = 'highway' AND lat BETWEEN " + dlat_minus + " AND " + dlat_plus + " AND lon BETWEEN " + dlon_minus + " AND " + dlon_plus + " AND rho != 0 "
                "GROUP BY nodes.node_id "
                "ORDER BY rho ASC "
                "LIMIT 1;";

        Request req(*this, query);

        while (req.step() != SQLITE_DONE) {
            req.data(ret.id, 0);
            req.data(ret.lat, 1);
            req.data(ret.lon, 2);
        }

        if (ret.lat != -1.0 && ret.lon != -1.0) {
            break;
        }

    }

    // RunTimeError assertion
    assert(ret.lat != -1.0 && ret.lon != -1.0);

    return ret.id;
}

std::map<uint64_t, std::vector<uint64_t>>
DataBase::getAdjacencyMatrixFull(uint64_t startNode, uint64_t endNode) {
    "SELECT ways.node_id,\n"
    "       LAG(ways.node_id, 1, 0) OVER (ORDER BY seq_id)  pv,\n"
    "       LEAD(ways.node_id, 1, 0) OVER (ORDER BY seq_id) nt\n"
    "FROM ways\n"
    "         JOIN nodes ON ways.node_id = nodes.node_id\n"
    "         JOIN way_tags ON way_tags.way_id = ways.way_id\n"
    "WHERE lat BETWEEN 55.7852698 - 0.7 AND 55.7852698 + 0.7\n"
    "    AND lon BETWEEN 37.6491053 - 0.7 AND 37.6491053 + 0.7\n"
    "    AND way_tags.tag_key = 'highway'\n"
    "   OR way_tags.tag_key = 'bridge'\n"
    "LIMIT 1000;\n"
    "\n"
    "-- 26999678|55.7852698|37.6491053";
    return {};
}

Request::Request(DataBase &database, const std::string &query) {
    int flag;
    sqlite3 *db = database.get_pointer();
    this->query = query.c_str();
    flag = sqlite3_prepare_v2(db, this->query, -1, &stmt, nullptr);

    if (flag != SQLITE_OK) {
        std::cerr << "Request.sqlite3_prepare_v2 failed: errcode = " << flag << std::endl;
        std::cerr << "Response: " << sqlite3_errmsg(db) << std::endl;
        assert(flag == SQLITE_OK);
    }
}

int Request::step() {
    return sqlite3_step(stmt);
}

void Request::data(unsigned long long int &ret, int col_id) {
    ret = sqlite3_column_int64(stmt, col_id);
}

void Request::data(double &ret, int col_id) {
    ret = sqlite3_column_double(stmt, col_id);
}

void Request::data(std::string &ret, int col_id) {
    ret = std::string((char *) sqlite3_column_text(stmt, col_id));
}

Request::~Request() {
    sqlite3_finalize(stmt);
}
