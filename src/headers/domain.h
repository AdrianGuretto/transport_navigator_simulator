// A file for defining database structs used across the app.

#pragma once
#include <string>
#include <vector>

#include "geo.h"

struct Stop{
    std::string name;
    geo::Coordinates coordinates;
};

struct Bus{
    std::string name;
    std::vector<Stop*> stops;
    double C_route_length;
    bool round_route;
};

struct BusResponse{
    BusResponse() = default;
    BusResponse(std::string_view b_name, size_t b_stops_number, size_t b_unique_stops_number, double b_L_route_length, double b_C_route_length);
    BusResponse(std::string_view b_name);
    
    std::string_view name;
    size_t stops_number;
    size_t unique_stops_number;
    double L_route_length;
    double C_route_length;
    bool success = false;
};
