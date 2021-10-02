/*
 * HelperStructs.cpp
 *
 *  Created on: 21.12.2016
 *      Author: alexey slovesnov
 */

#include "HelperStructs.h"

bool sortIntDouble(const IntDouble& r1,const IntDouble& r2) {
	return r1.second>r2.second;
}

bool sortIntInt(const IntInt& r1,const IntInt& r2) {
	return r1.first>r2.first;
}

bool sortStringInt(const StringInt& r1, const StringInt& r2){
	return r1.second==r2.second ? (r1.first<r2.first) : r1.second>r2.second;
}
