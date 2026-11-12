/*
 * WordsBase.h
 *
 *  Created on: 20.11.2017
 *      Author: alexey slovesnov
 */

#pragma once

#include "Parameters.h"
#include "consts.h"
#include <barrier>
#include <functional>
#include <latch>
#include <thread>

class WordsBase : public Parameters {
  Dictionary m_dictionary[DICTIONARY_SIZE];
  std::string m_keyboardOneRow[256][2];
  std::string m_keyboardRowDiagonals[256];
  std::vector<VectorSearchResult> m_thread_result;
  // multithread variables
  VectorThreadResult m_tr;
  std::vector<VectorMapStringTwoStringVectors> m_ma;
  VString m_vs;
  VectorInt m_chd, m_chd1;
  std::vector<MapAnagram> m_anagrams;
  std::vector<LetterGroupSplitMap> m_eqmapt;
  LetterGroupSplitMap m_eqmap;
  VectorStringInt m_si;
  int m_threads;
  std::unique_ptr<std::latch> m_latch;
  using BarrierCallback = std::move_only_function<void() noexcept>;
  std::unique_ptr<std::barrier<BarrierCallback>> m_barrier;
  std::vector<VectorBool> m_chain;
  std::vector<char> m_chainChar;
  VectorIntInt m_iiv;
  std::atomic<int> m_i;
  VVString m_vvs;
  VectorVectorInt m_layerIndex[2], m_vii;
  TableFormat m_table;
  int m_j;
  ENUM_CHAIN_MODE m_chainMode;
  VectorChainLayerResult m_chainLayerResult;
  VectorInt *m_chainPtr[2];
  std::string m_s;
  std::vector<std::vector<char>> m_template_a;
  char m_templateHelper[256];

  void chainMergeLayers() noexcept;
  void chainMergeLayers2() noexcept;
  void chainInit(const VString &vs);

#ifdef NOGTK
  void setDictionaryIndex(int i);
  // void test();
  void showLongestAnagram();            // for MAX_ANAGRAM_LENGTH
  void showLongestPangram();            // for MAX_PANGRAM_LENGTH
  void showLongestSimpleWordSequence(); // for MAX_WORD_SEQUENCE_LENGTH
  void showLongestDoubleWordSequence(); // for MAX_DOUBLE_WORD_SEQUENCE_LENGTH
#endif

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

  template <typename T>
  PairVectorIterators<T> iterators(int nthread, const std::vector<T> &v) {
    int chunk_size = v.size() / m_threads;
    int remainder = v.size() % m_threads;
    int start_idx = nthread * chunk_size + std::min(nthread, remainder);
    int end_idx = start_idx + chunk_size + (nthread < remainder ? 1 : 0);
    return {v.begin() + start_idx, v.begin() + end_idx};
  }

  PairVectorIterators<std::string> iterators(int nthread, int n = -1);

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
  void clusterSizeStatistics(int nthread);
  void clusterDiameterStatistics(int nthread);
  void findLetterGroupSplit(int nthread);
  void keyboardWords(int nthread);
  void dictionaryStatistics(int nthread);
  void wordFrequency(int nthread);
  void checkDictionary(int nthread);
  void twoCharactersDistribution(int nthread);
  void findWordsSplit(int nthread);

  void checkTemplatePreProseeding();
  void findChainPreProseeding();
  void clusterStatisticsPreProseeding();
  void findLetterGroupSplitPreProseeding();
  void checkKeyboardWordSimplePreProseeding();
  void checkKeyboardWordComplexPreProseeding();
  void checkDictionaryPreProseeding();

  void anagramsPostProseeding();
  void findLetterGroupSplitPostProseeding();
  void simpleDoubleWordSequencePostProseeding();
  void dictionaryStatisticsPostProseeding();
  void checkDictionaryPostProseeding();
  void twoCharactersDistributionPostProseeding();

  void twoDictionaries(int nthread);

  int fillDictionaryLength(int nthread, size_t length,
                           const VString &bounds = {});
  int findLongestChain(int nthread);
  IntInt getClasterSizeDiameter(int nthread);

  int chainAddIfFoundIndex(const std::string &s, VectorInt &v,
                           const VectorInt &otherSideOrdered = {});
  VectorInt chainLayer(int nthread, const VectorInt &layerIndex);
  void chainLayer2(int nthread, const VectorInt &layerIndex,
                   const VectorInt &otherSideOrdered = {});
  void chainSetPtr();
  static bool differenceOnlyOneChar(std::string const &a, std::string const &b);

  Dictionary const &getDictionary() const {
    return m_dictionary[getDictionaryIndex()];
  }

  void sortFilterResults(ENUM_JOB_TYPE e);

  void loadLanguages();

  static std::string pairsToString(VectorStringString const &v, bool p = 0);

  void updateProceedingText(std::string s = "", bool add = true);
  VectorInt relations(int nthread, VectorInt const &v1, VectorInt const &v2);
};
extern WordsBase *wordsBase;
