#include "headers/svg.h"

namespace svg {
    
std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap){
        switch(line_cap){
            case StrokeLineCap::BUTT:
                out << "butt";
                break;
            case StrokeLineCap::ROUND:
                out << "round";
                break;
            case StrokeLineCap::SQUARE:
                out << "square";
                break;
            default: break;
        }
    return out;
}
std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join){
    switch(line_join){
        case StrokeLineJoin::ARCS:
            out << "arcs";
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel";
            break;
        case StrokeLineJoin::ROUND:
            out << "round";
            break;
        case StrokeLineJoin::MITER:
            out << "miter";
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip";
            break;
        default: break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const Color& color){
    if (const std::string* p_str = std::get_if<std::string>(&color)){
        out << *p_str;
    }
    else if (std::holds_alternative<Rgb>(color)){
        Rgb p_rgb = std::get<Rgb>(color);
        out << "rgb(" << p_rgb.red << ',' << p_rgb.green << ',' << p_rgb.blue << ')';
    }
    else if (std::holds_alternative<Rgba>(color)){
        Rgba p_rgba = std::get<Rgba>(color);
        out << "rgba(" << p_rgba.red << ',' << p_rgba.green << ',' << p_rgba.blue << ',' << p_rgba.alpha << ')';
    }
    return out;
}
    
using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ------------ Polyline ----------------

Polyline& Polyline::AddPoint(Point point){
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const{
    auto& out = context.out;
    out << "<polyline points=\"";
    
    bool first = true;
    for (const Point& point : points_){
        if (first){
            out << point.x << ',' << point.y;
            first = false;
        }
        else{
            out << ' ' << point.x << ',' << point.y;
        }
    }
    out << "\"";
    RenderAttrs(out);
    out << "/>";
}


// -------------- Text ------------------
Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}
Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}
Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const{
    auto& out = context.out;
    out << "<text";
    RenderAttrs(out);
    out << " x=\"" << pos_.x << "\" y=\"" << pos_.y << "\" dx=\"" << offset_.x << "\" dy=\"" << offset_.y << "\" font-size=\"" << font_size_ << "\"";
    if (!font_family_.empty()) out << " font-family=\"" << font_family_ << "\"";
    if (!font_weight_.empty()) out << " font-weight=\"" << font_weight_ << "\"";
    out << '>';

    for (const char c : data_){
        if (__SVG_CHAR_MAP.count(c)){
            out << __SVG_CHAR_MAP.at(c);
        }
        else{
            out << c;
        }
    }

    out << "</text>";
}

// ---------------------------------

// Добавляет в svg-документ объект-наследник svg::Object
void Document::AddPtr(std::unique_ptr<Object>&& obj){
    objects_.push_back(std::move(obj));
}

// Выводит в ostream svg-представление документа
void Document::Render(std::ostream& out) const{
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";
    for (const std::unique_ptr<Object>& obj : objects_){
        obj->Render(out);
    }
    out << "</svg>";
}

}  // namespace svg