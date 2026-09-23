/*
 * HelperStructs.h
 *
 *  Created on: 21.12.2016
 *      Author: alexey slovesnov
 */

#pragma once

#include "aslov.h"
#include <set>

#ifdef NOGTK
//#define USE_STANDARD_REGEX
#endif

//if use gtk regex
#if defined(NOGTK) && !defined(USE_STANDARD_REGEX)
#include <glib.h>
#endif

using Dictionary = VString;
using DictionaryCI=Dictionary::const_iterator;
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
#endif

bool sortIntDouble(const IntDouble &r1, const IntDouble &r2);
bool sortStringInt(const StringInt &r1, const StringInt &r2);
std::string fastLocaleToUtf8(const std::string &src);
std::string fastUtf8ToLocale(const std::string &src);
