/*
 * WordsBase.h
 *
 *  Created on: 20.11.2017
 *      Author: alexey slovesnov
 */

#pragma once

#define STD_THREAD

#include "HelperStructs.h"
#include "Modification.h"
#include "SearchResult.h"
#include "aslov.h"
#include <algorithm>
#include <array>
#include <cstring>
#include <ctime>

#ifdef NOGTK
#define USE_STANDARD_REGEX
#include <regex>
#endif

#ifdef STD_THREAD
#include <thread>
#endif

#define pr2(...)                                                               \
  print_variables(__VA_ARGS__);                                                \
  std::cout << "\n";

const char SEPARATOR[] = "SEPARATOR";
constexpr std::string LANGUAGE[] = {"english", "russian"};
constexpr int LANGUAGES = SIZEI(LANGUAGE);
const std::string invalidDifference = "$";

class WordsBase;
extern WordsBase *wordsBase;

class WordsBase {
  // prepare addons before search;
  void setKeyboardOneRow();
  void setKeyboardRowDiagonals();

protected:
  StringSet m_dictionary[LANGUAGES];
  VString m_settings[LANGUAGES]; // m_settings[i] see ENUM_SETTINGS
                                 // {encoding=locale}
#ifndef NOGTK
  VString m_template[LANGUAGES]; //{encoding=locale}
#endif
  std::string m_keyboardOneRow[256][2];
  std::string m_keyboardRowDiagonals[256];
  ENUM_MENU m_menuClick; // last search option
  std::string m_out;
  SearchResultVector m_result;
  int m_longestWordLength[LANGUAGES];
  clock_t m_begin, m_end;
  bool m_outSplitted;
  // need fast compile so not include gtk files and use std::regex in console
  // mode
#ifdef USE_STANDARD_REGEX
  std::regex m_regex;
#endif

  int m_languageIndex;
  std::string m_entryText; // locale
  char m_templateHelper[256];
  std::vector<std::vector<char>> m_template_a;
  Modification m_modifications;
  std::string m_chainHelper[2]; // locale

  int m_comboValue
      [COMBOBOX_SIZE]; // Note use helper value is faster and thread safe, note
                       // m_comboValue[COMBOBOX_DICTIONARY] is not used
  int m_radioValue;    // todo in cgi mode
  bool m_checkValue;
  VString m_language; // utf8
  std::string m_addstatus;
#ifdef NOGTK
  VString m_cgiLanguage; // utf8
#else
  GRegex *m_regex[2];
  std::string m_filterText; // locale
  int m_filteredWordsCount;
  std::string m_programVersion;
  void freeRegex(int i);
#endif
std::vector<StringSetCI> m_it[LANGUAGES];

#ifdef STD_THREAD
  std::stop_token m_token;
#endif

  std::string getStatusString();
  std::string getTimeString();

  inline int getMaximumWordLength() {
    return m_longestWordLength[getDictionaryIndex()];
  }

  bool prepare();
  static std::string getShortLanguageString(int i);
  void setDictionaryIndex(int i);
  int getDictionaryIndex() const;

  static std::string path(int i, std::string s);
  static VString readFile(int i, std::string s);
  static VString readFile(std::string path);

#ifdef NOGTK
  static std::string getResourcePath(std::string name);
  void cgi();
#else
  inline const std::string &getTemplate(int i) const {
    return m_template[getDictionaryIndex()][i];
  }
  virtual bool userBreakThread() = 0;
  virtual void setMenuLabel(ENUM_MENU e, std::string const &text) = 0;
  virtual void setSortCombosState(bool enable)=0;
  virtual void endJobThread()=0;
  bool setCheckFilterRegex();
  bool testFilterRegex(const std::string &s);
#endif

  const std::string &getAlphabet() const {
    return m_settings[getDictionaryIndex()][SETTINGS_ALPHABET];
  }

  int getAlphabetSize() const { return getAlphabet().length(); }

  inline int alphabetIndex(char c) const {
    std::string::size_type k = getAlphabet().find(c);
    return k == std::string::npos ? -1 : k;
  }

  void run();
  void run_thread(int nthread);

  void fillResultFromMap(const MapStringTwoStringVectors &map, size_t len);

  // todo
  void test();
#ifdef NOGTK
  void checkLFAllFiles();
  void showLongestAnagram();            // for MAX_ANAGRAM_LENGTH
  void showLongestPangram();            // for MAX_PANGRAM_LENGTH
  void showLongestSimpleWordSequence(); // for MAX_WORD_SEQUENCE_LENGTH
  void showLongestDoubleWordSequence(); // for MAX_DOUBLE_WORD_SEQUENCE_LENGTH
#endif

public:
  WordsBase();
  virtual ~WordsBase();

  bool checkPangram(const std::string &s);
  bool checkTemplate(const std::string &s);
  bool checkPalindrome(const std::string &s);
  bool checkCrossword(const std::string &s);
  bool checkRegularExpression(const std::string &s);
  bool checkCharacterSequence(const std::string &s);
  bool checkConsonantVowelSequence(const std::string &s);
  bool checkDensity(const std::string &s);

  bool checkKeyboardWordSimple(const std::string &s);
  bool checkKeyboardWordComplex(const std::string &s);

  // return true if was user break like run() function, order is the same with
  bool findAnagram(int nthread);
  bool findSimpleWordSequence(int nthread);
  bool findDoubleWordSequence(int nthread);
  bool findWordSequenceFull(int nthread);
  bool findModification(int nthread);
  bool findChain(int nthread);
  bool findLetterGroupSplit(int nthread);
  bool twoDictionariesSimple(int nthread) {
    return twoDictionaries(nthread, false);
  }
  bool twoDictionariesTranslit(int nthread) {
    return twoDictionaries(nthread, true);
  }
  bool keyboardWords(int nthread);
  bool dictionaryStatistics(int nthread);
  bool wordFrequency(int nthread);
  bool checkDictionary(int nthread);
  bool twoCharactersDistribution(int nthread);

  static std::string getTwoDictionariesPath(bool translit);
  bool twoDictionaries(int nthread, bool translit);

  static int differentChars(std::string_view s);
  static bool spanIncluding(std::string_view p, std::string_view pattern) {
    return p.find_first_not_of(pattern) == std::string_view::npos;
  }
  static bool differenceOnlyOneChar(std::string const &a, std::string const &b);

  const std::string &getAlphabet() { return getSettings(SETTINGS_ALPHABET); }

  const char getAlphabetChar(int i) { return getAlphabet()[i]; }

  const std::string &getVowelConsonant(bool consonant) {
    return getSettings(consonant ? SETTINGS_CONSONANTS : SETTINGS_VOWELS);
  }

  inline const std::string &getSettings(ENUM_SETTINGS e) {
    return m_settings[getDictionaryIndex()][e];
  }

  inline StringSet const &getDictionary() const {
    return m_dictionary[getDictionaryIndex()];
  }

  bool isAlphabetChar(const char p) {
    return getAlphabet().find(p) != std::string::npos;
  }

  std::string intToStringLocaled(int v);
  void sortFilterResults();

  void loadLanguage();

  static std::string sub(std::string const &minuend,
                         std::string const &subtrahend);
  static std::string getOrderedString(std::string const &s);
  static std::string getUserString(std::string const &s);
  static StringStringVector
  getAllPairs(std::string const &s, std::string const &low = invalidDifference);
  static std::string pairsToString(StringStringVector const &v, bool p = 0);

};
