#pragma once
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <vector>
#include <string>
#include <algorithm>
#include <string_view>
#include <iostream>
#include <iomanip>
#include <optional>
#include <string>
#include <cassert>
#include <execution>
#include <sstream>
#include <functional>


#include "geo.h"
#include "domain.h"

using namespace std::string_view_literals;
using namespace std::string_literals;

namespace Transportation{
namespace detail {        
    void StripStr(std::string& str);

    class StopPairHasher{
    public:
        size_t operator()(const std::pair<const Stop*, const Stop*> hashing_pair) const noexcept;
    };
} // namespace detail

class TransportCatalogue{
public:
    TransportCatalogue() : dummy_map_({nullptr, }) {}
public: // --------- MODIFYING METHODS ---------
    void AddStop(std::string&& stop_name, const geo::Coordinates& coords);

    void AddBus(std::string&& bus_name, std::vector<Stop*> stops, const bool round_route);

    void SetStopDistance(const Stop* first_stop, const Stop* second_stop, const int distance);

public: // --------- QUERYING METHODS ---------
    BusResponse GetRoute(std::string_view bus_name) const noexcept;

    Stop* FindStop(std::string_view stop_name) const noexcept;

    int GetStopDistance(const Stop* first_stop, const Stop* second_stop) const noexcept;

    size_t GetStopCount() const noexcept;
    size_t GetBusCount() const noexcept;
    
    const std::deque<Bus*>& GetStopBusesList(const Stop* stop) noexcept;
    
    std::vector<const Bus*> GetAllBuses() const noexcept;

    std::vector<const Stop*> GetAllStops() const noexcept;
    std::vector<const Stop*> GetUsedStops() const noexcept;

    bool StopIsUsed(const Stop* stop) const noexcept;

private: // --------- HELPER METHODS ---------
    static double CountRouteLength(const std::vector<Stop*>& stops, const bool round_route){
        double route_length = 0;
        int stops_amount = stops.size();

        if (stops_amount < 2){
            return 0;
        }
        for (int i = 0; i <= stops_amount - 2; ++i){
            route_length += geo::ComputeDistance(stops[i]->coordinates, stops[i + 1]->coordinates);
        }

        return round_route == false ? route_length * 2 : route_length;
    }

private:// --------- FIELDS ---------
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, Bus*> busname_to_bus_;
    std::unordered_map<const Stop*, std::deque<Bus*>> stop_to_buses_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, int, detail::StopPairHasher> stoppair_to_distance_;

    std::deque<Bus*> dummy_map_;
};

} // namespace Transportation