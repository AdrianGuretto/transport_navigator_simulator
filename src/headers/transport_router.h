#pragma once

#include "router.h"
#include "transport_catalogue.h"

namespace Transportation{

struct RouterConfig{
    double bus_velocity = 0;
    int bus_wait_time = 0;
};

enum class RouteItemType{
    WAIT, BUS
};

struct RouteItem{
    RouteItemType type;
    std::string_view name;
    int span_count;
    double spent_time;
};

struct RouteResponse{
    bool success = false;
    std::string error = "N/A"; // for debugging purpuses
    double total_time = 0.0;
    std::vector<RouteItem> route_items;
};

// Transportatin Router class.
class Router{
public:
    using Graph = graph::DirectedWeightedGraph<double>;
    using InRouter = graph::Router<double>;
    using Edge = graph::Edge<double>;
    /**
     * @param config A configuration structure for the transport router.
     * @param transp_db A transport database.
    */
    Router(RouterConfig&& config, const TransportCatalogue& transp_db) : config_(std::move(config)) {
        BuildGraph(transp_db);
    }

    /** Main router building method. Builds a graph with stops and routes.
     * @param transp_db A transport database to build from
    */
    void BuildGraph(const TransportCatalogue& transp_db);
    
    /** Finds the shortest route from one stop to another.
     * @param stop_from A stop to build a route from.
     * @param stop_to A stop to build a route to.
     * @returns A response in a form of RouteResponse struct.
    */
    RouteResponse FindRoute(const std::string& stop_from, const std::string& stop_to);

private:
    constexpr static const double KMH_TO_MM_COEF = 100.0 / 6.0;
    
    /** Creates edges for stops (arrival and boarding vertecies)
     * @param stops A vector containing pointers to stops.
     * @param init_graph A graph to create edges on.
    */
    void CreateStopEdges(const std::vector<const Stop*>& stops, Graph& init_graph);

    /** Creates edges for routes off of the created stop edges.
     * @param transp_db A transport database.
     * @param buses A vector containing pointers to buses.
     * @param init_graph A graph to create route edges on.
    */
    void CreateRouteEdges(const TransportCatalogue& transp_db, const std::vector<const Bus*>& buses, Graph& init_graph);

    RouterConfig config_;
    Graph graph_;
    std::unique_ptr<InRouter> router_;

    std::unordered_map<std::string, graph::VertexId> stopname_to_vid_;
    std::unordered_map<graph::EdgeId, RouteItem> edgeid_to_item_;

};

} // namespace Transportation
