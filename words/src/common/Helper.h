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
#include <optional>
#include <set>
#include <unordered_map>

// Note cann't move this constants to consts.h
const int LANGUAGES = 2;
const std::string INVALID_DIFFERENCE = "$";
const int STACK_TRACE_DEPTH = 5;

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
using MenuMap = std::map<ENUM_MENU, GtkWidget *>;
using AnagramMap = std::unordered_map<std::string, VString>;

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
  SearchResult(std::string _s, int _length, int _words);
};

using SearchResultVector = std::vector<SearchResult>;
using BOOL_SEARCH_RESULT_SEARCH_RESULT_FUNCTION =
    bool (*)(const SearchResult &, const SearchResult &);

extern BOOL_SEARCH_RESULT_SEARCH_RESULT_FUNCTION SORT_FUNCTION[];

bool sortIntDouble(const IntDouble &r1, const IntDouble &r2);
bool sortStringInt(const StringInt &r1, const StringInt &r2);
std::string fastLocaleToUtf8(const std::string &src);
std::string fastUtf8ToLocale(const std::string &src);
std::string capitalizeFirstUtf8(const std::string &src);
ENUM_STRING getLetterDeclension(int number);
std::vector<IntVector> &sum(ThreadResultVector &v);
std::string lowercase_utf8_regex(const std::string &pattern);
bool breakMenu(const std::string &s);
#ifndef NDEBUG
void print_short_stack_trace(int max_lines = STACK_TRACE_DEPTH);
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

using EqMap = std::map<std::string, LetterGroupSplitItem>;

class LetterGroupSplitMap : public std::vector<EqMap> {
public:
  std::string get(std::string const &s) const {
    auto &a = (*this)[s.length()].find(s)->second;
    return a.string();
  }
  /*    StringStringVector allPairs(std::string const &s, int index = -1) const
    { StringStringVector v; for (size_t i = 1; i < s.size(); i++) { int j = -1;
        for (auto &e : (*this)[i]) {
          j++;
          if (j <= index) {
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
   */
  StringStringVector
  allPairs(std::string const &s,
           std::string const &low = INVALID_DIFFERENCE) const {
    StringStringVector v;
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
