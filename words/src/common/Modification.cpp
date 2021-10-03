/*
 * Modification.cpp
 *
 *  Created on: 17.11.2017
 *      Author: alexey slovesnov
 */

#include "Modification.h"
#include "WordsBase.h"

bool Modification::parse(const std::string& s) {
	ModificationItem ri;
	const char*p=s.c_str();//string already in lowercase

	//clear previous parsing
	vr.clear();
	while(1){
		//skip spaces before any search
		ModificationItem::skipSpaces(p);
		if(*p==0){
			return !vr.empty();
		}

		if(!ri.parse(p)){
#ifndef NDEBUG
			vr.clear();//vr.size()==0 means error in info() function
			//ri.info();
#endif
			return false;
		}

		vr.push_back(ri);
	}

}

std::string Modification::apply(const std::string& s, bool everyOperationChangesWord) {
	ModificationItemVectorCI it;
	std::string r=s;
	std::string q;
	for(it=vr.begin();it!=vr.end();it++){
		if(everyOperationChangesWord){
			q=r;
		}
		r=it->apply(r);
		if(r.empty()){
			return "";
		}
		if(everyOperationChangesWord && q==r){
			return "";
		}
	}
	return r;
}

#ifndef NDEBUG
std::string Modification::applyStepByStep(const std::string& s) {
	ModificationItemVectorCI it;
	std::string q=s;
	std::string r=s;
	for(it=vr.begin();it!=vr.end();it++){
		r=it->apply(r);
		q+=" "+r;
		if(r.empty()){
			return "";
		}
	}
	return q;

}
#endif

#ifndef NDEBUG
void Modification::info(){
	if(vr.empty()){
		printf("error");
		return;
	}
	printf("ok sz=%d{\n",int(vr.size()));// 32 & 64bit
	ModificationItemVectorCI it;
	for(it=vr.begin();it!=vr.end();it++){
		it->info();
	}
	printf("}\n");
}
#endif
