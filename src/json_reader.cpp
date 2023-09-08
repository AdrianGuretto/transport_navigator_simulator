#include "headers/json_reader.h"

/* --------- TC_QueryHandler CONSTRUCTOR --------- */
TC_QueryHandler::TC_QueryHandler(Transportation::TransportCatalogue& transp_catalogue) : db_(transp_catalogue) {
    stat_reqs_output_.reserve(50);
}

/* --------- BASE REQUESTS HANDLING --------- */
void TC_QueryHandler::AddStop(const json::Dict& stop){
    base_stop_reqs.push_back(stop);
}
void TC_QueryHandler::AddBus(const json::Dict& bus){
    base_bus_reqs.push_back(bus);
}

void TC_QueryHandler::ProcessBaseRequests(){
    std::deque<std::tuple<Stop*, std::string_view, int>> stops_to_distance;
    for (const json::Dict& stop : base_stop_reqs){
        std::string stop_name = stop.at("name").AsString();
        geo::Coordinates stop_coords{stop.at("latitude").AsDouble(), stop.at("longitude").AsDouble()};
        db_.AddStop(std::string(stop_name), std::move(stop_coords));
        Stop* stop_ptr = db_.FindStop(stop_name);

        for (const auto& [name, dist] : stop.at("road_distances").AsDict()){
            std::string_view road_stop_name(name);
            Stop* stop2_ptr = db_.FindStop(name);
            if (!stop2_ptr){
                stops_to_distance.push_back({stop_ptr, road_stop_name, dist.AsInt()});
                continue;
            }
            db_.SetStopDistance(stop_ptr, stop2_ptr, static_cast<uint32_t>(dist.AsInt()));
        }
    }
    for (const auto& [stop1_ptr, stop2_name, dist] : stops_to_distance){
        Stop* stop2_ptr = db_.FindStop(stop2_name);
        if (stop2_ptr){
            db_.SetStopDistance(stop1_ptr, stop2_ptr, static_cast<uint32_t>(dist));
        }
    }

    for (const json::Dict& bus : base_bus_reqs){
        std::string bus_name(bus.at("name").AsString());
        const json::Array& stop_arr = bus.at("stops").AsArray();
        std::vector<Stop*> bus_stops;
        bus_stops.reserve(stop_arr.size());

        for (const json::Node& stop_node : stop_arr){
            Stop* stop_ptr = db_.FindStop(std::string_view(stop_node.AsString()));
            bus_stops.push_back(stop_ptr);
        }
        db_.AddBus(std::move(bus_name), std::move(bus_stops), bus.at("is_roundtrip").AsBool());
    }
}

/* --------- STAT REQUESTS HANDLING --------- */
void TC_QueryHandler::AddStatStopRequest(const json::Dict& stop_req){
    using namespace json;

    Builder req_dict_builder;

    int id = stop_req.at("id").AsInt();
    
    std::string_view stop_name(stop_req.at("name").AsString());
    Stop* found_stop = db_.FindStop(stop_name);
    if (found_stop){
        const std::deque<Bus*> bus_list = db_.GetStopBusesList(found_stop);
        Array stop_buses;
        stop_buses.reserve(bus_list.size());
        for (const Bus* bus : bus_list){
            stop_buses.push_back(Node{bus->name});
        }
        req_dict_builder.StartDict()
                            .Key("buses").Value(std::move(stop_buses))
                            .Key("request_id").Value(id)
                        .EndDict();
    }
    else{
        req_dict_builder.StartDict()
                            .Key("request_id").Value(id)
                            .Key("error_message").Value("not found")
                        .EndDict();
    }


    stat_reqs_output_.push_back(Node{std::move(req_dict_builder.Build())});
}
void TC_QueryHandler::AddStatBusRequest(const json::Dict& bus_req){
    using namespace json;

    Builder req_dict_builder;
    int id = bus_req.at("id").AsInt();

    std::string_view bus_name(bus_req.at("name").AsString());
    BusResponse found_bus = db_.GetRoute(bus_name);
    if (found_bus.success){
        req_dict_builder.StartDict()
                            .Key("curvature").Value(found_bus.C_route_length)
                            .Key("request_id").Value(id)
                            .Key("route_length").Value(found_bus.L_route_length)
                            .Key("stop_count").Value(static_cast<int>(found_bus.stops_number))
                            .Key("unique_stop_count").Value(static_cast<int>(found_bus.unique_stops_number))
                        .EndDict();
    }
    else{
        req_dict_builder.StartDict()
                            .Key("request_id").Value(id)
                            .Key("error_message").Value("not found")
                        .EndDict();
    }

    stat_reqs_output_.push_back(Node{std::move(req_dict_builder.Build())});
}

void TC_QueryHandler::AddStatRouteRequest(const json::Dict& route_req, const std::unique_ptr<Transportation::Router>& router){
    using namespace json;

    Builder route_resp_builder;
    int id = route_req.at("id").AsInt();
    std::string stop_from = route_req.at("from").AsString();
    std::string stop_to = route_req.at("to").AsString();

    Transportation::RouteResponse resp = router->FindRoute(stop_from, stop_to);
    if (!resp.success){
        route_resp_builder.StartDict()
                            .Key("request_id").Value(id)
                            .Key("error_message").Value("not found")
                        .EndDict();
    }
    else{
        Array route_items;
        route_items.reserve(resp.route_items.size());
        for (const Transportation::RouteItem& item : resp.route_items){
            Builder item_builder;
            if (item.type == Transportation::RouteItemType::WAIT){
                item_builder.StartDict()
                                .Key("type").Value("Wait")
                                .Key("stop_name").Value(std::string(item.name))
                                .Key("time").Value(item.spent_time)
                            .EndDict();
            }
            else if (item.type == Transportation::RouteItemType::BUS){
                item_builder.StartDict()
                                .Key("type").Value("Bus")
                                .Key("bus").Value(std::string(item.name))
                                .Key("span_count").Value(item.span_count)
                                .Key("time").Value(item.spent_time)
                            .EndDict();
            }
            route_items.push_back(Node{std::move(item_builder.Build())});
        }

        route_resp_builder.StartDict()
                            .Key("request_id").Value(id)
                            .Key("total_time").Value(resp.total_time)
                            .Key("items").Value(std::move(route_items))
                        .EndDict();
    }

    stat_reqs_output_.push_back(Node{std::move(route_resp_builder.Build())});
    
}

void TC_QueryHandler::AddStatMapRequest(const int request_id, const std::string& rendered_map){
    using namespace json;

    Builder map_resp_builder;
    map_resp_builder.StartDict()
                        .Key("map").Value(rendered_map)
                        .Key("request_id").Value(request_id)
                    .EndDict();
    stat_reqs_output_.push_back(Node{std::move(map_resp_builder.Build())});
    
}

void TC_QueryHandler::ProcessStatRequests(std::ostream& out){
    json::Print(json::Document(std::move(stat_reqs_output_)), out);
}

JSON_TC_Builder::JSON_TC_Builder(Transportation::TransportCatalogue& transport_cat) : transp_ct_(transport_cat), query_handler_(transp_ct_) {}

void JSON_TC_Builder::BuildBaseRequests(const json::Array& base_requests){
    for (const json::Node& base_req : base_requests){
        const json::Dict& base_req_map = base_req.AsDict();
        const std::string_view type(base_req_map.at("type").AsString());
        if (type == "Stop"){
            query_handler_.AddStop(base_req_map);
        }
        else if (type == "Bus"){
            query_handler_.AddBus(base_req_map);
        }
    }
    query_handler_.ProcessBaseRequests();
}

svg::Color JSON_TC_Builder::ParseColor(const json::Node::Value& value){
    {
        if (const json::Array* arr_ptr = std::get_if<json::Array>(&value)){
            if (arr_ptr->size() == 3){
                unsigned char red = arr_ptr->at(0).AsInt(), green = arr_ptr->at(1).AsInt(), blue = arr_ptr->at(2).AsInt();
                return svg::Color(svg::Rgb{red, green, blue});
            }
            else if (arr_ptr->size() == 4){
                unsigned char red = arr_ptr->at(0).AsInt(), green = arr_ptr->at(1).AsInt(), blue = arr_ptr->at(2).AsInt();
                double alpha = arr_ptr->at(3).AsDouble();
                return svg::Color(svg::Rgba{red, green, blue, alpha});
            }
            else{
                throw std::logic_error("'underlayer_color' takes either 3 colors (Rgb) or 3 colors and 1 alpha value (Rgba).");
            }
        }
        else if (const std::string* str_ptr = std::get_if<std::string>(&value)){
            return svg::Color(*str_ptr);
        }
        else{
            throw std::logic_error("'underlayer_color' can either be a string, Rgb or Rgba value.");
        }
    }
}

void JSON_TC_Builder::BuildStatRequests(const json::Array& stat_requests, std::ostream& out){
    for (const json::Node& stat_req : stat_requests){
        const json::Dict& stat_req_map = stat_req.AsDict();
        const std::string_view type(stat_req_map.at("type").AsString());

        if (type == "Stop"){
            query_handler_.AddStatStopRequest(stat_req_map);
        }
        else if (type == "Bus"){
            query_handler_.AddStatBusRequest(stat_req_map);
        }
        else if (type == "Route"){
            query_handler_.AddStatRouteRequest(stat_req_map, p_router_);
        }
        else if (type == "Map"){
            std::ostringstream os;
            p_map_rendered_->Render(os);
            query_handler_.AddStatMapRequest(stat_req_map.at("id").AsInt(), os.str());
        }
    }
    query_handler_.ProcessStatRequests(out);
}

void JSON_TC_Builder::BuildMap(const json::Dict& settings){

    double width = settings.at("width").AsDouble(), height = settings.at("height").AsDouble();
    double padding = settings.at("padding").AsDouble();
    double line_width = settings.at("line_width").AsDouble();
    double stop_radius = settings.at("stop_radius").AsDouble();
    int bus_label_font_size = settings.at("bus_label_font_size").AsInt(), stop_label_font_size = settings.at("stop_label_font_size").AsInt();
    
    const json::Array& bus_label_offset_arr = settings.at("bus_label_offset").AsArray();
    const json::Array& stop_label_offset_arr = settings.at("stop_label_offset").AsArray();
    std::pair<double, double> bus_label_offset = std::make_pair(bus_label_offset_arr.at(0).AsDouble(), bus_label_offset_arr.at(1).AsDouble());
    std::pair<double, double> stop_label_offset = std::make_pair(stop_label_offset_arr.at(0).AsDouble(), stop_label_offset_arr.at(1).AsDouble());

    svg::Color underlayer_color(ParseColor(settings.at("underlayer_color").GetValue()));
    

    double underlayer_width = settings.at("underlayer_width").AsDouble();
    std::deque<svg::Color> color_palette;
    {
        const json::Array& colors = settings.at("color_palette").AsArray();

        for (const json::Node& color_node : colors){
            color_palette.push_back(ParseColor(color_node.GetValue()));
        }
    }
    p_map_rendered_ = std::make_unique<map_renderer::MapRenderer>(width, height, padding, line_width, stop_radius, bus_label_font_size, std::move(bus_label_offset), stop_label_font_size, std::move(stop_label_offset), std::move(underlayer_color), underlayer_width, std::move(color_palette), transp_ct_);
}

void JSON_TC_Builder::BuildRouter(const json::Dict& settings){
    Transportation::RouterConfig config{
        .bus_velocity = settings.at("bus_velocity").AsDouble(),
        .bus_wait_time = settings.at("bus_wait_time").AsInt()
    };

    p_router_ = std::make_unique<Transportation::Router>(std::move(config), transp_ct_);
}

void JSON_TC_Builder::BuildData(std::ostream& out){
    if (!p_read_json_data_){
        throw std::logic_error("No JSON data has been read to build data from.");
    }

    BuildBaseRequests(p_read_json_data_->at("base_requests").AsArray());
    BuildMap(p_read_json_data_->at("render_settings").AsDict());
    BuildRouter(p_read_json_data_->at("routing_settings").AsDict());
    BuildStatRequests(p_read_json_data_->at("stat_requests").AsArray(), out);
}

void JSON_TC_Builder::ReadData(std::istream& in){
    json::Document json_data(json::Load(in));
    p_read_json_data_ = std::make_unique<json::Dict>(json_data.GetRoot().AsDict());
}
