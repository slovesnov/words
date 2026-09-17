/*
 * HelperStructs.h
 *
 *  Created on: 21.12.2016
 *      Author: alexey slovesnov
 */

#pragma once

#include "aslov.h"
#include <set>

// Iterators
using StringI = std::string::iterator;
using StringCI = std::string::const_iterator;

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

// Sets
using StringSet = std::set<std::string>;

using TwoStringVectors = std::array<VString, 2>;

using MapStringTwoStringVectors = std::map<std::string, TwoStringVectors>;
using MapStringTwoStringVectorsI = MapStringTwoStringVectors::iterator;

class ChainNode {
public:
  std::string s;
  IntVector next;
  explicit ChainNode(std::string _s) : s(std::move(_s)) {}
};

using ChainNodeVector = std::vector<ChainNode>;
using ChainNodeVectorI = ChainNodeVector::iterator;
using ChainNodeVectorCI = ChainNodeVector::const_iterator;

bool sortIntDouble(const IntDouble &r1, const IntDouble &r2);
bool sortIntInt(const IntInt &r1, const IntInt &r2);
bool sortStringInt(const StringInt &r1, const StringInt &r2);
