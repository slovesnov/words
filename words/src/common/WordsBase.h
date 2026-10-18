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

class WordsBase {
  std::string m_ev;  // locale m_ev=utf8ToLocale(m_entryValue[ENTRY_TEMPLATE])
  VString m_evwords; // locale
  VString m_tvwords; // locale

protected:
  std::string m_entryValue[ENTRY_SIZE]; // utf8
  bool m_checkValue;
  int m_comboValue[COMBOBOX_SIZE];
  int m_radioValue;
  int m_sortlistValue;
  std::string m_textViewValueUtf8; // do splitr so need utf8
  std::array<int, BUTTON_SIZE>
      m_buttonValue; // use array allow assign all values

  Dictionary m_dictionary[DICTIONARY_SIZE];
  std::string m_keyboardOneRow[256][2];
  std::string m_keyboardRowDiagonals[256];
  ENUM_MENU m_menuClick; // last search option
  std::string m_out;
  VectorSearchResult m_result;
  std::vector<VectorSearchResult> m_thread_result;
  int m_longestWordLength[LANGUAGES];
  clock_t m_begin, m_end;
  UniquePcre2Code m_regex[2];
  UniquePcre2MatchData m_match[2];
  char m_templateHelper[256];
  std::vector<std::vector<char>> m_template_a;
  Modification m_modifications;
  std::array<std::string, STRING_SIZE> m_languageAll[LANGUAGES];   // utf8
  std::array<std::string, MENU_SIZE> m_menuAll[LANGUAGES];         // utf8
  std::array<std::string, SETTINGS_SIZE> m_settingsAll[LANGUAGES]; // locale
  std::string m_addstatus;
#ifndef NOGTK
  int m_filteredWordsCount;
#endif
  // multithread variables
  VectorThreadResult m_tr;
  std::vector<VectorMapStringTwoStringVectors> m_ma;
  VString m_vs;
  VectorInt m_chd;
  std::vector<MapAnagram> m_anagrams;
  std::vector<LetterGroupSplitMap> m_eqmapt;
  LetterGroupSplitMap m_eqmap;
  VectorStringInt m_si;
  int m_threads;
  std::stop_token m_token;
  std::unique_ptr<std::latch> m_latch;
  std::mutex m_mutex;
  std::vector<VectorBool> m_chain;
  MapIntInt m_wordLengthMap[LANGUAGES];
  VectorIntInt m_iiv;
  std::atomic<int> m_i;
  VVString m_vvs;

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
  IntInt iteratorsSize(int nthread, int total_size);
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
  void clusterStatistics(int nthread);
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

  void clusterStatisticsPreProseeding();
  void findLetterGroupSplitPreProseeding();
  void checkKeyboardWordSimplePreProseeding();
  void checkKeyboardWordComplexPreProseeding();
  void checkDictionaryPreProseeding();

  void anagramsPostProseeding();
  void clusterStatisticsPostProseeding();
  void findLetterGroupSplitPostProseeding();
  void simpleDoubleWordSequencePostProseeding();
  void dictionaryStatisticsPostProseeding();
  void checkDictionaryPostProseeding();
  void twoCharactersDistributionPostProseeding();

  void twoDictionaries(int nthread, int n);

  void fillDictionaryLength(int nthread, int length,
                            const SetString &exceptions);
  int findLongestChain(int nthread, int wordIndex,
                       const SetString &exceptions = {});

  bool chainAddIfFound(int nthread, const std::string &s, VectorInt &v,
                       const VectorInt &otherSide = {});
  std::pair<VectorInt, bool> chainLayer(int nthread,
                                        const VectorInt &layerIndex,
                                        const VectorInt &otherSide = {});
  std::pair<VectorInt, bool> chainLayer(int nthread, const std::string &word,
                                        const VectorInt &otherSide = {});
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

  static std::string intToString(int v, char c = ',');
  std::string intToStringLocaled(int v);
  void sortFilterResults(ENUM_JOB_TYPE e);

  void loadLanguages();

  static std::string pairsToString(VectorStringString const &v, bool p = 0);

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
  virtual void updateProceedingThread();
  void updateProceedingText(std::string s, bool add = true);
  const MapIntInt &getWordLengthMap() const;
  std::array<VectorInt, 2> relations(int nthread, VectorInt const &v1,
                                     VectorInt const &v2);
  virtual void showDrawingArea() {};
};

extern WordsBase *wordsBase;