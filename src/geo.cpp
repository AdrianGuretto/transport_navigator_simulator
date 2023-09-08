#define _USE_MATH_DEFINES
#include "headers/geo.h"

#include <cmath>

namespace geo {

double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    if (from == to) {
        return 0;
    }
    static const double dr = M_PI / 180.;
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
        * 6371000;
}

size_t CoordinatesHasher::operator()(const Coordinates& coords) const noexcept{
    std::hash<double> hasher;
    size_t lat_hash = hasher(coords.lat);
    size_t lng_hash = hasher(coords.lng);
    
    return lat_hash ^ (lng_hash + 0x9e3779b9 + (lat_hash << 6) + (lat_hash >> 2));
}
}  // namespace geo