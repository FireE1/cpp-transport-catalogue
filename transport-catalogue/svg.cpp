#include "svg.h"

#include <regex>

namespace svg {

using namespace std::literals;

inline void PrintColor(std::ostream& out, Rgb& rgb) {
    out << "rgb("sv << static_cast<short>(rgb.red) << ","sv
                    << static_cast<short>(rgb.green) << ","sv 
                    << static_cast<short>(rgb.blue) << ")"sv;
}
    
inline void PrintColor(std::ostream& out, Rgba& rgba) {
    out << "rgba("sv << static_cast<short>(rgba.red) << ","sv 
                     << static_cast<short>(rgba.green) << ","sv 
                     << static_cast<short>(rgba.blue) << ","sv 
                     << (rgba.alpha) << ")"sv;
}
    
inline void PrintColor(std::ostream& out, std::monostate) {
    out << "none"sv;
}
 
inline void PrintColor(std::ostream& out, std::string& color) {
    out << color;
}
    
std::ostream& operator<<(std::ostream& out, const Color& color) {
    std::visit([&out](auto value) {PrintColor(out, value);}, color);
    return out;
}

// ---------- Object ------------------

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
    out << R"(<circle cx=")" << center_.x << R"(" cy=")" << center_.y << R"(" )";
    out << R"(r=")" << radius_ << R"(")";
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << R"(<polyline points=")";
    bool is_first = true;
    for (const auto& point : points_)
    {
        if (!is_first)
        {
            out << " ";
        }
        out << point.x << ","sv << point.y;
        is_first = false;
    }
    out << R"(")";
    RenderAttrs(out);
    out << "/>";
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos) {
    position_ = pos;
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

std::string DataCorrect(const std::string& data) {
    std::string to_ret = data;
    to_ret = std::regex_replace(to_ret, std::regex("&"), "&amp;");
    to_ret = std::regex_replace(to_ret, std::regex("<"), "&lt;");
    to_ret = std::regex_replace(to_ret, std::regex(">"), "&gt;");
    to_ret = std::regex_replace(to_ret, std::regex("'"), "&apos;");
    to_ret = std::regex_replace(to_ret, std::regex("`"), "&apos;");
    to_ret = std::regex_replace(to_ret, std::regex(R"(")"), "&quot;");
    return to_ret;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text";
    RenderAttrs(out);
    out << R"( x=")" << position_.x << R"(" y=")" << position_.y <<
    R"(" dx=")"sv << offset_.x << R"(" dy=")" << offset_.y <<
    R"(" font-size=")" << font_size_; 
    if (!font_family_.empty())
    {
        out << R"(" font-family=")" << font_family_;
    }
    if (!font_weight_.empty())
    {
        out << R"(" font-weight=")"sv << font_weight_;
    }
    out << R"(">)" << DataCorrect(data_) << "</text>"sv;
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    RenderContext render(out, 2, 2);
    out << R"(<?xml version="1.0" encoding="UTF-8" ?>)" << "\n" <<
    R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)" << "\n";
    for (const auto& obj_ : objects_)
    {
        obj_->Render(render);
    }
    out << "</svg>"sv;
}

}  // namespace svg