/*
 * ModificationItem.cpp
 *
 *  Created on: 17.11.2017
 *      Author: alexey slovesnov
 */

#include "ModificationItem.h"

#include <stdlib.h>
#include "WordsBase.h"

/* replace
 * a>bc replace all occurrences of "a" to "bc"
 * d>0  replace all occurrences of "b" to empty string
 *
 * insert/append index,string (like remove command index goes first)
 * +0,b append "b" to beginning of string (insert before symbol with index 0)
 * +1,da append "da" after 1st symbol of string (insert before symbol with index 1)
 * +L-1,bb append "bb" before last symbol of string "xyz" -> "xybbz"
 * +L,bd append "bd" to the end string (insert before symbol with index l = after symbol with index L-1)
 *
 * substring index,length like std::substr function in c++
 * index=0..L-1. If index+length>s.length() then return all string from index
 * -0,3 first three symbols "abcde"->"abc"
 * -1,3 three symbols from positions 1 "abcde"->"bcd"
 * -l-3 last three symbols
 * -2 all symbols from 2nd position "abcde" -> "cde"
 * -L-3,2 maximum two symbols from length-3 position "abcde" -> "cd" same with -2,2 for string length=5
 * -4,L-3 maximum length-3 symbols from fourth position "abcde" -> "e"
 */

#ifdef NDEBUG
	#define RET return false;
#else
	#define RET errorLine=__LINE__;return false;
#endif

void ModificationItem::skipSpaces(const char*& p) {
	while(*p==' '){
		p++;
	}
}

bool ModificationItem::readAlphabetString(std::string& s, const char*& p) {
	s="";
	while(wordsBase->isAlphabetChar(*p)){
		s+=*p++;
	}
	return !s.empty();
}

std::string ModificationItem::apply(const std::string& s)const {
	assert(type!=MODIFICATION_TYPE_SIZE);
	int i,j;

	if(type==MODIFICATION_TYPE_REPLACE){
		return WordsBase::replaceAll(s,from,to);
	}
	else if(type==MODIFICATION_TYPE_INSERT){
		i = countIn(0,s);
		//if i==s.length() then s.substr(i)="" that's need php & c++ ok
		if( (l[0] && i<0) || i>int(s.length()) ){
			return "";//too short string
		}
		return s.substr(0,i)+from+s.substr(i);
	}
	else{//MODIFICATION_TYPE_SUBSTRING
		i=countIn(0,s);
		if(i>=int(s.length()) || i<0){
			return "";//too short string
		}
		j=countIn(1,s);
		if(j<=0){
			return "";//too short string
		}
		//if j>s.length() it's ok for php & c++, takes recent part of string
		return s.substr(i, j );
	}

}

bool ModificationItem::parseLI(const char*&p, int i, char operation) {
	assert(operation=='+' || operation=='-');
	l[i] = *p=='l';//string already in lowercase
	if(l[i]){
		p++;
	}
	skipSpaces(p);

	//strtol ("- 4") not recognise as -4, so read sign manually

	bool invert=false;
	bool readIn=true;
	char*q;

	if(l[i]){
		if(*p==','){
			readIn=false;
		}
		else if(*p=='-'){
			p++;
			invert=true;
		}
		else if(*p=='+'){
			p++;
		}
		else{
			RET
		}
	}
	else{
		if(!isdigit(*p)){
			RET
		}
	}

	if(readIn){
		in[i]=strtol(p,&q,10);//some spaces after sing can be it's ok
		if(invert){
			in[i]=-in[i];
		}
		if(p==q){
			RET
		}
		p=q;
	}
	else{
		in[i]=0;
	}

	if(l[i]){
		if( (operation=='+' && in[i]>0) || (operation=='-' && in[i]>=0) ){
			RET
		}
	}
	else{
		if(in[i]<0){
			RET
		}
	}

	return true;
}

bool ModificationItem::parse(const char*& p) {
	type=MODIFICATION_TYPE_SIZE;
	if(*p=='+'){
		p++;
		skipSpaces(p);
		if(!parseLI(p,0,'+')){
			RET
		}

		skipSpaces(p);
		if(*p++!=','){
			RET
		}

		skipSpaces(p);
		if(!readAlphabetString(from,p)){
			RET
		}

		type=MODIFICATION_TYPE_INSERT;
		return true;
	}
	else if(*p=='-'){
		p++;
		skipSpaces(p);
		if(!parseLI(p,0,'-')){
			RET
		}

		skipSpaces(p);
		if(*p==','){
			p++;
			skipSpaces(p);
			if(!parseLI(p,1,'-')){
				RET
			}

			if(!l[1] && in[1]==0){//-L-4,0 or -4,0
				RET
			}

			if( (l[0] && !l[1]) || (!l[0] && l[1]) ){
				if(in[0]+in[1]>0){
					RET
				}
			}

		}
		else{//-4 or -L-3
			if(!l[0] && in[0]==0){
				RET
			}
			/* -4 -> -4,L-4
			 * -L-3 -> -L-3,3
			 */
			l[1]=!l[0];
			in[1]=-in[0];
		}

		type=MODIFICATION_TYPE_SUBSTRING;
		return true;
	}
	else{
		if(!readAlphabetString(from,p)){
			RET
		}
		skipSpaces(p);
		if(*p++!='>'){
			RET
		}

		skipSpaces(p);
		if(!readAlphabetString(to,p) ){
			//no alphabet symbols found, parse "b>0"
			if(*p++=='0'){
				to="";
			}
			else{
				RET
			}
		}

		if(from==to){
			RET
		}

		type=MODIFICATION_TYPE_REPLACE;
		return true;
	}

	return false;
}

#ifndef NDEBUG
void ModificationItem::info()const {
	//should match with ENUM_MODIFICATION_TYPE
	const char *REPLACEMENT_NAMES[]={
		"REPLACE",
		"INSERT",
		"SUBSTRING",
		"ERROR"
	};
	printf("%s ",REPLACEMENT_NAMES[type]);
	if(type==MODIFICATION_TYPE_REPLACE){
		printf("[%s] -> [%s]",from.c_str(),to.c_str());
	}
	else if(type==MODIFICATION_TYPE_INSERT){
		printf("%s %s%d",from.c_str()
				, l[0] ? "L":""	,in[0]
				);
	}
	else if(type==MODIFICATION_TYPE_SUBSTRING){
		printf("%s%d %s%d"
				, l[0] ? "L":""	,in[0]
				, l[1] ? "L":""	,in[1]
				);
	}
	else if(type==MODIFICATION_TYPE_SIZE){
		printf("error%d",errorLine);
	}
	else{
		assert(0);
	}

	printf("\n");
}
#endif
