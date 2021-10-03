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

#ifdef CGI
	#define G_N_ELEMENTS(arr)		(sizeof (arr) / sizeof ((arr)[0]))
	#include <regex>
	#include <stdarg.h>
	#include <stdio.h>
#else
	#include <gtk/gtk.h>

#define GP2INT(a) int(int64_t(a))
#define GP(a) gpointer(int64_t(a))

#endif

const char SEPARATOR[]="SEPARATOR";
const std::string LNG[]={"en","ru"};
const std::string LANGUAGE[]={"english","russian"};

class WordsBase;
extern WordsBase*wordsBase;

//Note if use "const std::string&" instead of "const std::string" program works really much faster 0.05 -> 0.01
typedef bool (WordsBase::*BOOL_STRING_WORDSBASE_FUNCTION)(const std::string&);
typedef bool (WordsBase::*BOOL_VOID_WORDSBASE_FUNCTION)();

//========= BEGIN define println & printinfo functions ===========
/* to remove occurences println & printinfo before final release just commment them and try to compile
 */

#ifndef NDEBUG

	#ifdef CGI
		#define g_print printf
		#define G_DIR_SEPARATOR '/'
	#endif

#define println(f, ...)  {\
	char __s[1024],__b[127];\
	sprintf(__b,__FILE__);\
	char*__p=strrchr(__b,G_DIR_SEPARATOR);\
	sprintf(__s,f,##__VA_ARGS__);\
	g_print("%-40s %s:%d %s()\n",__s,__p==NULL?__b:__p+1,__LINE__,__func__);\
}

#define printinfo println(" ")

#else

#define println(f, ...)  ((void)0)
#define printinfo

#endif

//========= END define println & printinfo functions ===========

#define IN_ARRAY(array,item) (INDEX_OF(array,item)!=-1)
#define INDEX_OF(array,item) indexOf(array,G_N_ELEMENTS(array),item)

class WordsBase {
#ifdef CGI
	static std::string encode(const std::string& s, bool toUtf8);
#endif

	//prepare addons before search;
	void setTemplateSearch();
	void setKeyboardOneRow();
	void setKeyboardRowDiagonals();

protected:
	static const int LANGUAGES=G_N_ELEMENTS(LNG);
	StringSet m_dictionary[LANGUAGES];
	StringVector m_settings[LANGUAGES];//m_settings[i] see ENUM_SETTINGS {encoding=locale}
#ifndef CGI
	StringVector m_template[LANGUAGES];//{encoding=locale}
#endif
	std::string m_keyboardOneRow[256][2];
	std::string m_keyboardRowDiagonals[256];
	ENUM_MENU m_menuClick;//last search option
	std::string m_out;
	SearchResultVector m_result;
	int m_longestWordLength[LANGUAGES];
	clock_t m_begin;
#ifdef CGI
	std::regex m_regex;
#else
	GRegex*m_regex;
	GRegex*m_filterRegex;
	std::string m_filterText;//locale
	int m_filteredWordsCount;
#endif

	int m_languageIndex;
	std::string m_entryText;//locale
	char m_templateHelper[256];
	Modification m_modifications;
	std::string m_chainHelper[2];//locale

	int m_comboValue[COMBOBOX_SIZE];//Note use helper value is faster and thread safe, note m_comboValue[COMBOBOX_DICTIONARY] is not used
	bool m_checkValue;
	StringVector m_language;//utf8
#ifdef CGI
	StringVector m_cgiLanguage;//utf8
#endif

	std::string m_basePath;
	/* some helper strings in language.txt file are very long. String EVERY_MODIFICATION_CHANGE_WORD
	 * length > 1024 (in russain), files language.txt is loaded in two function so use one constant
	 */
	static const int MAX_BUFF_LEN=2048;

	std::string getStatusString();
	std::string getTimeString();

	inline int getMaximumWordLength(){
		return m_longestWordLength[getDictionaryIndex()];
	}

	static bool startsWith(const char*buff,const char*begin){
		return strncmp(buff,begin,strlen(begin))==0;
	}

	static bool startsWith(const char*buff,const std::string& begin){
		return startsWith(buff,begin.c_str());
	}

	static inline std::string getShortLanguageString(int i){
		assert(i>=0 && i<LANGUAGES);
		return LNG[i];
	}

#ifndef CGI
	inline const std::string& getTemplate(int i)const{
		return m_template[getDictionaryIndex()][i];
	}
#endif

	int getDictionaryIndex()const{
		return m_comboValue[COMBOBOX_DICTIONARY];
	}

	FILE* open(int i, std::string s, bool binary=false){
		std::string p=getResourcePath(getShortLanguageString(i) + "/"+s+".txt");
		return fopen(p.c_str(),binary? "rb":"r");
	}

	std::string getResourcePath(std::string name){
		return m_basePath+name;
	}

	bool prepare();

	//used for type=ENUM_MENU and for type=std::string see Words::parsePostParam
	template <class type>static int indexOf(const type a[], const unsigned aSize, const type e){
		unsigned i=std::find(a,a+aSize,e)-a;
		return i==aSize ? -1 : i;
	}

#ifndef CGI
	virtual bool userBreakThread()=0;
	virtual void setMenuLabel(ENUM_MENU e,std::string const& text)=0;
	bool setCheckFilterRegex();
	bool testFilterRegex(const std::string& s);
	void freeRegex(GRegex*r);
#endif

	const std::string& getAlphabet()const{
		return m_settings[getDictionaryIndex()][SETTINGS_ALPHABET];
	}

	int getAlphabetSize()const{
		return getAlphabet().length();
	}

	inline int alphabetIndex(char c)const{
		std::string::size_type k=getAlphabet().find(c);
		return k==std::string::npos ? -1 : k;
	}

	inline static int charIndex(const char*p, char c){
		return strchr(p,c)-p;
	}

	//return true if was user break
	bool run();

	void fillResultFromMap(const MapStringTwoStringVectors& map, size_t len);

public:
	WordsBase(const char* basePath);
	virtual ~WordsBase();

	static int** create2dArray(int dimension1, int dimension2);
	static void delete2dArray(int**p, int dimension1);

	bool checkPangram(const std::string& s);
	bool checkTemplate(const std::string& s);
	bool checkPalindrome(const std::string& s);
	bool checkCrossword(const std::string& s);
	bool checkRegularExpression(const std::string& s);
	bool checkCharacterSequence(const std::string& s);
	bool checkConsonantVowelSequence(const std::string& s);
	bool checkDensity(const std::string& s);

	bool checkKeyboardWordSimple(const std::string& s);
	bool checkKeyboardWordComplex(const std::string& s);

	//return true if was user break like run() function, order is the same with BOOL_VOID_FUNCTION
	bool findAnagram();
	bool findSimpleWordSequence();
	bool findDoubleWordSequence();
	bool findWordSequenceFull();
	bool findModification();
	bool findChain();
	bool twoDictionariesSimple(){
		return twoDictionaries(false);
	}
	bool twoDictionariesTranslit(){
		return twoDictionaries(true);
	}
	bool keyboardWords();
	bool dictionaryStatistics();
	bool wordFrequency();
	bool checkDictionary();
	bool twoCharactersDistribution();
	bool dummy(){return false;}//Helper wrapper function for language change menu
	//helper BOOL_VOID_FUNCTION
	bool twoDictionaries(bool translit);

	static std::string replaceAll(std::string subject, const std::string& from,const std::string& to);
	static bool spanIncluding(const char* p, const std::string& pattern);
	static bool differenceOnlyOneChar(std::string const& a,std::string const& b);

	const std::string& getAlphabet(){
		return getSettings(SETTINGS_ALPHABET);
	}

	const char getAlphabetChar(int i){
		return getAlphabet()[i];
	}

	const std::string& getVowelConsonant(bool consonant){
		return getSettings(consonant ? SETTINGS_CONSONANTS : SETTINGS_VOWELS);
	}

	inline const std::string& getSettings(ENUM_SETTINGS e){
		return m_settings[getDictionaryIndex()][e];
	}

	inline StringSet const& getDictionary()const{
		return m_dictionary[getDictionaryIndex()];
	}

	bool isAlphabetChar(const char p){
		return getAlphabet().find(p)!=std::string::npos;
	}

	//convert localed "s" to lowercase in cp1251
	static std::string localeToLowerCase(const std::string& s, bool onlyRussainChars=false);
	static std::string utf8ToLowerCase(const std::string& s, bool onlyRussainChars=false) {
		return localeToUtf8(localeToLowerCase(utf8ToLocale(s),onlyRussainChars));
	}

	static const std::string localeToUtf8(const std::string& s);
	static const std::string utf8ToLocale(const std::string& s);

	/* remove '\n' or '\r\n' under unix at the end of string
	 */
	static void removeLastCRLF(char*p);

	static std::string format(const char * format, ... );

	static std::string intToString(int v,char separator);
	std::string intToString(int v);
	void sortResults();

	void loadLanguage();
	static const std::string parseString(const char* buff);

};

#endif /* COMMON_WORDSBASE_H_ */
