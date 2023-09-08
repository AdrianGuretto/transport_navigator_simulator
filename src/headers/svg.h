#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <deque>
#include <optional>
#include <unordered_map>
#include <variant>


namespace svg {

struct Rgb{
    int red, green, blue;
};

struct Rgba{
    int red, green, blue;
    double alpha;
};

using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap);
std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join);
std::ostream& operator<<(std::ostream& out, const Color& color);

const inline std::unordered_map<char, std::string> __SVG_CHAR_MAP = {{'\"', "&quot;"}, {'\'', "&apos;"}, {'<', "&lt;"}, {'>', "&gt;"}, {'&', "&amp;"}};

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};


/*
 * Helper struct for storing SVG document context with indentation.
 * Stores a link to the output stream, current value, and the indentation step value.
*/
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

// Abstract base class for unifying objects data storage of SVG document's tags.
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

// A class for manipulating SVG stroke (path) properties.
template<typename Owner>
class PathProps{
public:
    Owner& SetFillColor(Color color){
        fill_color_ = std::move(color);
        return __AsOwner();
    }
    Owner& SetStrokeColor(Color color){
        stroke_color_ = std::move(color);
        return __AsOwner();
    }
    Owner& SetStrokeWidth(double width){
        stroke_width_ = width;
        return __AsOwner();
    }
    Owner& SetStrokeLineCap(StrokeLineCap line_cap){
        line_cap_ = std::move(line_cap);
        return __AsOwner();
    }
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join){
        line_join_ = std::move(line_join);
        return __AsOwner();
    }
protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const{
        if (fill_color_) out << " fill=\"" << *fill_color_ << "\"";
        if (stroke_color_) out << " stroke=\"" << *stroke_color_ << "\"";
        if (stroke_width_) out << " stroke-width=\"" << *stroke_width_ << "\"";
        if (line_cap_){
            out << " stroke-linecap=\"" << *line_cap_ << "\"";
        } 
        if (line_join_){
            out << " stroke-linejoin=\"" << *line_join_ << "\"";
        }
    }
private:
    Owner& __AsOwner(){
        return static_cast<Owner&>(*this);
    }
private: // --------- FIELDS ---------

    std::optional<Color> fill_color_, stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;
};


// https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

// https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
class Polyline final: public Object, public PathProps<Polyline> {
public:
    // Adds a vertex to the current polyline.
    Polyline& AddPoint(Point point);

private:
    void RenderObject(const RenderContext& context) const override;

    std::vector<Point> points_;
};


// https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
class Text : public Object, public PathProps<Text> {
public:
    Text& SetPosition(Point pos);

    Text& SetOffset(Point offset);

    Text& SetFontSize(uint32_t size);

    Text& SetFontFamily(std::string font_family);

    Text& SetFontWeight(std::string font_weight);

    Text& SetData(std::string data);

private:
    void RenderObject(const RenderContext& context) const override;

    Point pos_ = {0.0, 0.0}, offset_ = {0.0, 0.0};
    uint32_t font_size_ = 1;
    std::string font_family_, font_weight_;
    std::string data_;
};

class ObjectContainer{
public:
    template<class Obj>
    void Add(Obj obj){
        AddPtr(std::move(std::make_unique<Obj>(std::move(obj))));
    }

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    virtual ~ObjectContainer() = default;
};

class Document : public ObjectContainer{
public:

    // Adds an svg::Object to the SVG document.
    void AddPtr(std::unique_ptr<Object>&& obj);

    // Outputs document representation to `out` stream.
    void Render(std::ostream& out) const;

private:
    std::deque<std::unique_ptr<Object>> objects_;
};

// An abstract class for objects which can be drawn.
class Drawable{
public:
    virtual void Draw(ObjectContainer& container) const = 0;

    virtual ~Drawable() = default;
};

}  // namespace svg