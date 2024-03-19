/*
 * WordsBase.h
 *
 *  Created on: 20.11.2017
 *      Author: alexey slovesnov
 */

#ifndef COMMON_WORDSBASE_H_
#define COMMON_WORDSBASE_H_

#include "SearchResult.h"
#include "HelperStructs.h"
#include "Modification.h"
#include <cassert>
#include <cstring>
#include <algorithm>
#include <ctime>
#include "aslov.h"

#ifdef CGI
//	#include <regex>
//	#include <stdarg.h>
//	#include <stdio.h>
#endif

const char SEPARATOR[] = "SEPARATOR";
const std::string LNG[] = { "en", "ru" };
const std::string LANGUAGE[] = { "english", "russian" };

class WordsBase;
extern WordsBase *wordsBase;

//Note if use "const std::string&" instead of "const std::string" program works really much faster 0.05 -> 0.01
typedef bool (WordsBase::*BOOL_STRING_WORDSBASE_FUNCTION)(const std::string&);
typedef bool (WordsBase::*BOOL_VOID_WORDSBASE_FUNCTION)();

class WordsBase {
	//prepare addons before search;
	void setTemplateSearch();
	void setKeyboardOneRow();
	void setKeyboardRowDiagonals();

protected:
	static const int LANGUAGES = SIZEI(LNG);
	StringSet m_dictionary[LANGUAGES];
	VString m_settings[LANGUAGES]; //m_settings[i] see ENUM_SETTINGS {encoding=locale}
#ifndef CGI
	VString m_template[LANGUAGES]; //{encoding=locale}
#endif
	std::string m_keyboardOneRow[256][2];
	std::string m_keyboardRowDiagonals[256];
	ENUM_MENU m_menuClick; //last search option
	std::string m_out;
	SearchResultVector m_result;
	int m_longestWordLength[LANGUAGES];
	clock_t m_begin, m_end;
	bool m_outSplitted;
#ifdef _WIN32
	//under windows use powerful regex from gtk even in cgi mode
	GRegex *m_regex;
	GRegex *m_filterRegex;
	std::string m_filterText; //locale
	int m_filteredWordsCount;
#else
	std::regex m_regex;
#endif

	int m_languageIndex;
	std::string m_entryText; //locale
	char m_templateHelper[256];
	Modification m_modifications;
	std::string m_chainHelper[2]; //locale

	int m_comboValue[COMBOBOX_SIZE]; //Note use helper value is faster and thread safe, note m_comboValue[COMBOBOX_DICTIONARY] is not used
	bool m_checkValue;
	VString m_language; //utf8
	std::string m_addstatus;
#ifdef CGI
	VString m_cgiLanguage;//utf8
#else
	std::string m_programVersion;
#endif

	/* some helper strings in language.txt file are very long. String EVERY_MODIFICATION_CHANGE_WORD
	 * length > 1024 (in russain), files language.txt is loaded in two function so use one constant
	 */
	static const int MAX_BUFF_LEN = 2048;

	std::string getStatusString();
	std::string getTimeString();

	inline int getMaximumWordLength() {
		return m_longestWordLength[getDictionaryIndex()];
	}

	static inline std::string getShortLanguageString(int i) {
		assert(i>=0 && i<LANGUAGES);
		return LNG[i];
	}

#ifndef CGI
	inline const std::string& getTemplate(int i) const {
		return m_template[getDictionaryIndex()][i];
	}
#endif

	int getDictionaryIndex() const {
		return m_comboValue[COMBOBOX_DICTIONARY];
	}

	FILE* open(int i, std::string s, int mode = 0);
#ifdef CGI
	std::string getResourcePath(std::string name);
	void cgi();
#endif

	bool prepare();

#ifndef CGI
	virtual bool userBreakThread()=0;
	virtual void setMenuLabel(ENUM_MENU e, std::string const &text)=0;
	bool setCheckFilterRegex();
	bool testFilterRegex(const std::string &s);
#endif

	const std::string& getAlphabet() const {
		return m_settings[getDictionaryIndex()][SETTINGS_ALPHABET];
	}

	int getAlphabetSize() const {
		return getAlphabet().length();
	}

	inline int alphabetIndex(char c) const {
		std::string::size_type k = getAlphabet().find(c);
		return k == std::string::npos ? -1 : k;
	}

	//return true if was user break
	bool run();

	void fillResultFromMap(const MapStringTwoStringVectors &map, size_t len);

public:
	WordsBase();
	virtual ~WordsBase();

	bool checkPangram(const std::string &s);
	bool checkTemplate(const std::string &s);
	bool checkPalindrome(const std::string &s);
	bool checkCrossword(const std::string &s);
	bool checkRegularExpression(const std::string &s);
	bool checkCharacterSequence(const std::string &s);
	bool checkLetterGroupSplit(const std::string &s);
	bool checkConsonantVowelSequence(const std::string &s);
	bool checkDensity(const std::string &s);

	bool checkKeyboardWordSimple(const std::string &s);
	bool checkKeyboardWordComplex(const std::string &s);

	//return true if was user break like run() function, order is the same with BOOL_VOID_FUNCTION
	bool findAnagram();
	bool findSimpleWordSequence();
	bool findDoubleWordSequence();
	bool findWordSequenceFull();
	bool findModification();
	bool findChain();
	bool findLetterGroupSplit();
	bool twoDictionariesSimple() {
		return twoDictionaries(false);
	}
	bool twoDictionariesTranslit() {
		return twoDictionaries(true);
	}
	bool keyboardWords();
	bool dictionaryStatistics();
	bool wordFrequency();
	bool checkDictionary();
	bool twoCharactersDistribution();
	bool dummy() {
		return false;
	} //Helper wrapper function for language change menu
	  //helper BOOL_VOID_FUNCTION
	bool twoDictionaries(bool translit);

	static bool spanIncluding(const char *p, const std::string &pattern);
	static bool differenceOnlyOneChar(std::string const &a,
			std::string const &b);

	const std::string& getAlphabet() {
		return getSettings(SETTINGS_ALPHABET);
	}

	const char getAlphabetChar(int i) {
		return getAlphabet()[i];
	}

	const std::string& getVowelConsonant(bool consonant) {
		return getSettings(consonant ? SETTINGS_CONSONANTS : SETTINGS_VOWELS);
	}

	inline const std::string& getSettings(ENUM_SETTINGS e) {
		return m_settings[getDictionaryIndex()][e];
	}

	inline StringSet const& getDictionary() const {
		return m_dictionary[getDictionaryIndex()];
	}

	bool isAlphabetChar(const char p) {
		return getAlphabet().find(p) != std::string::npos;
	}

	/* remove '\n' or '\r\n' under unix at the end of string
	 */
	static void removeLastCRLF(char *p);

	std::string intToStringLocaled(int v);
	void sortFilterResults();

	void loadLanguage();
	static const std::string parseString(const char *buff);

};

#endif /* COMMON_WORDSBASE_H_ */
