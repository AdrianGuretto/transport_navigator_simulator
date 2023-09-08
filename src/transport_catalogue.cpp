#include "headers/transport_catalogue.h"

namespace Transportation{
    namespace detail{
        void StripStr(std::string& str){
            str = str.substr(str.find_first_not_of(' '));
            str = str.substr(0, str.find_last_not_of(' ') + 1);
        }

        size_t StopPairHasher::operator()(const std::pair<const Stop*, const Stop*> hashing_pair) const noexcept{
            uint64_t num = std::hash<const void*>{}(hashing_pair.first) * 2 + std::hash<const void*>{}(hashing_pair.second) * 8;
            return static_cast<size_t>(num);
        }
    }

    void TransportCatalogue::AddStop(std::string&& stop_name, const geo::Coordinates& coords){
        assert(!stop_name.empty());
        Stop* stop = FindStop(stop_name);
        if (stop && stop->coordinates == coords){
            return;
        }
        stops_.push_back({stop_name, coords});
        Stop* stop_element = &stops_[stops_.size() - 1];
        stop_to_buses_[stop_element];
        stopname_to_stop_[std::string_view(stop_element->name)] = stop_element;
    }


    void TransportCatalogue::AddBus(std::string&& bus_name, std::vector<Stop*> stops, const bool round_route){ 
        assert((!bus_name.empty() && !stops.empty())); 
        if (busname_to_bus_.count(bus_name)){ 
            return; 
        } 
        double C_route_length = CountRouteLength(stops, round_route); 
        std::vector<Stop*> reversed_route(stops.begin(), stops.begin() + stops.size() - (round_route == true ? 0 : 1));             
        if (!round_route){ 
            reversed_route.insert(reversed_route.end(), stops.rbegin(), stops.rend()); 
        } 
        if (reversed_route.empty()){
            return;
        }

        buses_.push_back({bus_name, std::move(reversed_route), C_route_length, round_route}); 
        Bus* bus_element = &buses_.back(); 

        std::unordered_set<Stop*> tmp_stops; 
        for (Stop* stop : stops){ 
            if (stop_to_buses_.count(stop) && tmp_stops.count(stop) == 0){ 
                stop_to_buses_[stop].push_back(bus_element); 
                tmp_stops.insert(stop); 
            } 
        } 
        busname_to_bus_[std::string_view(bus_element->name)] = bus_element; 
    }

    BusResponse TransportCatalogue::GetRoute(std::string_view bus_name) const noexcept{ 
        assert(!bus_name.empty()); 
        if (busname_to_bus_.count(bus_name)){ 

            Bus* bus = busname_to_bus_.at(bus_name); 
            std::vector<Stop*>& stops = bus->stops; 
            double L_route_length = 0;  
            size_t stops_count = stops.size();  

            for (size_t i = 0; i < stops_count - 1; ++i){  
                unsigned int dist = GetStopDistance(stops[i], stops[i + 1]);  
                L_route_length += dist;  
            } 

            std::unordered_set<Stop*> unique_stops(stops.begin(), stops.end()); 
            return {std::string_view(bus->name), stops.size(), unique_stops.size(), L_route_length, (L_route_length / (bus->C_route_length * 1.0))}; 
        } 
        return {bus_name}; 
    } 

    Stop* TransportCatalogue::FindStop(std::string_view stop_name) const noexcept{
        assert(!stop_name.empty());
        if (stopname_to_stop_.count(stop_name)){
            return stopname_to_stop_.at(stop_name);
        }
        return nullptr; 
    }

    void TransportCatalogue::SetStopDistance(const Stop* first_stop, const Stop* second_stop, const int distance){
        if (first_stop != nullptr && second_stop != nullptr){
            stoppair_to_distance_[{first_stop, second_stop}] = distance;
        }
    }

    int TransportCatalogue::GetStopDistance(const Stop* first_stop, const Stop* second_stop) const noexcept{
        if (first_stop != nullptr && second_stop != nullptr){
            if (stoppair_to_distance_.count({first_stop, second_stop})){
                return stoppair_to_distance_.at({first_stop, second_stop});
            }
            else if (stoppair_to_distance_.count({second_stop, first_stop})){
                return stoppair_to_distance_.at({second_stop, first_stop});
            }
        }
        return 0;
    }

    size_t TransportCatalogue::GetStopCount() const noexcept{
        return stops_.size();
    }
    size_t TransportCatalogue::GetBusCount() const noexcept{
        return buses_.size();
    }

    const std::deque<Bus*>& TransportCatalogue::GetStopBusesList(const Stop* stop) noexcept{
        bool stop_exists = stop_to_buses_.count(stop) > 0;
        if (stop_exists){
            std::deque<Bus*>& stop_buses_list = stop_to_buses_.at(stop);
            std::sort(std::execution::par, stop_buses_list.begin(), stop_buses_list.end(), [](const Bus* left, const Bus* right){ return left->name < right->name; });
            return stop_buses_list;
        }
        return dummy_map_;
    }

    std::vector<const Bus*> TransportCatalogue::GetAllBuses() const noexcept{
        std::vector<const Bus*> buses_list;
        for (const Bus& bus : buses_){
            buses_list.push_back(&bus);
        }
        std::sort(std::execution::par, buses_list.begin(), buses_list.end(), [](const Bus* left, const Bus* right){ return left->name < right->name; });
        return buses_list;
    }

    std::vector<const Stop*> TransportCatalogue::GetAllStops() const noexcept{
        std::vector<const Stop*> stops;
        stops.reserve(stop_to_buses_.size());
        for (const auto& [stop, buses] : stop_to_buses_){
            stops.push_back(stop);
        }
        std::sort(std::execution::par, stops.begin(), stops.end(), [](const Stop* left, const Stop* right){ return left->name < right->name; });
        return stops;
    }

    std::vector<const Stop*> TransportCatalogue::GetUsedStops() const noexcept{
        std::vector<const Stop*> used_stops;
        used_stops.reserve(stops_.size());
        for (const Stop* stop : GetAllStops()){
            if (StopIsUsed(stop)){
                used_stops.push_back(stop);
            }
        }
        std::sort(std::execution::par, used_stops.begin(), used_stops.end(), [](const Stop* left, const Stop* right){ return left->name < right->name; });
        return used_stops;
    }

    bool TransportCatalogue::StopIsUsed(const Stop* stop) const noexcept{
        return stop_to_buses_.count(stop) && !stop_to_buses_.at(stop).empty();
    }
}