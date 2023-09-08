#include "headers/domain.h"

BusResponse::BusResponse(std::string_view b_name, size_t b_stops_number, size_t b_unique_stops_number, double b_L_route_length, double b_C_route_length) 
: name(b_name), stops_number(b_stops_number), unique_stops_number(b_unique_stops_number), L_route_length(b_L_route_length), C_route_length(b_C_route_length), success(true) {}

BusResponse::BusResponse(std::string_view b_name) : name(b_name) {}