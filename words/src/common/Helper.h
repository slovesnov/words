/*
 * Helper.h
 *
 *  Created on: 21.12.2016
 *      Author: alexey slovesnov
 */

#pragma once

#define ASLOV_SHORT_LOGGING
#include "aslov.h"
#include "enums.h"
#include <cmath>
#include <set>
#include <unordered_map>

// Note cann't move this constants to consts.h
const int LANGUAGES = 2;
const std::string INVALID_DIFFERENCE = "$";
const int STACK_TRACE_DEPTH = 5;

using Dictionary = VString;
template <typename T>
using PairVectorIterators = std::pair<typename std::vector<T>::const_iterator,
                                      typename std::vector<T>::const_iterator>;
using uchar = unsigned char;
using TwoStringVectors = std::array<VString, 2>;

// Pairs
using StringInt = std::pair<std::string, int>;
using IntDouble = std::pair<int, double>;
using IntInt = std::pair<int, int>;
using StringString = std::pair<std::string, std::string>;

// Vectors
using VectorInt = std::vector<int>;
using VectorDouble = std::vector<double>;
using VectorVectorInt = std::vector<VectorInt>;
using VectorBool = std::vector<bool>;
using VectorStringInt = std::vector<StringInt>;
using VectorStringIntCI = VectorStringInt::const_iterator;
using VectorIntInt = std::vector<IntInt>;
using VectorIntIntCI = VectorIntInt::const_iterator;
using VectorStringString = std::vector<StringString>;
using VVString = std::vector<VString>;

// Maps
using MapIntInt = std::map<int, int>;
using MapIntMapIntInt = std::map<int, MapIntInt>;
using MapStringStringVector = std::map<std::string, VString>;
using MapStringStringVectorI = MapStringStringVector::iterator;
using MapStringInt = std::map<std::string, int>;
using MapStringIntI = MapStringInt::iterator;
using MapStringTwoStringVectors = std::map<std::string, TwoStringVectors>;
using VectorMapStringTwoStringVectors = std::vector<MapStringTwoStringVectors>;
using MapMenu = std::map<ENUM_MENU, GtkWidget *>;
using MapAnagram = std::unordered_map<std::string, VString>;

// Sets
using SetString = std::set<std::string>;
// using SetInt = std::set<int>;

class ThreadResult {
public:
  int total;
  VectorVectorInt a;
  std::string s;
  void clear(int n, int n1 = -1);
  void add(VectorVectorInt &e) const;
};
using VectorThreadResult = std::vector<ThreadResult>;

struct StartStopButtonState {
  bool imageStart, enable;
};

template <typename KeyT, typename ValueT> class LookupTable {
public:
  // Helper structure to enable flexible initialization patterns
  struct InitRow {
    std::vector<KeyT> keys;
    ValueT value;

    // Constructor 1: Handles single key mappings like {Key, Value}
    InitRow(KeyT key, ValueT val)
        : keys{std::move(key)}, value(std::move(val)) {}

    // Constructor 2: Handles multiple keys mapped to one value like {{Key1,
    // Key2}, Value}
    InitRow(std::initializer_list<KeyT> key_list, ValueT val)
        : keys(key_list), value(std::move(val)) {}
  };

private:
  std::unordered_map<KeyT, ValueT> m;

public:
  // Main constructor that flattens rows of single or multiple keys into the map
  LookupTable(std::initializer_list<InitRow> init_list) {
    for (const auto &row : init_list) {
      for (const auto &key : row.keys) {
        m[key] = row.value;
      }
    }
  }

  // Checks if the key exists in the table
  bool has(const KeyT &key) const { return m.find(key) != m.end(); }

  // Retrieves the value by key, returning std::nullopt if not found
  std::optional<ValueT> get(const KeyT &key) const {
    if (auto it = m.find(key); it != m.end()) {
      return it->second;
    }
    return std::nullopt;
  }

  // Returns the total number of unique keys mapped
  int size() const { return m.size(); }

  // Returns the internal iteration index of a key, or -1 if not found
  // Warning: order is unstable due to std::unordered_map hashing
  int indexOf(const KeyT &key) const {
    int index = 0;
    for (const auto &[k, v] : m) {
      if (k == key)
        return index;
      index++;
    }
    return -1;
  }

  // Retrieves both the value and its internal iteration index by key
  // Warning: order is unstable due to std::unordered_map hashing
  std::optional<std::pair<ValueT, int>> getWithIndex(const KeyT &key) const {
    int index = 0;
    for (const auto &a : m) {
      if (a.first == key) {
        return std::pair<ValueT, int>{a.second, index};
      }
      index++;
    }
    return std::nullopt;
  }
};

// max depends on language
struct ComboData {
  int min;
  std::array<int, LANGUAGES> max;
  int active;
  ENUM_STRING s1, s2, send;
};

struct SpinData {
  ENUM_STRING e;
  double initial, minimum, maximum, step_increment, page_increment, climb_rate;
};

class SearchResult {
public:
  std::string s;
  int length;
  int words;
  double percent[VOWELS_CONSONANTS_SIZE]; // percent of vowels,consonants
  int differentCharacters;

  static std::string out;
  static VectorDouble vd;
  static MapIntMapIntInt mapIMII;
  SearchResult(std::string _s, int _length, int _words);
};

using VectorSearchResult = std::vector<SearchResult>;
using BOOL_SEARCH_RESULT_SEARCH_RESULT_FUNCTION =
    bool (*)(const SearchResult &, const SearchResult &);

extern BOOL_SEARCH_RESULT_SEARCH_RESULT_FUNCTION SORT_FUNCTION[];

bool sortIntDouble(const IntDouble &r1, const IntDouble &r2);
bool sortStringInt(const StringInt &r1, const StringInt &r2);
std::string fastLocaleToUtf8(const std::string &src);
std::string fastUtf8ToLocale(const std::string &src);
std::string capitalizeFirstUtf8(const std::string &src);
ENUM_STRING getLetterDeclension(int number);
VectorVectorInt &sum(VectorThreadResult &v);
std::string lowercase_utf8_regex(const std::string &pattern);
bool breakMenu(const std::string &s);
#ifndef NDEBUG
void printShortStackTrace(int max_lines = STACK_TRACE_DEPTH);
#endif

std::string sub(std::string const &minuend, std::string const &subtrahend);
std::string getOrderedString(std::string const &s);

class LetterGroupSplitItem {
public:
  std::string sub;
  VString anagrams;
  void add(const std::string &s) { anagrams.push_back(s); }
  std::string string() const {
    std::string o = joinV(anagrams);
    if (anagrams.size() != 1) {
      o = '{' + o + '}';
    }
    return o;
  }
};

using MapEq = std::map<std::string, LetterGroupSplitItem>;

class LetterGroupSplitMap : public std::vector<MapEq> {
public:
  std::string get(std::string const &s) const {
    auto &a = (*this)[s.length()].find(s)->second;
    return a.string();
  }
  VectorStringString
  allPairs(std::string const &s,
           std::string const &low = INVALID_DIFFERENCE) const {
    VectorStringString v;
    for (size_t i = 1; i < s.size(); i++) {
      for (auto &e : (*this)[i]) {
        if (low == INVALID_DIFFERENCE || low <= e.first) {
          auto a = sub(s, e.first);
          if (a != INVALID_DIFFERENCE && e.first <= a) {
            if ((*this)[a.length()].contains(a)) {
              v.push_back({get(e.first), get(a)});
            }
          }
        }
      }
    }
    return v;
  }
};

VString getLocaleWords(const std::string &strUtf8);
template <typename T>
int indexOfOrdered(const T &value, const std::vector<T> &vec) {
  auto it = std::lower_bound(vec.begin(), vec.end(), value);
  if (it != vec.end() && *it == value) {
    return static_cast<int>(std::distance(vec.begin(), it));
  }
  return -1;
}
std::string squareBrackets(int v);
void cairo_rounded_rectangle(cairo_t *cr, double x, double y, double width,
                             double height, double radius);
void hsl_to_rgb(double h, double s, double l, double *r, double *g, double *b);
void draw_with_hsl_color(cairo_t *cr, double ratio);

struct Color {
  double r, g, b;

  Color(int r_255, int g_255, int b_255) {
    r = r_255 / 255.0;
    g = g_255 / 255.0;
    b = b_255 / 255.0;
  }

  Color(double r_val, double g_val, double b_val)
      : r(r_val), g(g_val), b(b_val) {}

  void set(cairo_t *cr) const { cairo_set_source_rgb(cr, r, g, b); }
};

struct Line {
  double x1, y1, x2, y2;
  Line() {}
  Line(double _x1, double _y1, double _x2, double _y2) {
    set(_x1, _y1, _x2, _y2);
  }

  void set(double _x1, double _y1, double _x2, double _y2) {
    x1 = _x1;
    y1 = _y1;
    x2 = _x2;
    y2 = _y2;
  }

  double x(double y) const { return ((x2 - x1) / (y2 - y1)) * (y - y1) + x1; }
  double y(double x) const { return ((y2 - y1) / (x2 - x1)) * (x - x1) + y1; }

  double operator()(double x) const { return y(x); }
};

struct Graph {
  Line linex, liney;
  cairo_t *cr;
  PangoLayout *layout;
  int margin;
  Graph() {}

  void set(cairo_t *_cr, PangoLayout *_layout, int left, int top, int width,
           int height, double minx, double maxx, double miny, double maxy,
           int _margin) {
    cr = _cr;
    layout = _layout;
    margin = _margin;
    linex.set(minx, left, maxx, left + width);
    liney.set(maxy, top, miny, top + height);

    auto l = snap_to_pixel(left);
    auto t = snap_to_pixel(top);
    auto r = snap_to_pixel(l + width);
    auto b = snap_to_pixel(t + height);

    cairo_set_line_width(cr, 1.0);
    cairo_move_to(cr, l, t);
    cairo_line_to(cr, l, b);
    cairo_line_to(cr, r, b);
    cairo_stroke(cr);
  }

  void set(cairo_t *_cr, PangoLayout *_layout, int left, int top, int width,
           int height, Line const &lx, Line const &ly, int _margin) {
    set(_cr, _layout, left, top, width, height, lx(0), lx(width), ly(0),
        ly(height), _margin);
  }

  void draw_text(double x, double y, std::string const &s, Align h_align,
                 Align v_align) {
    // 1. Assign the text string to the layout container
    pango_layout_set_text(layout, s.c_str(), -1);

    // 2. Sync layout dimensions with Cairo context matrix scale changes
    pango_cairo_update_layout(cr, layout);

    // 3. Retrieve bounding dimensions in raw integer pixels
    PangoRectangle ink_rect;
    PangoRectangle logical_rect;
    pango_layout_get_pixel_extents(layout, &ink_rect, &logical_rect);

    // --- Horizontal Alignment ---
    double target_x = x;
    switch (h_align) {
    case Align::Start: // Left edge
      target_x = x - ink_rect.x;
      break;

    case Align::Center: // Center along X-axis
      target_x = x - ink_rect.x - (ink_rect.width / 2.0);
      break;

    case Align::End: // Right edge
      target_x = x - ink_rect.x - ink_rect.width;
      break;
    }

    // --- Vertical Alignment ---
    double target_y = y;
    switch (v_align) {
    case Align::Start: // Top edge
      target_y = y - ink_rect.y;
      break;

    case Align::Center: // Center along Y-axis
      target_y = y - ink_rect.y - (ink_rect.height / 2.0);
      break;

    case Align::End: // Bottom edge
      target_y = y - ink_rect.y - ink_rect.height;
      break;
    }

    cairo_move_to(cr, target_x, target_y);
    pango_cairo_show_layout(cr, layout);
  }

  void drawxlabel(double _x) { // minx...maxx
    double x = snap_to_pixel(linex(_x));
    double y = snap_to_pixel(bottom());
    double b = snap_to_pixel(y + margin);
    cairo_move_to(cr, x, y);
    cairo_line_to(cr, x, b);
    cairo_stroke(cr);

    std::string s = std::format("{:g}", _x);
    draw_text(x, y + margin / 2, s, Align::Start, Align::Center);
  }

  void drawylabel(double _y) { // miny...maxy
    drawylabel(_y, std::format("{:g}", _y));
  }

  static const int MARGIN = 3;
  static constexpr gdouble PANGO_TO_PIXELS = 1.0 / PANGO_SCALE;

  void drawylabel(double _y, std::string const &s, Align xalign = Align::End,
                  Align yalign = Align::Center) { // miny...maxy
    double x = snap_to_pixel(left());
    double y = snap_to_pixel(liney(_y));
    double x1 = snap_to_pixel(left() - margin);
    cairo_move_to(cr, x, y);
    cairo_line_to(cr, x1, y);
    cairo_stroke(cr);

    cairo_save(cr);
    int add = 0;
    if (xalign == Align::End) {
      add = MARGIN;
    } else if (xalign == Align::Start) {
      add = -MARGIN;
    }
    cairo_translate(cr, x - margin / 2, y + add);
    cairo_rotate(cr, -G_PI / 2);
    draw_text(0, 0, s, xalign, yalign);
    cairo_restore(cr);
  }

  void axisy(std::string const &s) {
    cairo_save(cr);
    cairo_translate(cr, left() + MARGIN, centery());
    cairo_rotate(cr, -G_PI / 2);
    draw_text(0, 0, s, Align::Center, Align::Start);
    cairo_restore(cr);
  }

  void drawPoint(gdouble x, gdouble y, gdouble radius, const Color &pointColor,
                 std::string const &s, const Color &stringColor) {
    pointColor.set(cr);
    drawPoint(x, y, radius);

    int text_width, text_height;
    pango_layout_set_text(layout, s.c_str(), -1);
    pango_layout_get_size(layout, &text_width, &text_height);
    auto t_width = text_width * PANGO_TO_PIXELS;
    auto t_height = text_height * PANGO_TO_PIXELS;
    auto tx = linex(x) - t_width / 2;
    auto ty = liney(y) - t_height / 2;
    stringColor.set(cr);
    cairo_move_to(cr, tx, ty);
    pango_cairo_show_layout(cr, layout);
  }

  void drawPoint(gdouble x, gdouble y, gdouble radius) {
    cairo_arc(cr, linex(x), liney(y), radius, 0, 2 * G_PI);
    cairo_fill(cr);
  }

  void drawThinPoint(gdouble _x, gdouble _y, bool clip = true) {
    double x = snap_to_pixel(linex(_x));
    double y = snap_to_pixel(liney(_y));
    if ((clip && x >= left() && x < width() && y >= top() && y < bottom()) ||
        !clip) {
      cairo_rectangle(cr, x, y, 1.0, 1.0);
      cairo_fill(cr);
    }
  }

  // fil=.5 then text fill about 50% oh height,add=1 because one label is out of
  // axis
  int nLabelsY(std::string const &s, double fill = .5, int add = 1) const {
    pango_layout_set_text(layout, s.c_str(), -1);
    int w;
    pango_layout_get_size(layout, &w, NULL);
    return std::round(height() / (w * PANGO_TO_PIXELS) * fill + add);
  }

  int nLabelsStepX(int n, std::string const &s, double fill = .25) const {
    pango_layout_set_text(layout, s.c_str(), -1);
    int w;
    pango_layout_get_size(layout, &w, NULL);
    return std::round(n * w * PANGO_TO_PIXELS / width() / fill);
  }

  static double snap_to_pixel(double val) { return std::floor(val) + 0.5; }

  int left() const { return linex.y1; }
  int right() const { return linex.y2; }
  int width() const { return right() - left(); }
  int top() const { return liney.y1; }
  int bottom() const { return liney.y2; }
  int height() const { return bottom() - top(); }
  double centerx() const { return (left() + right()) / 2.; }
  double centery() const { return (top() + bottom()) / 2.; }
  double minx() const { return linex.x1; }
  double maxx() const { return linex.x2; }
  double miny() const { return liney.x2; } // invert order
  double maxy() const { return liney.x1; } // invert order
};

PairDoubleDouble leastSquares(const VectorDouble &x, const VectorDouble &y);

struct ChainLayerResult {
  VectorInt v, opposite;
};
using VectorChainLayerResult = std::vector<ChainLayerResult>;

template <typename T> void addMove(std::vector<T> &v, std::vector<T> &add) {
  v.reserve(v.size() + add.size());
  v.insert(v.end(), std::make_move_iterator(add.begin()),
           std::make_move_iterator(add.end()));
}

bool spanIncluding(std::string_view p, std::string_view pattern);
int differentChars(std::string_view s);
std::string string(ENUM_STRING e);
std::string intToString(int v, char c = ',');

class TableFormat {
private:
  std::vector<ENUM_STRING> m_en;
  std::vector<int> m_width;
  int m_rows;

  template <typename T> std::string formatCell(const T &arg, size_t i) const {
    using DecayedT = std::decay_t<T>;

    // 1. If it's an ENUM_STRING, convert it to std::string using wordsBase
    if constexpr (std::is_same_v<DecayedT, ENUM_STRING>) {
      std::string word_str = ::string(arg);
      return formatCell(word_str,
                        i); // Recursively format the converted string
    }
    // 2. Standard formatting logic for all other types
    else {
      // If it's the last (+1) argument, print WITHOUT alignment
      if (i == m_width.size()) {
        return std::format("{}", arg);
      }

      if constexpr (std::is_floating_point_v<DecayedT>) {
        // For double/float: right-align, width m_width[i], 2 decimal places
        return std::format("{:>{}.2f}", arg, m_width[i]);
      } else if constexpr (std::is_convertible_v<T, std::string_view>) {
        return std::format("{:>{}}", arg, m_width[i]);
      } else {
        return std::format("{:>{}}", intToString(arg), m_width[i]);
      }
    }
  }

  // Helper function performing pack expansion using compile-time indices
  template <typename Tuple, size_t... EnIndices, size_t... WidthIndices>
  std::string createWithTitleImpl(Tuple &&t, std::index_sequence<EnIndices...>,
                                  std::index_sequence<WidthIndices...>) {
    constexpr size_t N = sizeof...(EnIndices);
    // 1. Call init() by extracting the first N elements and the next N
    // elements from the tuple We add N to WidthIndices to shift into the
    // second half of the tuple
    init({static_cast<ENUM_STRING>(std::get<EnIndices>(t))...},
         {static_cast<int>(std::get<WidthIndices + N>(t))...});

    // 2. Call string() with the very last element (index 2N)
    return string(std::get<2 * N>(t));
  }

public:
  TableFormat() {}
  TableFormat(std::initializer_list<ENUM_STRING> en,
              std::initializer_list<int> width) {
    init(en, width);
  }

  int rows() const { return m_rows; }
  void init(std::initializer_list<ENUM_STRING> en,
            std::initializer_list<int> width) {
    m_rows = 0;
    m_en = en;
    m_width = width;
    assert(en.size() == width.size());
    int i = -1;
    for (auto a : en) {
      i++;
      m_width[i] = std::max(m_width[i], utf8Length(::string(a)));
    }
  }

  template <typename... Args> std::string string(Args... args) {
    const size_t args_count = sizeof...(args);
    size_t i = 0;
    m_rows++;
    if constexpr (args_count == 1) {
      using FirstArgT =
          std::decay_t<std::tuple_element_t<0, std::tuple<Args...>>>;
      if constexpr (std::is_same_v<FirstArgT, ENUM_STRING>) {
        ENUM_STRING en = [args...](auto first, auto...) {
          return first;
        }(args...);

        std::string s;
        for (; i <= std::size(m_en); ++i) {
          s += (i ? " " : "");
          s += formatCell(i == std::size(m_en) ? en : m_en[i], i);
        }
#ifdef NOGTK
        return s;
#else
        return "<b>" + s + "</b>";
#endif
      }
    }
    assert(args_count == m_width.size() || args_count == m_width.size() + 1);

    std::string s = "\n";

    (
        [&](const auto &arg) {
          s += (i ? " " : "");
          s += formatCell(arg, i);
          i++;
        }(args),
        ...);
    return s;
  }

  // New function: accepts an odd number of arguments (2n + 1)
  template <typename... Args> std::string createWithTitle(Args... args) {
    // Compile-time check ensuring the number of arguments is odd and at least
    // 1
    static_assert(sizeof...(Args) % 2 != 0,
                  "Argument count must be odd (2n + 1)");

    constexpr size_t N = (sizeof...(Args) - 1) / 2;

    // Pack all arguments into a tuple for convenient slicing without copying
    auto args_tuple = std::forward_as_tuple(args...);

    // Call the private helper function, passing index sequences for packet
    // splitting
    return createWithTitleImpl(
        args_tuple, std::make_index_sequence<N>{}, // Indices from 0 to N-1
        std::make_index_sequence<N>{});            // Indices from N to 2N-1
  }
};
