/*
 * ModificationItem.h
 *
 *  Created on: 17.11.2017
 *      Author: alexey slovesnov
 */

#ifndef MODIFICATIONITEM_H_
#define MODIFICATIONITEM_H_

#include "enums.h"
#include <string>

class ModificationItem {
	std::string from, to;
	bool l[2];
	int in[2];
#ifndef NDEBUG
	int errorLine;
#endif
public:
	ENUM_MODIFICATION_TYPE type;
private:
	bool parseLI(const char *&p, int i, char operation);

	inline int countIn(int i, const std::string &s) const {
		return l[i] ? s.length() + in[i] : in[i];
	}

	bool readAlphabetString(std::string &s, const char *&p);
public:
	static void skipSpaces(const char *&p);

#ifndef NDEBUG
	void info()const;
#endif

	//more convenient use const char*
	bool parse(const char *&p);

	std::string apply(const std::string &s) const;

};

#endif /* MODIFICATIONITEM_H_ */
