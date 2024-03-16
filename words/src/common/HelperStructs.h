/*
 * HelperStructs.h
 *
 *  Created on: 21.12.2016
 *      Author: alexey slovesnov
 */

#ifndef HELPERSTRUCTS_H_
#define HELPERSTRUCTS_H_

#include "aslov.h"
#include <set>

typedef std::string::iterator StringI;
typedef std::string::const_iterator StringCI;

//pairs
typedef std::pair<std::string, int> StringInt;
typedef std::pair<int, double> IntDouble;
typedef std::pair<int, int> IntInt;

//vectors
typedef std::vector<int> IntVector;
typedef std::vector<StringInt> StringIntVector;
typedef StringIntVector::const_iterator StringIntVectorCI;
typedef VString *StringVectorPtr;
typedef std::vector<IntInt> IntIntVector;
typedef IntIntVector::const_iterator IntIntVectorCI;

//maps
typedef std::map<std::string, VString> MapStringStringVector;
typedef MapStringStringVector::iterator MapStringStringVectorI;
typedef std::map<std::string, int> MapStringInt;
typedef MapStringInt::iterator MapStringIntI;

//sets
typedef std::set<std::string> StringSet;
typedef StringSet::const_iterator StringSetCI;

struct TwoStringVectors {
	VString v1, v2;
	void add(int i, std::string s) {
		if (i == 0) {
			v1.push_back(s);
		} else {
			v2.push_back(s);
		}
	}
};
//after TwoStringVectors
typedef std::map<std::string, TwoStringVectors> MapStringTwoStringVectors;
typedef MapStringTwoStringVectors::iterator MapStringTwoStringVectorsI;
typedef MapStringTwoStringVectors::const_iterator MapStringTwoStringVectorsCI;

class ChainNode { //if use struct eclipse editor don't see ChainNode in WordsBase
public:
	std::string s;
	IntVector next; //use int index instead of ChainNode* because vector could reallocate memory, so ptrs became invalid
	ChainNode(std::string const &_s) {
		s = _s;
	}
};
//after ChainNode
typedef std::vector<ChainNode> ChainNodeVector;
typedef ChainNodeVector::iterator ChainNodeVectorI;
typedef ChainNodeVector::const_iterator ChainNodeVectorCI;

bool sortIntDouble(const IntDouble &r1, const IntDouble &r2);
bool sortIntInt(const IntInt &r1, const IntInt &r2);
bool sortStringInt(const StringInt &r1, const StringInt &r2);

#endif /* HELPERSTRUCTS_H_ */
