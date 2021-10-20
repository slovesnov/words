/*
 * SearchResult.cpp
 *
 *  Created on: 14.12.2016
 *      Author: alexey slovesnov
 */


#include "SearchResult.h"
#include "WordsBase.h"
#include <string.h>

SearchResult::SearchResult(std::string _s, int _length, int _words) {
	s=_s;
	length=_length;
	words=_words;
	std::set<char> set;
	unsigned i,l;
	const char*p[VOWELS_CONSONANTS_SIZE];
	const char*q;
	for(i=0;i<VOWELS_CONSONANTS_SIZE;i++){
		percent[i]=0;
		p[i]=wordsBase->getVowelConsonant(i==VOWELS_CONSONANTS_CONSONANTS).c_str();
	}


	for(l=0,q=s.c_str() ; *q!=0 && *q!=' ' ;q++,l++){//for many words goes until first space
		set.insert(*q);
		for(i=0;i<VOWELS_CONSONANTS_SIZE;i++){
			if(strchr(p[i],*q)!=NULL){
				percent[i]++;
				break;
			}
		}
	}

	for(i=0;i<VOWELS_CONSONANTS_SIZE;i++){
		percent[i]*=100./l;
	}

	differentCharacters=set.size();
}


//Note all sort functions sort by alphabet if comparing parameters of search results are equal
bool sortalphabetAscending (const SearchResult& r1,const SearchResult& r2) {
	return r1.s<r2.s;
}

bool sortalphabetDescending (const SearchResult& r1,const SearchResult& r2) {
	return r1.s>r2.s;
}


#define M(n,name) bool sort##name##Ascending(const SearchResult& r1,const SearchResult& r2){return r1.n==r2.n ? r1.s<r2.s : r1.n<r2.n;}\
		bool sort##name##Descending(const SearchResult& r1,const SearchResult& r2){return r1.n==r2.n ? r1.s<r2.s : r1.n>r2.n;}

#define MM(a) M(a,a)

MM(length)
MM(words)
MM(differentCharacters);
M(percent[VOWELS_CONSONANTS_VOWELS],vowel)
M(percent[VOWELS_CONSONANTS_CONSONANTS],consonant)

#undef MM
#undef M
#define M(name) &sort##name##Ascending,&sort##name##Descending,

#define M(name) &sort##name##Ascending,&sort##name##Descending,
BOOL_SEARCH_RESULT_SEARCH_RESULT_FUNCTION SORT_FUNCTION[]={
		M(alphabet)
		M(length)
		M(words)
		M(vowel)
		M(consonant)
		M(differentCharacters)
};
#undef M
const int NUMBER_OF_SORTS=SIZEI(SORT_FUNCTION)/2;
