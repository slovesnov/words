/*
 * WordsBase.h
 *
 *  Created on: 20.11.2017
 *      Author: alexey slovesnov
 */

#pragma once

#include "Modification.h"
#include "consts.h"
#include <latch>
#include <thread>

#define PCRE2_CODE_UNIT_WIDTH 8
#define PCRE2_STATIC
#include <pcre2.h>
using UniquePcre2Code = UniqueGtkResource<pcre2_code, pcre2_code_free>;
using UniquePcre2MatchData =
    UniqueGtkResource<pcre2_match_data, pcre2_match_data_free>;

const char SEPARATOR[] = "SEPARATOR";

class WordsBase;
extern WordsBase *wordsBase;

class WordsBase {
  std::string m_ev; // locale m_ev=utf8ToLocale(m_entryValue[ENTRY_TEMPLATE])

protected:
  std::string m_entryValue[ENTRY_SIZE]; // utf8
  bool m_checkValue;
  int m_comboValue[COMBOBOX_SIZE];
  int m_radioValue;
  int m_sortlistValue;
  std::string m_textViewValue; // locale
  std::array<int, BUTTON_SIZE>
      m_buttonValue; // use array allow assign all values

  Dictionary m_dictionary[DICTIONARY_SIZE];
  std::string m_keyboardOneRow[256][2];
  std::string m_keyboardRowDiagonals[256];
  ENUM_MENU m_menuClick; // last search option
  std::string m_out;
  SearchResultVector m_result;
  std::vector<SearchResultVector> m_thread_result;
  int m_longestWordLength[LANGUAGES];
  clock_t m_begin, m_end;
  UniquePcre2Code m_regex[2];
  UniquePcre2MatchData m_match[2];
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
  std::vector<AnagramMap> m_anagrams;
  std::vector<LetterGroupSplitMap> m_eqmapt;
  LetterGroupSplitMap m_eqmap;
  StringIntVector m_si;
  int m_threads;
  std::stop_token m_token;
  std::unique_ptr<std::latch> m_latch;
  std::mutex m_mutex;

  std::string getStatusString();
  std::string getTimeString();

  int getMaximumWordLength() {
    return m_longestWordLength[getDictionaryIndex()];
  }

  bool prepare();
#ifdef NOGTK
  void setDictionaryIndex(int i);
  // void test();
  void showLongestAnagram();            // for MAX_ANAGRAM_LENGTH
  void showLongestPangram();            // for MAX_PANGRAM_LENGTH
  void showLongestSimpleWordSequence(); // for MAX_WORD_SEQUENCE_LENGTH
  void showLongestDoubleWordSequence(); // for MAX_DOUBLE_WORD_SEQUENCE_LENGTH
#endif

  static std::string path(int i, std::string s);
  static VString readFile(int i, std::string s);
  static VString readFile(std::string path);

#ifdef NOGTK
  static std::string getResourcePath(std::string name);
  void cgi();
#else
  bool testFilterRegex(const std::string &s);
#endif

public:
  WordsBase();

  void run(ENUM_JOB_TYPE e);
  void run_thread(int nthread);
  PairDCIDCI iterators(int nthread, int n = -1);

  bool checkPangram(const std::string &s);
  bool checkTemplate(const std::string &s);
  bool checkPalindrome(const std::string &s);
  bool checkCrossword(const std::string &s);

  bool checkRegularExpression(const std::string &s, pcre2_code *re,
                              pcre2_match_data *match_data);
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
  void twoDictionariesStrict(int nthread) { twoDictionaries(nthread, 0); }
  void twoDictionariesSimple(int nthread) { twoDictionaries(nthread, 1); }
  void twoDictionariesTranslit(int nthread) { twoDictionaries(nthread, 2); }
  void keyboardWords(int nthread);
  void dictionaryStatistics(int nthread);
  void wordFrequency(int nthread);
  void checkDictionary(int nthread);
  void twoCharactersDistribution(int nthread);
  void findWordsSplit(int nthread);

  void findLetterGroupSplitPreProseeding();
  void checkKeyboardWordSimplePreProseeding();
  void checkKeyboardWordComplexPreProseeding();
  void checkDictionaryPreProseeding();

  void anagramsPostProseeding();
  void findLetterGroupSplitPostProseeding();
  void simpleDoubleWordSequencePostProseeding();
  void dictionaryStatisticsPostProseeding();
  void wordFrequencyPostProseeding();
  void checkDictionaryPostProseeding();
  void twoCharactersDistributionPostProseeding();

  void twoDictionaries(int nthread, int n);

  static int differentChars(std::string_view s);
  static bool spanIncluding(std::string_view p, std::string_view pattern) {
    return p.find_first_not_of(pattern) == std::string_view::npos;
  }
  static bool differenceOnlyOneChar(std::string const &a, std::string const &b);

  int getLanguageIndex() const { return m_buttonValue[BUTTON_LANGUAGE]; }
  int getDictionaryIndex() const { return m_buttonValue[BUTTON_DICTIONARY]; }

  Dictionary const &getDictionary() const {
    return m_dictionary[getDictionaryIndex()];
  }

  std::string intToStringLocaled(int v);
  void sortFilterResults(ENUM_JOB_TYPE e);

  void loadLanguages();

  static std::string pairsToString(StringStringVector const &v, bool p = 0);

  bool createRegex(ENUM_ENTRY e);
  bool createRegex(ENUM_ENTRY e, UniquePcre2Code &r, UniquePcre2MatchData &m);

  const std::string &alphabet() const;
  int alphabetSize() const;
  int alphabetIndex(char c) const;
  const std::string &vowelConsonant(bool consonant);
  bool isAlphabetChar(char c) const;
  const std::string &string(ENUM_STRING e) const;
  const std::string &string(ENUM_MENU e) const;
  const std::string string(ENUM_STRING e, ENUM_STRING e1) const;
  const std::string &string(int i) const;
  const std::string &stringUsingDictionary(ENUM_STRING e,
                                           bool opposite = false) const;
  const std::string &settings(ENUM_SETTINGS e, int i = 0) const;
};
