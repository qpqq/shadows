#include "solar.hpp"

double to_rad(double x) {
    return x * M_PI / 180;
}

double to_deg(double x) {
    return x * 180 / M_PI;
}

std::pair<double, double>
solar_coord(double latit, double longit, int year, int mon, int mday, int hour, int min, int sec, double time_zone) {
    int a = (14 - mon) / 12;
    int y = year + 4800 - a;
    int m = mon + 12 * a - 3;
    int jul_day_n = mday + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;
    double jul_day =
            jul_day_n + (hour - 12) / (double) 24 + min / (double) 1440 + sec / (double) 86400 - time_zone / 24;
    double jul_cen = (jul_day - 2451545) / 36525;

    double mean_long = fmod(280.46646 + jul_cen * (36000.76983 + jul_cen * 0.0003032), 360);
    double mean_anom = 357.52911 + jul_cen * (35999.05029 - 0.0001537 * jul_cen);
    double eq_of_ctr = sin(to_rad(mean_anom)) * (1.914602 - jul_cen * (0.004817 + 0.000014 * jul_cen)) +
                       sin(to_rad(2 * mean_anom)) * (0.019993 - 0.000101 * jul_cen) +
                       sin(to_rad(3 * mean_anom)) * 0.000289;
    double true_long = mean_long + eq_of_ctr;
    double app_long = true_long - 0.00569 - 0.00478 * sin(to_rad(125.04 - 1934.136 * jul_cen));

    double obliq_mean =
            23 + (26 + ((21.448 - jul_cen * (46.815 + jul_cen * (0.00059 - jul_cen * 0.001813)))) / 60) / 60;
    double obliq_corr = obliq_mean + 0.00256 * cos(to_rad(125.04 - 1934.136 * jul_cen));

    double dec = to_deg(asin(sin(to_rad(obliq_corr)) * sin(to_rad(app_long))));

    double eccent = 0.016708634 - jul_cen * (0.000042037 + 0.0000001267 * jul_cen);
    double var_y = tan(to_rad(obliq_corr / 2)) * tan(to_rad(obliq_corr / 2));
    double eq_of_time = 4 * to_deg(var_y * sin(2 * to_rad(mean_long)) - 2 * eccent * sin(to_rad(mean_anom)) +
                                   4 * eccent * var_y * sin(to_rad(mean_anom)) * cos(2 * to_rad(mean_long)) -
                                   0.5 * var_y * var_y * sin(4 * to_rad(mean_long)) -
                                   1.25 * eccent * eccent * sin(2 * to_rad(mean_anom)));
    double local_time = ((sec / (double) 60 + min) / 60 + hour) / 24;
    double true_solar_time = fmod(local_time * 1440 + eq_of_time + 4 * longit - 60 * time_zone, 1440);
    double hour_angle;
    if (true_solar_time / 4 < 0)
        hour_angle = true_solar_time / 4 + 180;
    else
        hour_angle = true_solar_time / 4 - 180;

    double zenith = to_deg(acos(sin(to_rad(latit)) * sin(to_rad(dec)) +
                                cos(to_rad(latit)) * cos(to_rad(dec)) * cos(to_rad(hour_angle))));
    double elev = 90 - zenith;
    double refr;
    if (elev > 85)
        refr = 0;
    else if (elev > 5)
        refr = 58.1 / tan(to_rad(elev)) - 0.07 / pow(tan(to_rad(elev)), 3) + 0.000086 / pow(tan(to_rad(elev)), 5);
    else if (elev > -0.575)
        refr = 1735 + elev * (-518.2 + elev * (103.4 + elev * (-12.79 + elev * 0.711)));
    else
        refr = 20.772 / tan(to_rad(elev));
    refr /= 3600;
    double elev_corr = elev + refr;

    double azim;
    if (hour_angle > 0)
        azim = fmod(to_deg(acos(((sin(to_rad(latit)) * cos(to_rad(zenith))) - sin(to_rad(dec))) /
                                (cos(to_rad(latit)) * sin(to_rad(zenith))))) + 180, 360);
    else
        azim = fmod(540 - to_deg(acos(((sin(to_rad(latit)) * cos(to_rad(zenith))) - sin(to_rad(dec))) /
                                      (cos(to_rad(latit)) * sin(to_rad(zenith))))), 360);

    return {elev_corr, azim};
}

[[maybe_unused]] void solar_coord_test() {
    // TODO use chrono for fun

    double latit = (55.7263 + 55.7816) / 2;
    double longit = (37.6503 + 37.7864) / 2;
    double time_zone = 3;

    time_t now = time(nullptr);
    tm *ltm = localtime(&now);

    std::pair<double, double> coord = solar_coord(latit, longit, 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday,
                                                  ltm->tm_hour, ltm->tm_min, ltm->tm_sec, time_zone);

    printf("Solar elevation = %f \nSolar azimuth = %f\n", coord.first, coord.second);
}

