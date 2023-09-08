#pragma once

#include <variant>
#include <limits>
#include <algorithm>
#include <execution>
#include <deque>
#include <unordered_set>

#include "svg.h"
#include "transport_catalogue.h"

inline const double EPSILON = std::numeric_limits<double>::epsilon();
inline bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

namespace map_renderer{

class SphereProjector {
public:
    SphereProjector() = default;
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding)
    {
        if (points_begin == points_end) {
            return;
        }

        // Find points with minimal longitude.
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Find points with maximal latitude.
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Compute the scale coefficient across the x-axis.
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Compute the scale coefficient across the y-axis.
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            zoom_coeff_ = *height_zoom;
        }
    }

    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

class MapRenderer{
public:
    MapRenderer() = default;
    explicit MapRenderer(double width, double height, double padding, double line_width, double stop_radius, int bus_label_font_size,
                        std::pair<double, double> bus_label_offset, double stop_label_font_size, std::pair<double, double> stop_label_offset,
                        svg::Color underlayer_color, double underlayer_width, std::deque<svg::Color>&& color_palette, const Transportation::TransportCatalogue& transp_db);

    void Render(std::ostream& out_stream);


private: // --------- HELPER METHODS ---------

    void DrawRoutes();
    void DrawStops();

private: // --------- FIELDS ---------
    double width_ = 0, height_ = 0;
    double padding_ = 0;
    double line_width_ = 0, stop_radius_ = 0;

    int bus_label_font_size_ = 0;
    std::pair<double, double> bus_label_offset_;
    
    double stop_label_font_size_ = 0;
    std::pair<double, double> stop_label_offset_;

    svg::Color underlayer_color_;
    double underlayer_width_ = 0;

    std::deque<svg::Color> color_palette_;

    svg::Document svg_document_;

    const Transportation::TransportCatalogue& transport_db_;
    SphereProjector geo_proj_;

};
} // namespace map_renderer