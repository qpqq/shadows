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

// y (lat)
// ^
// |
// |
// |
// |
// |
// | - - - - - - - > x (lon)

/**
 * This class contains tools for building a grid with shadows
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

    double offsetOffset = 1000 * 360 / EarthPerimeter;

    double defaultLevel = 3;
    double height = 3;

    double elev{}, azim{};

    std::vector<std::vector<double>> grid;

public:

    /**
     * Default constractor
     */
    Grid();

    /**
     * Constructor initializing a grid with shadows
     * @param coords vector of coordinates specifying the grid size
     * @param offset the offset in radians from the boundaries set by coords. The grid will be built in this new border
     * @param ArrOfWays a vector of buildings containing ways
     * @param gridStep grid pitch in meters
     */
    Grid(std::vector<std::vector<std::string>> coords, double offset, std::vector<Way> ArrOfWays, double gridStep);

    /**
     * Get the value in a cell
     * @param p point
     * @return the value of this point
     */
    double getColor(iPnt p);

    /**
     * Fills the cell with the value
     * @param p point
     */
    void plot(iPnt p);

    /**
     * Fills a set of cells with their values
     * @param points vector of points
     */
    void plotPnts(const std::vector<iPnt> &points);

    /**
     * Converts coordinates to a grid point
     * @param lat latitude in degrees
     * @param lon longitude in degrees
     * @return point
     */
    [[nodiscard]] iPnt pntToPnt(double lat, double lon) const;

    /**
     * Returns the vector of points under the segment
     * @param p1 one of the vertices of the segment
     * @param p2 another of the vertices of the segment
     * @return vector of points
     * @note Works only for segments with an inclination of less than 45 degrees
     */
    static std::vector<iPnt> pntsUnderLineLow(iPnt p1, iPnt p2);

    /**
     * Returns the vector of points under the segment
     * @param p1 one of the vertices of the segment
     * @param p2 another of the vertices of the segment
     * @return vector of points
     * @note Works only for segments with an inclination of more than 45 degrees
     */
    static std::vector<iPnt> pntsUnderLineHigh(iPnt p1, iPnt p2);

    /**
     * Returns the vector of points under the segment
     * @param p1 one of the vertices of the segment
     * @param p2 another of the vertices of the segment
     * @return vector of points
     */
    static std::vector<iPnt> pntsUnderLine(iPnt p1, iPnt p2);

    /**
     * Declares grid rows from startInd to endInd
     * @param startInd the first index of the declaration
     * @param endInd the last index of the declaration
     * @param color default value of grid cells
     */
    void _makeGrid(int startInd, int endInd, double color);

    /**
     * Declares grid rows. Multithreading implementation
     * @param color default value of grid cells
     * @param numberOfThreads the number of threads
     * @note By default, the function itself asks the system how many threads it has
     */
    void makeGrid(double color, int numberOfThreads = -1);

    /**
     * Marks shadows from buildings and their contours on the grid from the startInd to the endInd of the waysArr
     * @param startInd the first index of the waysArr
     * @param endInd the last index of the waysArr
     */
    void _fillIn(int startInd, int endInd);

    /**
     * Marks shadows from buildings and their contours on the grid. Multithreading implementation
     * @param numberOfThreads the number of threads
     * @note By default, the function itself asks the system how many threads it has
     */
    void fillIn(int numberOfThreads = -1);

    /**
     * Calculates the percentage of the segment that falls into the shaded cells
     * @param p1 one of the vertices of the segment
     * @param p2 another of the vertices of the segment
     * @return a number from 0 to 1 -- shading
     */
    double _shadowPerc(iPnt p1, iPnt p2);

    /**
     * Calculates the percentage of the segment that falls into the shaded cells
     * @param node1 one of the nodes of the segment
     * @param node2 another of the nodes of the segment
     * @return a number from 0 to 1 -- shading
     */
    double shadowPerc(GraphNode node1, GraphNode node2);

    /**
     * Prints the grid to the console
     */
    [[maybe_unused]] void print_grid();
};
