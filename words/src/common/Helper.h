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
private:
  std::unordered_map<KeyT, ValueT> m;

public:
  LookupTable(std::initializer_list<std::pair<KeyT, ValueT>> init_list)
      : m(init_list.begin(), init_list.end()) {}

  bool has(const KeyT &key) const { return m.find(key) != m.end(); }

  std::optional<ValueT> get(const KeyT &key) const {
    auto it = m.find(key);
    if (it != m.end()) {
      return it->second;
    }
    return std::nullopt;
  }

  int size() const { return m.size(); }

  int indexOf(const KeyT &key) const {
    int index = 0;
    for (const auto &[k, v] : m) {
      if (k == key) {
        return index;
      }
      index++;
    }
    return -1;
  }

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

class SearchResult {
public:
  std::string s;
  int length;
  int words;
  double percent[VOWELS_CONSONANTS_SIZE]; // percent of vowels,consonants
  int differentCharacters;

  static std::string out;
  static VectorDouble vd;
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
int indexOfOrdered(const std::string &s, const VString &a);
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

  void set(cairo_t *cr) { cairo_set_source_rgb(cr, r, g, b); }
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

  double get(double x) { return ((y2 - y1) / (x2 - x1)) * (x - x1) + y1; }
};

struct Line2 {
  Line linex, liney;
  Line2() {}
  Line2(double x11, double y11, double x12, double y12, double x21, double y21,
        double x22, double y22) {
    set(x11, y11, x12, y12, x21, y21, x22, y22);
  }

  void set(double x11, double y11, double x12, double y12, double x21,
           double y21, double x22, double y22) {
    linex.set(x11, y11, x12, y12);
    liney.set(x21, y21, x22, y22);
  }

  void draw(cairo_t *cr, gdouble x, gdouble y, gdouble radius) {
    cairo_arc(cr, linex.get(x), liney.get(y), radius, 0, 2 * G_PI);
    cairo_fill(cr);
  }
};
