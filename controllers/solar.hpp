#pragma once

#include <cmath>
#include <utility>
#include <cstdio>
#include <ctime>

/**
 * The perimeter of the Earth in meters
 */
const double EarthPerimeter = 2 * M_PI * 6378 * 1000;

/**
 * Converts degrees to radians.
 */
double toRad(double x);

/**
 * Converts radians to degrees.
 */
double toDeg(double x);

/**
 * Calculates the altitude and azimuth of the sun.
 * @param latit [-90, 90]
 * @param longit [-90, 90]
 * @param year current year
 * @param mon [1, 12]
 * @param mday [1, how_many_days_in_month]
 * @param hour [0, 23]
 * @param min [0, 59]
 * @param sec [0, 59]
 * @param time_zone in hours
 * @return elevation and azimuth respectively
 * @see https://gml.noaa.gov/grad/solcalc/
 */
std::pair<double, double>
solarCoords(double latit, double longit, int year, int mon, int mday, int hour, int min, int sec, double time_zone);

/**
 * Returns solar coordinates at the moment
 * @param lat latitude of the observation point
 * @param lon longitude of the observation point
 * @param time_zone time zone of the observation point
 * @return elevation and azimuth respectively
 */
std::pair<double, double> getSolarCoords(double lat, double lon, double time_zone = 3);
