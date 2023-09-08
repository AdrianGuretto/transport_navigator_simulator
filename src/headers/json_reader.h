#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_builder.h" 
#include "transport_router.h"

#include <memory>

// A class for handling `base` and `stat` requests to the DB.
class TC_QueryHandler{
public:
    TC_QueryHandler() = default;
    TC_QueryHandler(Transportation::TransportCatalogue& transp_catalogue);

public: // --------- BASE REQUESTS HANDLING --------- 
    void AddStop(const json::Dict& stop);
    void AddBus(const json::Dict& bus);

    // Builds transport database from the base requests.
    void ProcessBaseRequests();

public: // --------- STAT REQUESTS HANDLING --------- 
    void AddStatStopRequest(const json::Dict& stop_req);
    void AddStatBusRequest(const json::Dict& bus_req);
    void AddStatRouteRequest(const json::Dict& route_req, const std::unique_ptr<Transportation::Router>& router);
    void AddStatMapRequest(const int request_id, const std::string& rendered_map);

    // Outputs processed stat requests to `out` stream.
    void ProcessStatRequests(std::ostream& out);
    
private: // --------- FIELDS ---------
    Transportation::TransportCatalogue& db_;
    std::deque<json::Dict> base_stop_reqs, base_bus_reqs;
    json::Array stat_reqs_output_;
};

// A class for building transport database off of JSON data from an input stream.
class JSON_TC_Builder{
public: // --------- CONSTRUCTORS ---------
    explicit JSON_TC_Builder() = default;
    explicit JSON_TC_Builder(Transportation::TransportCatalogue& transport_cat);

public: // --------- METHODS ---------
    // Builds data, processes stat_requests, and outputs the responses to `out`.
    void BuildData(std::ostream& out);

    // Reads JSON data from `in` input stream.
    void ReadData(std::istream& in);
private: // --------- HELPER METHODS ---------

    void BuildMap(const json::Dict& settings);
    void BuildRouter(const json::Dict& settings);
    void BuildBaseRequests(const json::Array& base_requests);
    void BuildStatRequests(const json::Array& stat_requests, std::ostream& out);

    // Parses color from a node, if `value` is either an Array or a String.
    static svg::Color ParseColor(const json::Node::Value& value);

private: // --------- FIELDS ---------
    Transportation::TransportCatalogue& transp_ct_;
    TC_QueryHandler query_handler_;
    std::unique_ptr<map_renderer::MapRenderer> p_map_rendered_;
    std::unique_ptr<json::Dict> p_read_json_data_;
    std::unique_ptr<Transportation::Router> p_router_;
};
