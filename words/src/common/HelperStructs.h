/*
 * HelperStructs.h
 *
 *  Created on: 21.12.2016
 *      Author: alexey slovesnov
 */

#pragma once

#include "aslov.h"
#include "enums.h"
#include <optional>
#include <set>
#include <unordered_map>

#ifdef NOGTK
// #define USE_STANDARD_REGEX
#endif

// if use gtk regex
#if defined(NOGTK) && !defined(USE_STANDARD_REGEX)
#include <glib.h>
#endif

using Dictionary = VString;
using DictionaryCI = Dictionary::const_iterator;
using PairDCIDCI = std::pair<DictionaryCI, DictionaryCI>;
using uchar = unsigned char;
using TwoStringVectors = std::array<VString, 2>;

// Pairs
using StringInt = std::pair<std::string, int>;
using IntDouble = std::pair<int, double>;
using IntInt = std::pair<int, int>;
using StringString = std::pair<std::string, std::string>;

// Vectors
using IntVector = std::vector<int>;
using StringIntVector = std::vector<StringInt>;
using StringIntVectorCI = StringIntVector::const_iterator;
using StringVectorPtr = VString *;
using IntIntVector = std::vector<IntInt>;
using IntIntVectorCI = IntIntVector::const_iterator;
using StringStringVector = std::vector<StringString>;
using VVString = std::vector<VString>;

// Maps
using MapStringStringVector = std::map<std::string, VString>;
using MapStringStringVectorI = MapStringStringVector::iterator;
using MapStringInt = std::map<std::string, int>;
using MapStringIntI = MapStringInt::iterator;
using MapStringTwoStringVectors = std::map<std::string, TwoStringVectors>;
using VMapStringTwoStringVectors = std::vector<MapStringTwoStringVectors>;

// Sets
using StringSet = std::set<std::string>;

class ChainNode {
public:
  std::string s;
  IntVector next;
  explicit ChainNode(std::string _s) : s(std::move(_s)) {}
};

using ChainNodeVector = std::vector<ChainNode>;
using ChainNodeVectorI = ChainNodeVector::iterator;
using ChainNodeVectorCI = ChainNodeVector::const_iterator;

class ThreadResult {
public:
  int total;
  std::vector<IntVector> a;
  std::string s;
  void clear(int n, int n1 = -1);
  void add(std::vector<IntVector> &e) const;
};
using ThreadResultVector = std::vector<ThreadResult>;

#ifndef USE_STANDARD_REGEX
struct GRegexDeleter {
  void operator()(GRegex *r) const {
    if (r)
      g_regex_unref(r);
  }
};
using SafeGRegex = std::unique_ptr<GRegex, GRegexDeleter>;

struct PangoFontDescDeleter {
    void operator()(PangoFontDescription* desc) const {
        if (desc) {
            pango_font_description_free(desc);
        }
    }
};

using SafePangoFontDesc = std::unique_ptr<PangoFontDescription, PangoFontDescDeleter>;

#endif

struct StartStopButtonState {
  bool imageStart, enable;
};

template <typename KeyT, typename ValueT> class LookupTable {
private:
  std::unordered_map<KeyT, ValueT> m;

public:
  LookupTable(std::initializer_list<std::pair<const KeyT, ValueT>> init_list)
      : m(init_list) {}

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

bool sortIntDouble(const IntDouble &r1, const IntDouble &r2);
bool sortStringInt(const StringInt &r1, const StringInt &r2);
std::string fastLocaleToUtf8(const std::string &src);
std::string fastUtf8ToLocale(const std::string &src);
std::string capitalizeFirstUtf8(const std::string &src);
ENUM_STRING getLetterDeclension(int number);
