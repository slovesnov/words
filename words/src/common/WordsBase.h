/*
 * WordsBase.h
 *
 *  Created on: 20.11.2017
 *      Author: alexey slovesnov
 */

#pragma once

#include "HelperStructs.h"
#include "Modification.h"
#include "SearchResult.h"
#include "aslov.h"
#include "consts.h"
#include <algorithm>
#include <array>
#include <cstring>
#include <ctime>
#include <thread>

#ifdef USE_STANDARD_REGEX
#include <regex>
#endif

#include <mutex> //TODO
extern std::mutex cout_mutex;
#define prsync(...)                                                            \
  {                                                                            \
    std::lock_guard<std::mutex> lock(cout_mutex);                              \
    prs(__VA_ARGS__);                                                          \
  }

const char SEPARATOR[] = "SEPARATOR";
constexpr std::string LANGUAGE[] = {"english", "russian"};
constexpr int LANGUAGES = SIZEI(LANGUAGE);
const std::string invalidDifference = "$";

class WordsBase;
extern WordsBase *wordsBase;

class WordsBase {
  std::string m_entryValue;    // locale
  std::string m_textViewValue; // locale
  bool m_checkValue;

protected:
  int m_comboValue[COMBOBOX_SIZE]; // set in frame.cpp
  int m_radioValue;                // set in frame.cpp

  Dictionary m_dictionary[DICTIONARY_SIZE];
  std::string m_keyboardOneRow[256][2];
  std::string m_keyboardRowDiagonals[256];
  ENUM_MENU m_menuClick; // last search option
  std::string m_out;
  SearchResultVector m_result;
  std::vector<SearchResultVector> m_thread_result;
  int m_longestWordLength[LANGUAGES];
  clock_t m_begin, m_end;
#ifdef USE_STANDARD_REGEX
  std::regex m_regex;
#else
  SafeGRegex m_regex[2];
#endif
  int m_languageIndex;
  int m_dictionaryIndex;
  char m_templateHelper[256];
  std::vector<std::vector<char>> m_template_a;
  Modification m_modifications;
  std::string m_chainHelper[2];                                    // locale
  std::array<std::string, STRING_SIZE> m_languageAll[LANGUAGES];   // utf8
  std::array<std::string, MENU_SIZE> m_menuAll[LANGUAGES];         // utf8
  std::array<std::string, SETTINGS_SIZE> m_settingsAll[LANGUAGES]; // locale
  std::string m_addstatus;
#ifndef NOGTK
  int m_filteredWordsCount;
#endif
  // multithread variables
  ThreadResultVector m_tr;
  std::vector<IntVector> m_iv;
  std::vector<VMapStringTwoStringVectors> m_ma;
  VString m_chdv;
  std::vector<int> m_chd;

  std::stop_token m_token;

  std::string getStatusString();
  std::string getTimeString();

  int getMaximumWordLength() {
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
  bool testFilterRegex(const std::string &s);
#endif

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

  void run(ENUM_JOB_TYPE e);
  void run_thread(int nthread);
  PairDCIDCI iterators(int n, int nthread);
  PairDCIDCI iterators(ENUM_DICTIONARY e, int nthread);

  bool checkPangram(const std::string &s);
  bool checkTemplate(const std::string &s);
  bool checkPalindrome(const std::string &s);
  bool checkCrossword(const std::string &s);
  bool checkRegularExpression(const std::string &s, const SafeGRegex &r);
  bool checkRegularExpression(const std::string &s) {
    return checkRegularExpression(s, m_regex[0]);
  }
  bool checkCharacterSequence(const std::string &s);
  bool checkConsonantVowelSequence(const std::string &s);
  bool checkDensity(const std::string &s);

  bool checkKeyboardWordSimple(const std::string &s);
  bool checkKeyboardWordComplex(const std::string &s);

  void findAnagram(int nthread);
  void findSimpleWordSequence(int nthread);
  void findDoubleWordSequence(int nthread);
  void findWordSequenceFull(int nthread);
  void findModification(int nthread);
  void findChain(int nthread);
  void findLetterGroupSplit(int nthread);
  void twoDictionariesSimple(int nthread) { twoDictionaries(nthread, false); }
  void twoDictionariesTranslit(int nthread) { twoDictionaries(nthread, true); }
  void keyboardWords(int nthread);
  void dictionaryStatistics(int nthread);
  void wordFrequency(int nthread);
  void checkDictionary(int nthread);
  void twoCharactersDistribution(int nthread);

  void checkKeyboardWordSimplePreProseeding();
  void checkKeyboardWordComplexPreProseeding();
  void checkDictionaryPreProseeding();

  void dictionaryStatisticsPostProseeding();
  void wordFrequencyPostProseeding();
  void twoCharactersDistributionPostProseeding();
  void simpleDoubleWordSequencePostProseeding();
  void checkDictionaryPostProseeding();

  static std::string getTwoDictionariesPath(bool translit);
  void twoDictionaries(int nthread, bool translit);

  static int differentChars(std::string_view s);
  static bool spanIncluding(std::string_view p, std::string_view pattern) {
    return p.find_first_not_of(pattern) == std::string_view::npos;
  }
  static bool differenceOnlyOneChar(std::string const &a, std::string const &b);

  Dictionary const &getDictionary() const {
    return m_dictionary[getDictionaryIndex()];
  }

  std::string intToStringLocaled(int v);
  void sortFilterResults(ENUM_JOB_TYPE e);

  void loadLanguages();

  static std::string sub(std::string const &minuend,
                         std::string const &subtrahend);
  static std::string getOrderedString(std::string const &s);
  static std::string getUserString(std::string const &s);
  static StringStringVector
  getAllPairs(std::string const &s, std::string const &low = invalidDifference);
  static std::string pairsToString(StringStringVector const &v, bool p = 0);
  virtual std::string getEntryString(ENUM_ENTRY e) const;
  virtual std::string getTextViewString() const;
  virtual bool getCheck() const;

  bool createRegex(ENUM_ENTRY e);
  bool createRegex(ENUM_ENTRY e, SafeGRegex &r);

  const std::string &alphabet() const;
  int alphabetSize() const;
  int alphabetIndex(char c) const;
  const std::string &vowelConsonant(bool consonant);
  bool isAlphabetChar(char c) const;
  const std::string &string(ENUM_STRING e) const;
  const std::string string(ENUM_STRING e,ENUM_STRING e1) const;
  const std::string &string(int i) const;
  const std::string &stringUsingDictionary(ENUM_STRING e) const;
  const std::string &settings(ENUM_SETTINGS e, int i = 0) const;
};
