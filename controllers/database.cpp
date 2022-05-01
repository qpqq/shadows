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
    std::cout << "Opening " << path << "... ";
    flag = sqlite3_open_v2(this->path, &db, SQLITE_OPEN_READWRITE, nullptr);

    if (flag != SQLITE_OK) {
        std::cout << "failed" << std::endl;
        std::cerr << "response: " << sqlite3_errmsg(db) << std::endl;
        assert(flag == SQLITE_OK);
    } else {
        std::cout << "done" << std::endl;
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

unsigned long long int DataBase::closestNode(const std::vector<std::string> &coords) {
    const std::string &lat = coords[0];
    const std::string &lon = coords[1];

    // TODO: сделать проверку пустоты запроса через count() или количества строк (sqlite3.h)???.

    std::string query, dlat_plus, dlat_minus, dlon_plus, dlon_minus, _radius;
    std::vector<node> points;
    node mid_node;
    double radius = 0.00015625; // 1.0 km
    double delta = 0.00015625; // 1.0 km
    unsigned int i;

    mid_node.id = 0;
    mid_node.lat = -1.0;
    mid_node.lon = -1.0;

    std::cout << "Searching for the closest node... ";

    // searching for the closest node by accumulating the radius
    for (i = 1; i < 9; ++i) {

        radius += delta;
        _radius = std::to_string(radius);
        dlat_minus =    lat + " - " + _radius;
        dlat_plus  =    lat + " + " + _radius;
        dlon_minus =    lon + " - " + _radius;
        dlon_plus  =    lon + " + " + _radius;

        query = "SELECT node_id, lat, lon "
                "FROM road_nodes "
                "WHERE lat BETWEEN " + dlat_minus + " AND " + dlat_plus + " AND  lon BETWEEN " + dlon_minus + " AND " +
                dlon_plus + ";";

        Request req(*this, query);

        while (req.step() != SQLITE_DONE) {

            req.data(mid_node.id, 0);
            req.data(mid_node.lat, 1);
            req.data(mid_node.lon, 2);

            if (mid_node.id != 0) {
                points.push_back(mid_node);
            }

        }

        if (!points.empty()) {
            break;
        }

    }

    // RunTimeError assertion
    assert(!points.empty());

    std::vector<node> result;
    Closest finder;

    result = finder.kClosest(points, 1);

    std::cout << "done: " << result[0].id << std::endl;

    return result[0].id;
}

std::map<uint64_t, std::vector<uint64_t>>
DataBase::getAdjacencyMatrixFull(uint64_t startNode, uint64_t endNode) {

    std::cout << "Building adjacencyMatrix... ";

    std::map<uint64_t, std::vector<uint64_t>> dict;
    std::string query_matrix, between, query_nodes;
    std::string start_node, end_node, lat_low, lat_up, lon_left, lon_right;
    double lat1, lat2, lon1, lon2, delta;
    mate mid_mate;
    node mid_node;

    delta = 0.000625;

    start_node = std::to_string(startNode);
    end_node   = std::to_string(endNode);

    query_nodes = "SELECT node_id, lat, lon "
                  "FROM nodes "
                  "WHERE node_id = " + start_node + ";";

    Request req1_nodes(*this, query_nodes);

    while (req1_nodes.step() != SQLITE_DONE) {
        req1_nodes.data(mid_node.id, 0);
        req1_nodes.data(mid_node.lat, 1);
        req1_nodes.data(mid_node.lon, 2);
    }

    lat1 = mid_node.lat;
    lon1 = mid_node.lon;

    query_nodes = "SELECT node_id, lat, lon "
                  "FROM nodes "
                  "WHERE node_id = " + end_node + ";";

    Request req2_nodes(*this, query_nodes);

    while (req2_nodes.step() != SQLITE_DONE) {
        req2_nodes.data(mid_node.id, 0);
        req2_nodes.data(mid_node.lat, 1);
        req2_nodes.data(mid_node.lon, 2);
    }

    lat2 = mid_node.lat;
    lon2 = mid_node.lon;

    lat_low   = std::to_string(std::min(lat1, lat2) - delta);
    lat_up    = std::to_string(std::max(lat1, lat2) + delta);
    lon_left  = std::to_string(std::min(lon1, lon2) - delta);
    lon_right = std::to_string(std::max(lon1, lon2) + delta);


    between = "lat BETWEEN " + lat_low + " AND " + lat_up + " AND lon BETWEEN " + lon_left + " AND " + lon_right;

    // TODO разорвать два пути вместе

    query_matrix = "SELECT node_id, prev, next "
                   "FROM adjacency "
                   "WHERE " + between + ";";

    Request req_matrix(*this, query_matrix);

    while (req_matrix.step() != SQLITE_DONE) {
        req_matrix.data(mid_mate.id, 0);
        req_matrix.data(mid_mate.prev, 1);
        req_matrix.data(mid_mate.next, 2);

        if (mid_mate.prev != 0) {
            if (std::find(begin(dict[mid_mate.id]), end(dict[mid_mate.id]), mid_mate.prev) == std::end(dict[mid_mate.id])) {
                dict[mid_mate.id].push_back(mid_mate.prev);
            }
        }

        if (mid_mate.next != 0) {
            if (std::find(begin(dict[mid_mate.id]), end(dict[mid_mate.id]), mid_mate.next) == std::end(dict[mid_mate.id])) {
                dict[mid_mate.id].push_back(mid_mate.next);
            }
        }

    }

    std::cout << "done: ";
    std::cout << "number of elements: " << dict.size() << std::endl;

    return dict;

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

Closest::Closest() = default;

Closest::~Closest() = default;

std::vector<node> Closest::kClosest(std::vector<node> &points, int k) {
    return quickSelect(points, k);
}

std::vector<node> Closest::quickSelect(std::vector<node> &points, int k) {
    int left = 0, right = points.size() - 1;
    int pivotIndex = points.size();
    while (pivotIndex != k) {
        // Repeatedly partition the vector
        // while narrowing in on the kth element
        pivotIndex = partition(points, left, right);
        if (pivotIndex < k) {
            left = pivotIndex;
        } else {
            right = pivotIndex - 1;
        }
    }

    // Return the first k elements of the partially sorted vector
    return std::vector<node>(points.begin(), points.begin() + k);
}

int Closest::partition(std::vector<node> &points, int left, int right) {
    node &pivot = choosePivot(points, left, right);
    double pivotDist = squaredDistance(pivot);
    while (left < right) {
        // Iterate through the range and swap elements to make sure
        // that all points closer than the pivot are to the left
        if (squaredDistance(points[left]) >= pivotDist) {
            std::swap(points[left], points[right]);
            // points[left].swap(points[right]);
            right--;
        } else {
            left++;
        }
    }

    // Ensure the left pointer is just past the end of
    // the left range then return it as the new pivotIndex
    if (squaredDistance(points[left]) < pivotDist)
        left++;
    return left;
}

node &Closest::choosePivot(std::vector<node> &points, int left, int right) {
    // Choose a pivot element of the vector
    return points[left + (right - left) / 2];
}

double Closest::squaredDistance(node &point) {
    // Calculate and return the squared Euclidean distance
    return point.lat * point.lat + point.lon * point.lon;
}

void Closest::shift(node zero, node point) {
    // change coordinate system:
    // u = x - xZero;
    // v = y - yZero;

    point.lat = point.lat - zero.lat;
    point.lon = point.lon - zero.lon;
}
