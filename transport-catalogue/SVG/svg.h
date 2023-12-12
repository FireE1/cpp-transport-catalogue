#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace svg {

struct Rgb {

  Rgb() = default;

  Rgb(double r, double g, double b) : red(r), green(g), blue(b) {}

  double red = 0.0;
  double green = 0.0;
  double blue = 0.0;
};

inline void PrintColor(std::ostream &out, Rgb &rgb);

struct Rgba {

  Rgba() = default;

  Rgba(double r, double g, double b, double a)
      : red(r), green(g), blue(b), alpha(a) {}

  double red = 0.0;
  double green = 0.0;
  double blue = 0.0;
  double alpha = 1.0;
};

inline void PrintColor(std::ostream &out, Rgba &rgba);

using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;

inline const Color NoneColor{"none"};

inline void PrintColor(std::ostream &out, std::monostate);
inline void PrintColor(std::ostream &out, std::string &color);
std::ostream &operator<<(std::ostream &out, const Color &color);

struct Point {
  Point() = default;
  Point(double x, double y) : x(x), y(y) {}
  double x = 0;
  double y = 0;
};

enum class StrokeLineCap {
  BUTT,
  ROUND,
  SQUARE,
};

inline std::ostream &operator<<(std::ostream &out,
                                StrokeLineCap stroke_line_cap) {
  using namespace std::literals;
  if (stroke_line_cap == StrokeLineCap::BUTT) {
    out << "butt"sv;
  } else if (stroke_line_cap == StrokeLineCap::ROUND) {
    out << "round"sv;
  } else if (stroke_line_cap == StrokeLineCap::SQUARE) {
    out << "square"sv;
  }
  return out;
}

enum class StrokeLineJoin {
  ARCS,
  BEVEL,
  MITER,
  MITER_CLIP,
  ROUND,
};

inline std::ostream &operator<<(std::ostream &out,
                                StrokeLineJoin stroke_line_join) {
  using namespace std::literals;
  if (stroke_line_join == StrokeLineJoin::ARCS) {
    out << "arcs"sv;
  } else if (stroke_line_join == StrokeLineJoin::BEVEL) {
    out << "bevel"sv;
  } else if (stroke_line_join == StrokeLineJoin::MITER) {
    out << "miter"sv;
  } else if (stroke_line_join == StrokeLineJoin::MITER_CLIP) {
    out << "miter-clip"sv;
  } else if (stroke_line_join == StrokeLineJoin::ROUND) {
    out << "round"sv;
  }
  return out;
}

template <typename Owner> class PathProps {
public:
  Owner &SetFillColor(Color color) {
    fill_color_ = std::move(color);
    return AsOwner();
  }

  Owner &SetStrokeColor(Color color) {
    stroke_color_ = std::move(color);
    return AsOwner();
  }

  Owner &SetStrokeWidth(double width) {
    stroke_width_ = width;
    return AsOwner();
  }

  Owner &SetStrokeLineCap(StrokeLineCap line_cap) {
    stroke_line_cap_ = line_cap;
    return AsOwner();
  }

  Owner &SetStrokeLineJoin(StrokeLineJoin line_join) {
    stroke_line_join_ = line_join;
    return AsOwner();
  }

protected:
  ~PathProps() = default;

  void RenderAttrs(std::ostream &out) const {
    using namespace std::literals;

    if (fill_color_ != std::nullopt) {
      out << " fill=\""sv << fill_color_.value() << "\""sv;
    }
    if (stroke_color_ != std::nullopt) {
      out << " stroke=\""sv << stroke_color_.value() << "\""sv;
    }
    if (stroke_width_ != std::nullopt) {
      out << " stroke-width=\""sv << stroke_width_.value() << "\""sv;
    }
    if (stroke_line_cap_ != std::nullopt) {
      out << " stroke-linecap=\""sv << stroke_line_cap_.value() << "\""sv;
    }
    if (stroke_line_join_ != std::nullopt) {
      out << " stroke-linejoin=\""sv << stroke_line_join_.value() << "\""sv;
    }
  }

private:
  Owner &AsOwner() { return static_cast<Owner &>(*this); }

  std::optional<Color> fill_color_;
  std::optional<Color> stroke_color_;
  std::optional<double> stroke_width_;
  std::optional<StrokeLineCap> stroke_line_cap_;
  std::optional<StrokeLineJoin> stroke_line_join_;
};

struct RenderContext {
  RenderContext(std::ostream &out) : out(out) {}

  RenderContext(std::ostream &out, int indent_step, int indent = 0)
      : out(out), indent_step(indent_step), indent(indent) {}

  RenderContext Indented() const {
    return {out, indent_step, indent + indent_step};
  }

  void RenderIndent() const {
    for (int i = 0; i < indent; ++i) {
      out.put(' ');
    }
  }

  std::ostream &out;
  int indent_step = 0;
  int indent = 0;
};

class Object {
public:
  void Render(const RenderContext &context) const;

  virtual ~Object() = default;

private:
  virtual void RenderObject(const RenderContext &context) const = 0;
};

class Circle final : public Object, public PathProps<Circle> {
public:
  Circle &SetCenter(Point center);
  Circle &SetRadius(double radius);

private:
  void RenderObject(const RenderContext &context) const override;

  Point center_;
  double radius_ = 1.0;
};

class Polyline final : public Object, public PathProps<Polyline> {
public:
  // Добавляет очередную вершину к ломаной линии
  Polyline &AddPoint(Point point);

private:
  void RenderObject(const RenderContext &context) const override;

  std::vector<Point> points_;
};

class Text final : public Object, public PathProps<Text> {
public:
  // Задаёт координаты опорной точки (атрибуты x и y)
  Text &SetPosition(Point pos);

  // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
  Text &SetOffset(Point offset);

  // Задаёт размеры шрифта (атрибут font-size)
  Text &SetFontSize(uint32_t size);

  // Задаёт название шрифта (атрибут font-family)
  Text &SetFontFamily(std::string font_family);

  // Задаёт толщину шрифта (атрибут font-weight)
  Text &SetFontWeight(std::string font_weight);

  // Задаёт текстовое содержимое объекта (отображается внутри тега text)
  Text &SetData(std::string data);

private:
  void RenderObject(const RenderContext &context) const override;

  Point position_;
  Point offset_;
  uint32_t font_size_ = 1;
  std::string font_family_;
  std::string font_weight_;
  std::string data_;
};

class ObjectContainer {
public:
  template <typename Obj> void Add(Obj object) {
    objects_.emplace_back(std::make_unique<Obj>(std::move(object)));
  }

  virtual void AddPtr(std::unique_ptr<Object> &&obj) = 0;

  virtual ~ObjectContainer() = default;

protected:
  std::vector<std::unique_ptr<Object>> objects_;
};

class Drawable {
public:
  virtual void Draw(svg::ObjectContainer &container) const = 0;
  virtual ~Drawable() = default;
};

class Document : public ObjectContainer {
public:
  // Добавляет в svg-документ объект-наследник svg::Object
  void AddPtr(std::unique_ptr<Object> &&obj);

  // Выводит в ostream svg-представление документа
  void Render(std::ostream &out) const;
};

} // namespace svg
