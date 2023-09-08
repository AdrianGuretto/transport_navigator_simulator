#pragma once

#include <functional>

namespace geo {

struct Coordinates {
    double lat; // latitude
    double lng; // longitude
    bool operator==(const Coordinates& other) const {
        return lat == other.lat && lng == other.lng;
    }
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
};

struct CoordinatesHasher{
    CoordinatesHasher() = default;

    size_t operator()(const Coordinates& coords) const noexcept;
};

double ComputeDistance(Coordinates from, Coordinates to);


}  // namespace geo