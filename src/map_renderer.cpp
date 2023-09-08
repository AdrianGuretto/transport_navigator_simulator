#include "headers/map_renderer.h"

namespace map_renderer{

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

MapRenderer::MapRenderer(double width, double height, double padding, double line_width, double stop_radius, int bus_label_font_size,
                    std::pair<double, double> bus_label_offset, double stop_label_font_size, std::pair<double, double> stop_label_offset,
                    svg::Color underlayer_color, double underlayer_width, std::deque<svg::Color>&& color_palette, const Transportation::TransportCatalogue& transport_db)
                    :
                    width_(width), height_(height), padding_(padding), line_width_(line_width), stop_radius_(stop_radius), bus_label_font_size_(bus_label_font_size),
                    bus_label_offset_(bus_label_offset), stop_label_font_size_(stop_label_font_size), stop_label_offset_(stop_label_offset),
                    underlayer_color_(underlayer_color), underlayer_width_(underlayer_width), color_palette_(std::move(color_palette)), transport_db_(transport_db){
                        std::vector<geo::Coordinates> stops_coords;
                        for (const Stop* stop : transport_db_.GetUsedStops()){
                            stops_coords.push_back(stop->coordinates);
                        }
                        geo_proj_ = SphereProjector(stops_coords.begin(), stops_coords.end(), width_, height_, padding_);
                    }


void MapRenderer::DrawStops(){
    std::vector<const Stop*> stops = transport_db_.GetUsedStops();

    for (const Stop* stop : stops){
        svg_document_.Add(svg::Circle{}.SetCenter(geo_proj_(stop->coordinates)).SetFillColor("white").SetRadius(stop_radius_));
    }

    for (const Stop* stop : stops){
        svg::Point stop_pos = geo_proj_(stop->coordinates);
        svg::Point stop_offset = {stop_label_offset_.first, stop_label_offset_.second};

        svg_document_.Add(svg::Text{}.SetPosition(stop_pos).SetOffset(stop_offset).SetFontSize(stop_label_font_size_).SetFontFamily("Verdana").SetData(stop->name).SetFillColor(underlayer_color_).SetStrokeColor(underlayer_color_).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetStrokeWidth(underlayer_width_));
        svg_document_.Add(svg::Text{}.SetPosition(stop_pos).SetOffset(stop_offset).SetFontSize(stop_label_font_size_).SetFontFamily("Verdana").SetData(stop->name).SetFillColor("black"));
    }
}

void MapRenderer::DrawRoutes(){
    size_t color_palette_index = 0;
    const size_t color_palette_size = color_palette_.size();
    const auto index_check = [&color_palette_index, &color_palette_size](){
        if (color_palette_index >= color_palette_size){
            color_palette_index = 0;
        }
    };

    std::vector<const Bus*> buses = transport_db_.GetAllBuses();

    for (const Bus* bus : buses){
        index_check();

        svg::Polyline line; 

        line.SetFillColor(std::string("none"))
            .SetStrokeColor(color_palette_.at(color_palette_index))
            .SetStrokeWidth(line_width_)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        const std::vector<Stop*>& bus_stops = bus->stops;

        for (const Stop* stop : bus_stops){
            if (stop){
                line.AddPoint(geo_proj_(stop->coordinates));
            }
        }

        svg_document_.Add(std::move(line));
        ++color_palette_index;
    }
    color_palette_index = 0;

    for (const Bus* bus : buses){
        index_check();
        svg::Point first_text_pos = geo_proj_(bus->stops[0]->coordinates);
        svg::Point text_offset{bus_label_offset_.first, bus_label_offset_.second};

        const auto add_route_text = [&](svg::Point text_pos){
            svg_document_.Add(svg::Text{}
                .SetPosition(text_pos)
                .SetOffset(text_offset)
                .SetFontSize(bus_label_font_size_)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetData(bus->name)
                .SetFillColor(underlayer_color_)
                .SetStrokeColor(underlayer_color_)
                .SetStrokeWidth(underlayer_width_)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
            svg_document_.Add(svg::Text{}
                .SetPosition(text_pos)
                .SetOffset(text_offset)
                .SetFontSize(bus_label_font_size_)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold")
                .SetData(bus->name)
                .SetFillColor(color_palette_.at(color_palette_index)));
        };

        add_route_text(first_text_pos);
        size_t middle_stop_index = bus->stops.size() / 2;
        
        if (!bus->round_route && bus->stops.at(middle_stop_index) != bus->stops.at(0)){
            add_route_text(geo_proj_(bus->stops.at(middle_stop_index)->coordinates));
        }
        ++color_palette_index;
    }

}


void MapRenderer::Render(std::ostream& out_stream){
    DrawRoutes();
    DrawStops();
    svg_document_.Render(out_stream);
}
 // namespace map_renderer
}