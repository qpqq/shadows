#pragma once

#include <vector>
#include <iostream>
#include <cmath>
#include <thread>

#include "database.hpp"
#include "solar.hpp"

/**
 * Point on the grid
 */
struct iPnt {
    int x;
    int y;
    double color;
};

/**
 * y (lat)
 * ^
 * |
 * |
 * |
 * |
 * |
 * | - - - - - - - > x (lon)
 */

class Grid {

private:

    std::vector<Way> waysArr;

    double step{};

    double min_lat{}, max_lat{};
    double min_lon{}, max_lon{};

    int n_lat{};
    int n_lon{};
    int n_x{};
    int n_y{};

    double dlat{};
    double dlon{};

    double alpha = 0.01;

    double height = 3;

    double elev = 1, azim = 180;

    std::vector<std::vector<double>> grid;

public:

    Grid();

    Grid(std::vector<std::vector<std::string>> coords, double offset, std::vector<Way> ArrOfWays, double gridStep);

    double getColor(iPnt p);

    void plot(iPnt p);

    void plotPnts(const std::vector<iPnt> &points);

    [[nodiscard]] iPnt pntToPnt(double lat, double lon) const;

    static std::vector<iPnt> pntsUnderLineLow(iPnt p1, iPnt p2);

    static std::vector<iPnt> pntsUnderLineHigh(iPnt p1, iPnt p2);

    static std::vector<iPnt> pntsUnderLine(iPnt p1, iPnt p2);

    void _fillIn(int start_ind, int final_ind);

    void fillIn(int numberOfThreads = -1);

    double _shadowPerc(iPnt p1, iPnt p2);

    double shadowPerc(GraphNode node1, GraphNode node2);

    [[maybe_unused]] void print_grid();
};
