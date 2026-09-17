/*
 * Modification.h
 *
 *  Created on: 17.11.2017
 *      Author: alexey slovesnov
 */

#pragma once

#include "ModificationItem.h"
#include <vector>

typedef std::vector<ModificationItem> ModificationItemVector;
typedef ModificationItemVector::const_iterator ModificationItemVectorCI;

class Modification {
	ModificationItemVector vr;
public:
	bool parse(const std::string &s);
	std::string apply(const std::string &s, bool everyOperationChangesWord =
			false);

#ifndef NDEBUG
	std::string applyStepByStep(const std::string& s);
	void info();
#endif

};
