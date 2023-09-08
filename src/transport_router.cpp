#include "headers/transport_router.h"

namespace Transportation{

void Router::BuildGraph(const TransportCatalogue& transp_db){
    const std::vector<const Stop*> stops_list = transp_db.GetAllStops();
    const std::vector<const Bus*> buses_list = transp_db.GetAllBuses();
    Graph init_graph(stops_list.size() * 2);


    CreateStopEdges(stops_list, init_graph); // creating two edges for one stop: start and boarding
    CreateRouteEdges(transp_db, buses_list, init_graph); // creating routes for the stop edges

    graph_ = std::move(init_graph);
    router_ = std::make_unique<InRouter>(graph_);
}


void Router::CreateStopEdges(const std::vector<const Stop*>& stops, Graph& init_graph){
    graph::VertexId vid = 0;

    for (const Stop* stop : stops){ 
        stopname_to_vid_[stop->name] = vid;
        const graph::EdgeId stop_edge = init_graph.AddEdge({
            .from = vid,
            .to = ++vid,
            .span_count = 0,
            .weight = static_cast<double>(config_.bus_wait_time)
        });

        edgeid_to_item_[stop_edge] = RouteItem{
            .type = RouteItemType::WAIT,
            .name = stop->name,
            .span_count = 0,
            .spent_time = static_cast<double>(config_.bus_wait_time)
        };
        ++vid;
    }
}

void Router::CreateRouteEdges(const TransportCatalogue& transp_db, const std::vector<const Bus*>& buses, Graph& init_graph){
    for (const Bus* bus : buses){
        std::vector<Stop*> bus_stops = bus->stops;
        if (!bus->round_route){
            bus_stops.resize(bus_stops.size() / 2 + 1); // since stops in the bus info are stored as they are on the road, we get rid of the inversed ones
        }

        size_t stops_count = bus_stops.size();
        for (size_t i = 0; i < stops_count; ++i){ // bridging stops on a route to each other
            const Stop* first_stop = bus_stops.at(i);
            for (size_t j = i + 1; j < stops_count; ++j){
                const Stop* second_stop = bus_stops.at(j);

                double time = 0;
                double inverse_time = 0;
                for (size_t n = i + 1; n <= j; ++n){ // calculate ride time from one stop to another
                    time += transp_db.GetStopDistance(bus_stops.at(n - 1), bus_stops.at(n)) / (config_.bus_velocity * KMH_TO_MM_COEF);
                    inverse_time += transp_db.GetStopDistance(bus_stops.at(n), bus_stops.at(n - 1)) / (config_.bus_velocity * KMH_TO_MM_COEF);
                }

                const graph::EdgeId eid = init_graph.AddEdge({ // create an edge from first stop (boarding allias) to the second one (start allias)
                    .from = stopname_to_vid_.at(first_stop->name) + 1,
                    .to = stopname_to_vid_.at(second_stop->name),
                    .span_count = static_cast<int>(j - i),
                    .weight = time
                });
                edgeid_to_item_[eid] = RouteItem{ // map edgeID to RouteItem
                    .type = RouteItemType::BUS,
                    .name = bus->name,
                    .span_count = static_cast<int>(j - i),
                    .spent_time = time
                };

                if (!bus->round_route){ // if the route is not round, then we will construct inverse route from the second stop (boarding) to the first (start)
                    const graph::EdgeId inv_eid = init_graph.AddEdge({
                        .from = stopname_to_vid_.at(second_stop->name) + 1,
                        .to = stopname_to_vid_.at(first_stop->name),
                        .span_count = static_cast<int>(j - i),
                        .weight = inverse_time
                    });
                    edgeid_to_item_[inv_eid] = RouteItem{
                        .type = RouteItemType::BUS,
                        .name = bus->name,
                        .span_count = static_cast<int>(j - i),
                        .spent_time = inverse_time
                    };
                }
            }
        } 
    }
}

RouteResponse Router::FindRoute(const std::string& stop_from, const std::string& stop_to){
    if (graph_.GetEdgeCount() == 0){
        return RouteResponse{.error = "[!] The graph contains no connecting edges."};
    }
    if (!stopname_to_vid_.count(stop_from) || !stopname_to_vid_.count(stop_to)){
        return RouteResponse{.error = "[!] One of the provided stops does not exist."};
    }

    graph::VertexId stop_from_vid = stopname_to_vid_.at(stop_from), stop_to_vid = stopname_to_vid_.at(stop_to);
    std::optional<InRouter::RouteInfo> built_data = router_->BuildRoute(stop_from_vid, stop_to_vid);

    if (!built_data){
        return RouteResponse{.error = "[!] Failed to build route."};
    }

    RouteResponse resp;
    resp.success = true;
    resp.total_time = built_data->weight;
    resp.route_items.reserve(built_data->edges.size());

    for (const graph::EdgeId eid : built_data->edges){
        resp.route_items.push_back(edgeid_to_item_.at(eid));
    }

    return resp;
}

} // namespace Transportation