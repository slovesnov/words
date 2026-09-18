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

const char SEPARATOR[] = "SEPARATOR";
constexpr std::string LANGUAGE[] = {"english", "russian"};
constexpr int LANGUAGES = SIZEI(LANGUAGE);

class WordsBase;
extern WordsBase *wordsBase;

// Note if use "const std::string&" instead of "const std::string" program works
// really much faster 0.05 -> 0.01
typedef bool (WordsBase::*BOOL_STRING_WORDSBASE_FUNCTION)(const std::string &);
typedef bool (WordsBase::*BOOL_VOID_WORDSBASE_FUNCTION)();

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
#else
  GRegex *m_regex;
  GRegex *m_filterRegex;
  std::string m_filterText; // locale
  int m_filteredWordsCount;
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
  std::string m_programVersion;
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

  // return true if was user break
  bool run();

  void fillResultFromMap(const MapStringTwoStringVectors &map, size_t len);

  //todo
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
  // BOOL_VOID_FUNCTION
  bool findAnagram();
  bool findSimpleWordSequence();
  bool findDoubleWordSequence();
  bool findWordSequenceFull();
  bool findModification();
  bool findChain();
  bool findLetterGroupSplit();
  bool twoDictionariesSimple() { return twoDictionaries(false); }
  bool twoDictionariesTranslit() { return twoDictionaries(true); }
  bool keyboardWords();
  bool dictionaryStatistics();
  bool wordFrequency();
  bool checkDictionary();
  bool twoCharactersDistribution();
  bool dummy() { return false; }

  static std::string getTwoDictionariesPath(bool translit);
  // Helper wrapper function for language change menu
  // helper BOOL_VOID_FUNCTION
  bool twoDictionaries(bool translit);

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
};
