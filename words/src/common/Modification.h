/*
 * Modification.h
 *
 *  Created on: 17.11.2017
 *      Author: alexey slovesnov
 */

#ifndef MODIFICATION_H_
#define MODIFICATION_H_

#include "ModificationItem.h"
#include <vector>

typedef std::vector<ModificationItem> ModificationItemVector;
typedef ModificationItemVector::const_iterator ModificationItemVectorCI;

class Modification {
	ModificationItemVector vr;
public:
	bool parse(const std::string& s);
	std::string apply(const std::string& s, bool everyOperationChangesWord=false);

#ifndef NDEBUG
	std::string applyStepByStep(const std::string& s);
	void info();
#endif

};

#endif /* MODIFICATION_H_ */
