#include "grid.hpp"

Grid::Grid() = default;

Grid::Grid(std::vector<std::vector<std::string>> coords, double offset, std::vector<Way> ArrOfWays, double gridStep)
        : waysArr{std::move(ArrOfWays)}, step{gridStep} {

    std::cout << "Fill the grid with shadows... ";
    std::cout.flush();

    auto bound = DataBase::boundaries(coords, offset);
    min_lat = std::stod(bound[0]) - offsetOffset;
    max_lat = std::stod(bound[1]) + offsetOffset;
    min_lon = std::stod(bound[2]) - offsetOffset;
    max_lon = std::stod(bound[3]) + offsetOffset;

    double dlat_meters = (max_lat - min_lat) * (EarthPerimeter / 360);
    double dlon_meters = (max_lon - min_lon) * (EarthPerimeter / 360);

    n_lat = (int) ceil(dlat_meters / step);
    n_lon = (int) ceil(dlon_meters / step);
    n_x = n_lon;
    n_y = n_lat;

    dlat = (max_lat - min_lat) / (double) n_lat;
    dlon = (max_lon - min_lon) / (double) n_lon;

    auto solarCoords = getSolarCoords((max_lat + min_lat) / 2, (max_lon + min_lon) / 2);
    elev = solarCoords.first;
//    elev = 20;
    azim = solarCoords.second;
//    azim = 270;

    grid = std::vector<std::vector<double>>(n_y);

    if (elev <= 0)
        makeGrid(1);
    else {
        makeGrid(0);

//        _fillIn(0, (int) waysArr.size());
        fillIn();
    }

    std::cout << "done: " << n_x << "×" << n_y
              << ", elev = " << elev << ", azim = " << azim << std::endl;
}

double Grid::getColor(iPnt p) {
    return grid[n_y - 1 - p.y][p.x];
}

void Grid::plot(iPnt p) {
    grid[n_y - 1 - p.y][p.x] = p.color;
}

void Grid::plotPnts(const std::vector<iPnt> &points) {
    for (auto p: points)
        plot(p);
}

iPnt Grid::pntToPnt(double lat, double lon) const {
    int y = (int) ((lat - min_lat) / dlat);
    int x = (int) ((lon - min_lon) / dlon);

    if (x < 0 or x > n_x - 1 or y < 0 or y > n_y - 1)
        std::cerr << "Out of bounds" << std::endl;

    x = std::max(0, x);
    y = std::max(0, y);
    y = std::min(y, n_y - 1);
    x = std::min(x, n_x - 1);

    return {x, y};
}


std::vector<iPnt> Grid::pntsUnderLineLow(iPnt p1, iPnt p2) {
    int dx = abs(p1.x - p2.x);
    int dy = abs(p1.y - p2.y);
    int error = 0;
    int slope = (dy + 1);

    int slope_sign;
    if (p2.y - p1.y >= 0)
        slope_sign = 1;
    else
        slope_sign = -1;

    std::vector<iPnt> marked;

    int y = p1.y;
    for (int x = p1.x; x <= p2.x; x++) {
        marked.push_back({x, y, 1});

        error += slope;

        if (error >= dx + 1) {
            y += slope_sign;
            error -= dx + 1;
        }
    }

    return marked;
}

std::vector<iPnt> Grid::pntsUnderLineHigh(iPnt p1, iPnt p2) {
    int dx = abs(p1.x - p2.x);
    int dy = abs(p1.y - p2.y);
    int error = 0;
    int slope = (dx + 1);

    int slope_sign;
    if (p2.x - p1.x >= 0)
        slope_sign = 1;
    else
        slope_sign = -1;

    std::vector<iPnt> marked;

    int x = p1.x;
    for (int y = p1.y; y <= p2.y; y++) {
        marked.push_back({x, y, 1});

        error += slope;

        if (error >= dy + 1) {
            x += slope_sign;
            error -= dy + 1;
        }
    }

    return marked;
}

std::vector<iPnt> Grid::pntsUnderLine(iPnt p1, iPnt p2) {
    if (p1.x == p2.x) {
        std::vector<iPnt> marked;

        int x = p1.x;
        for (int y = std::min(p1.y, p2.y); y <= std::max(p1.y, p2.y); y++) {
            marked.push_back({x, y, 1});
        }

        return marked;
    }

    if (abs(p1.x - p2.x) >= abs(p1.y - p2.y)) {
        if (p1.x > p2.x)
            return pntsUnderLineLow(p2, p1);

        return pntsUnderLineLow(p1, p2);
    }

    if (p1.y > p2.y)
        return pntsUnderLineHigh(p2, p1);

    return pntsUnderLineHigh(p1, p2);
}

void Grid::_makeGrid(int startInd, int endInd, double color) {
    for (int i = startInd; i < endInd; i++)
        grid[i] = std::vector<double>(n_x, color);
}

void Grid::makeGrid(double color, int numberOfThreads) {
    if (numberOfThreads == -1) {
        numberOfThreads = (int) std::thread::hardware_concurrency(); // number of processor threads
        if (numberOfThreads == 0)
            numberOfThreads = 8;
    }

    std::vector<std::thread> threads;

    for (int i = 0; i < numberOfThreads; i++) {
        int startInd = i * (int) grid.size() / numberOfThreads;
        int endInd = (i + 1) * (int) grid.size() / numberOfThreads;

        if (i == numberOfThreads - 1)
            endInd = (int) grid.size();

        threads.emplace_back(&Grid::_makeGrid, this, startInd, endInd, color);
    }

    for (auto &t: threads) {
        t.join();
    }
}

void Grid::_fillIn(int startInd, int endInd) {
    for (int j = startInd; j < endInd; j++) {
        auto temp_way = waysArr[j];

        double levels = defaultLevel;
        if (temp_way.tags.find("levels") != temp_way.tags.end())
            levels = std::stod(temp_way.tags["levels"]);

        double dlon_shadow = (-height / EarthPerimeter * 360 * levels * cos(toRad(azim)) / tan(toRad(elev)));
        double dlat_shadow = (-height / EarthPerimeter * 360 * levels * sin(toRad(azim)) / tan(toRad(elev)));

        int dx_shadow = (int) round(dlat_shadow / dlat);
        int dy_shadow = (int) round(dlon_shadow / dlon);

        for (int i = 1; i < temp_way.seq.size(); i++) {
            double lat1 = temp_way.seq[i - 1].lat;
            double lon1 = temp_way.seq[i - 1].lon;
            double lat2 = temp_way.seq[i].lat;
            double lon2 = temp_way.seq[i].lon;

            iPnt p1 = pntToPnt(lat1, lon1);
            iPnt p2 = pntToPnt(lat2, lon2);

            auto marked = pntsUnderLine(p1, p2);
            plotPnts(marked);

            for (auto p: marked) {
                int x_new = p.x + dx_shadow;
                x_new = std::max(0, x_new);
                x_new = std::min(x_new, n_lon - 1);
                int y_new = p.y + dy_shadow;
                y_new = std::max(0, y_new);
                y_new = std::min(y_new, n_lat - 1);

                plotPnts(pntsUnderLine(p, {x_new, y_new}));
            }
        }
    }
}

void Grid::fillIn(int numberOfThreads) {
    if (numberOfThreads == -1) {
        numberOfThreads = (int) std::thread::hardware_concurrency(); // number of processor threads
        if (numberOfThreads == 0)
            numberOfThreads = 8;
    }

    std::vector<std::thread> threads;

    for (int i = 0; i < numberOfThreads; i++) {
        int startInd = i * (int) waysArr.size() / numberOfThreads;
        int endInd = (i + 1) * (int) waysArr.size() / numberOfThreads;

        if (i == numberOfThreads - 1)
            endInd = (int) waysArr.size();

        threads.emplace_back(&Grid::_fillIn, this, startInd, endInd);
    }

    for (auto &t: threads) {
        t.join();
    }
}

double Grid::_shadowPerc(iPnt p1, iPnt p2) {
    auto points = pntsUnderLine(p1, p2);

    int shadowPoints = 0;
    for (auto p: points) {
        if (getColor(p) == 1)
            shadowPoints += 1;
    }

    return shadowPoints / (double) points.size();
}

double Grid::shadowPerc(GraphNode node1, GraphNode node2) {
    return _shadowPerc(pntToPnt(node1.x, node1.y), pntToPnt(node2.x, node2.y));
}

[[maybe_unused]] void Grid::print_grid() {
    for (auto &row: grid) {
        for (auto &cell: row) {
            if (cell == 0)
                std::cout << ".";
            else
                std::cout << "@";
        }
        std::cout << std::endl;
    }
}
