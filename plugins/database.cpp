#include "plugins/database.hpp"

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

DataBase::~DataBase() = default;

void DataBase::open() {
    std::cout << "Opening " << path << "... ";
    std::cout.flush();

    int flag = sqlite3_open_v2(this->path, &db, SQLITE_OPEN_READWRITE, nullptr);

    if (flag != SQLITE_OK) {
        std::cout << "failed" << std::endl;
        std::cerr << "response: " << sqlite3_errmsg(db) << std::endl;
        assert(flag == SQLITE_OK);
    } else {
        std::cout << "done" << std::endl;
    }
}

void DataBase::initNodes() {
    std::cout << "Initializing nodes... ";
    std::cout.flush();

    std::string query = "SELECT nodes.node_id, nodes.lat, nodes.lon "
                        "FROM nodes;";

    Request req_node(*this, query);

    while (req_node.step() != SQLITE_DONE) {
        GraphNode graphNode;
        req_node.data(graphNode.id, 0);
        req_node.data(graphNode.x, 1);
        req_node.data(graphNode.y, 2);

        nodes[graphNode.id] = graphNode;
    }

    std::cout << "done: " << nodes.size() << " elements" << std::endl;
}

void DataBase::initAdjacencyMatrix() {
    std::cout << "Initializing adjacency matrix... ";
    std::cout.flush();

    Mate mid_mate;

    std::string query = "SELECT node_id, prev, next, lat, lon, tag_val "
                        "FROM adjacency;";

    Request req_matrix(*this, query);

    while (req_matrix.step() != SQLITE_DONE) {
        req_matrix.data(mid_mate.curr.id, 0);
        req_matrix.data(mid_mate.prev, 1);
        req_matrix.data(mid_mate.next, 2);
        req_matrix.data(mid_mate.curr.x, 3);
        req_matrix.data(mid_mate.curr.y, 4);
        req_matrix.data(mid_mate.path_type, 5);

        mid_mate.curr.fineness = price_list[mid_mate.path_type];

        if (mid_mate.prev != 0)
            adjacencyMatrix[mid_mate.curr].insert(nodes[mid_mate.prev]);

        if (mid_mate.next != 0)
            adjacencyMatrix[mid_mate.curr].insert(nodes[mid_mate.next]);

    }

    std::cout << "done: " << adjacencyMatrix.size() << " elements" << std::endl;
}

void DataBase::initBuildings() {
    std::cout << "Initializing buildings... ";
    std::cout.flush();

    std::string query, query_tags, query_nodes, pattern_tags, pattern_nodes;
    std::string between, tag_key, tag_val;

    Way way;
    Node node;
    unsigned int i, counter = 0;

    query = "SELECT way_id "
            "FROM buildings;";

    pattern_tags = "SELECT way_tags.tag_key, way_tags.tag_val "
                   "FROM way_tags "
                   "WHERE way_tags.tag_key = 'building:levels' AND way_tags.way_id = ";

    pattern_nodes = "SELECT nodes.node_id, nodes.lat, nodes.lon "
                    "FROM nodes "
                    "JOIN ways ON ways.node_id = nodes.node_id "
                    "WHERE ways.way_id = ";

    Request req_ways(*this, query);

    while (req_ways.step() != SQLITE_DONE) {
        req_ways.data(way.id, 0);
        buildings.push_back(way);
        counter++;
    }

    counter = 0;

    for (i = 0; i < buildings.size(); ++i) {
        query_tags = pattern_tags + std::to_string(buildings[i].id) + ";";
        query_nodes = pattern_nodes + std::to_string(buildings[i].id) + ";";

        Request req_tags(*this, query_tags);
        Request req_nodes(*this, query_nodes);

        while (req_tags.step() != SQLITE_DONE) {
            req_tags.data(tag_key, 0);
            req_tags.data(tag_val, 1);
            buildings[i].tags.insert(std::pair<std::string, std::string>(tag_key, tag_val));
        }

        while (req_nodes.step() != SQLITE_DONE) {
            req_nodes.data(node.id, 0);
            req_nodes.data(node.lat, 1);
            req_nodes.data(node.lon, 2);
            buildings[i].seq.push_back(node);
            counter++;
        }
    }

    std::cout << "done: " << buildings.size() << " elements" << std::endl;
}

void DataBase::initAndStart(const Json::Value &config) {
    this->path = "../shadow.db";
    open();

    initNodes();
    initAdjacencyMatrix();
    initBuildings();

    std::cout << "The server is ready" << std::endl;
    std::cout << std::endl;
}

void DataBase::shutdown() {
    sqlite3_close(db);
    this->~DataBase();
}

const char *DataBase::getPath() {
    return path;
}

sqlite3 *DataBase::getPointer() {
    return db;
}

std::string DataBase::toStringWithPrecision(double x, const int n) {
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << x;
    return out.str();
}

std::vector<std::string> DataBase::boundaries(const std::vector<std::vector<std::string>> &coords, double offset) {
    double latLow, latHigh, lonLeft, lonRight;
    latLow = 90;
    latHigh = -90;
    lonLeft = 180;
    lonRight = -180;

    for (auto coord: coords) {
        latLow = std::min(latLow, std::stod(coord[0]));
        latHigh = std::max(latHigh, std::stod(coord[0]));
        lonLeft = std::min(lonLeft, std::stod(coord[1]));
        lonRight = std::max(lonRight, std::stod(coord[1]));
    }

    latLow -= offset;
    latHigh += offset;
    lonLeft -= offset;
    lonRight += offset;

    return {toStringWithPrecision(latLow), toStringWithPrecision(latHigh),
            toStringWithPrecision(lonLeft), toStringWithPrecision(lonRight)};
}

std::vector<Way>
DataBase::buildingsReceive(std::vector<std::string> &coords1, std::vector<std::string> &coords2, double offset) {

    std::cout << "Building buildings... ";
    std::cout.flush();

    double latLow, latHigh, lonLeft, lonRight;

    auto bound = boundaries({coords1, coords2}, offset);
    latLow = std::stod(bound[0]);
    latHigh = std::stod(bound[1]);
    lonLeft = std::stod(bound[2]);
    lonRight = std::stod(bound[3]);

    std::vector<Way> buildingsCrop;

    for (auto &building: buildings) {

        int pass = 1;
        for (auto &node: building.seq) {
            if (!(latLow < node.lat and node.lat < latHigh and lonLeft < node.lon and node.lon < lonRight)) {
                pass = 0;
                break;
            }
        }

        if (pass == 0)
            continue;

        buildingsCrop.push_back(building);
    }

    std::cout << "done: " << buildingsCrop.size() << " elements" << std::endl;

    return buildingsCrop;
}

[[maybe_unused]] void DataBase::buildingsReceiveTest() {
    std::string lat_low, lon_left, lat_up, lon_right;

    lat_low = "55.7274074";
    lon_left = "37.5765893";
    lat_up = "55.8322596";
    lon_right = "37.7582195";

    std::vector<std::string> coords1 = {lat_low, lon_left};
    std::vector<std::string> coords2 = {lat_up, lon_right};

    std::vector<Way> a;

    a = buildingsReceive(coords1, coords2, 0);
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

std::map<GraphNode, std::set<GraphNode>>
DataBase::getAdjacencyMatrix(std::vector<std::string> &coords1, std::vector<std::string> &coords2,
                             double offset) {
    // TODO удалить road_nodes

    std::cout << "Building adjacency matrix... ";
    std::cout.flush();

    double latLow, latHigh, lonLeft, lonRight;

    auto bound = boundaries({coords1, coords2}, offset);
    latLow = std::stod(bound[0]);
    latHigh = std::stod(bound[1]);
    lonLeft = std::stod(bound[2]);
    lonRight = std::stod(bound[3]);


    std::map<GraphNode, std::set<GraphNode>> adjacencyMatrixCrop;

    for (auto &pair: adjacencyMatrix) {
        auto graphNode = pair.first;
        auto adjacencyNodes = pair.second;

        if (!(latLow < graphNode.x and graphNode.x < latHigh and lonLeft < graphNode.y and graphNode.y < lonRight))
            continue;

        std::set<GraphNode> tempSet;
        for (auto &node: adjacencyNodes) {
            if (latLow < node.x and node.x < latHigh and lonLeft < node.y and node.y < lonRight)
                tempSet.insert(node);
        }

        adjacencyMatrixCrop[graphNode] = tempSet;
    }

    std::cout << "done: " << adjacencyMatrixCrop.size() << " elements" << std::endl;

    return adjacencyMatrixCrop;
}

Node DataBase::nodeCoord(const std::string &node_id) {

    std::string query;
    Node ret;

    query = "SELECT nodes.lat, nodes.lon "
            "FROM nodes "
            "WHERE node_id = " + node_id + ";";

    Request req_node(*this, query);

    while (req_node.step() != SQLITE_DONE) {
        req_node.data(ret.lat, 0);
        req_node.data(ret.lon, 1);
    }

    return ret;
}

GraphNode DataBase::getNode(uint64_t node) {
    std::string node_id = std::to_string(node);
    GraphNode graphNode;
    Node mid_node = nodeCoord(node_id);
    graphNode.id = node;
    graphNode.x = mid_node.lat;
    graphNode.y = mid_node.lon;
    return graphNode;
}

GraphNode DataBase::closestNode(const std::vector<std::string> &coords) {
    const std::string &lat = coords[0];
    const std::string &lon = coords[1];

    // TODO: сделать проверку пустоты запроса через count() или количества строк (sqlite3.h)???.

    std::string query, dlat_plus, dlat_minus, dlon_plus, dlon_minus, _radius;
    std::vector<GraphNode> points;
    GraphNode mid_node, zero;
    double radius = 1000; // 1.0 km
    double delta = 1000; // 1.0 km
    radius *= 180 / M_PI / 6378100;
    delta *= 180 / M_PI / 6378100;

    unsigned int i;

    zero.x = std::stod(lat);
    zero.y = std::stod(lon);

    mid_node.id = 0;
    mid_node.x = -1.0;
    mid_node.y = -1.0;

    std::cout << "Searching for the closest node... ";
    std::cout.flush();

    // searching for the closest node by accumulating the radius
    for (i = 1; i < 9; ++i) {

        radius += delta;
        _radius = toStringWithPrecision(radius);
        dlat_minus = lat;
        dlat_minus += " - " + _radius;
        dlat_plus = lat;
        dlat_plus += " + " + _radius;
        dlon_minus = lon;
        dlon_minus += " - " + _radius;
        dlon_plus = lon;
        dlon_plus += " + " + _radius;

        query = "SELECT node_id, lat, lon "
                "FROM road_nodes "
                "WHERE lat BETWEEN " + dlat_minus;
        query += " AND " + dlat_plus;
        query += " AND  lon BETWEEN " + dlon_minus;
        query += " AND " + dlon_plus + ";";

        Request req(*this, query);

        while (req.step() != SQLITE_DONE) {

            req.data(mid_node.id, 0);
            req.data(mid_node.x, 1);
            req.data(mid_node.y, 2);

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

    std::vector<GraphNode> result;
    Closest finder(zero);

    result = finder.kClosest(points, 1);

    std::cout << "done: " << result[0].id << std::endl;

    return result[0];
}

Request::Request(DataBase &database, const std::string &query) {
    int flag;
    sqlite3 *db = database.getPointer();
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

void Request::data(uint64_t &ret, int col_id) {
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

Closest::Closest(GraphNode zero) {
    this->zero = zero;
}

Closest::~Closest() = default;

std::vector<GraphNode> Closest::kClosest(std::vector<GraphNode> &points, int k) {
    shift(points);
    return quickSelect(points, k);
}

std::vector<GraphNode> Closest::quickSelect(std::vector<GraphNode> &points, int k) {
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

    antiShift(points);

    // Return the first k elements of the partially sorted vector
    return std::vector<GraphNode>(points.begin(), points.begin() + k);
}

int Closest::partition(std::vector<GraphNode> &points, int left, int right) {
    GraphNode &pivot = choosePivot(points, left, right);
    double pivotDist = squaredDistance(pivot);
    while (left < right) {
        // Iterate through the range and swap elements to make sure
        // that all points closer than the pivot are to the left
        if (squaredDistance(points[left]) >= pivotDist) {
            std::swap(points[left], points[right]);
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

GraphNode &Closest::choosePivot(std::vector<GraphNode> &points, int left, int right) {
    // Choose a pivot element of the vector
    return points[left + (right - left) / 2];
}

double Closest::squaredDistance(GraphNode &point) {
    // Calculate and return the squared Euclidean distance
    return point.x * point.x + point.y * point.y;
}

void Closest::shift(std::vector<GraphNode> &points) {
    // change coordinate system:
    // u = x - xZero;
    // v = y - yZero;

    for (auto &point: points) {
        point.x = point.x - zero.x;
        point.y = point.y - zero.y;
    }
}

void Closest::antiShift(std::vector<GraphNode> &points) {
    for (auto &point: points) {
        point.x = point.x + zero.x;
        point.y = point.y + zero.y;
    }
}
