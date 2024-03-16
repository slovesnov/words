/*
 * SearchResult.h
 *
 *  Created on: 14.12.2016
 *      Author: alexey slovesnov
 */

#ifndef SEARCHRESULT_H_
#define SEARCHRESULT_H_

#include "enums.h"
#include <string>
#include <vector>

class SearchResult {
public:
	std::string s;
	int length;
	int words;
	double percent[VOWELS_CONSONANTS_SIZE]; //percent of vowels,consonants
	int differentCharacters;

	SearchResult(std::string _s, int _length, int _words);
};

typedef std::vector<SearchResult> SearchResultVector;
typedef SearchResultVector::const_iterator SearchResultVectorCI;
typedef bool (*BOOL_SEARCH_RESULT_SEARCH_RESULT_FUNCTION)(const SearchResult&,
		const SearchResult&);
extern BOOL_SEARCH_RESULT_SEARCH_RESULT_FUNCTION SORT_FUNCTION[];
extern const int NUMBER_OF_SORTS;

#endif /* SEARCHRESULT_H_ */
