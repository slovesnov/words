/*
 * WordsBase.h
 *
 *  Created on: 20.11.2017
 *      Author: alexey slovesnov
 */

#pragma once

#include "Modification.h"
#include "consts.h"
#include <barrier>
#include <functional>
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
  class TableFormat {
  private:
    std::vector<ENUM_STRING> m_en;
    std::vector<int> m_width;

  public:
    int m_rows;

  private:
    template <typename T> std::string formatCell(const T &arg, size_t i) const {
      using DecayedT = std::decay_t<T>;

      // 1. If it's an ENUM_STRING, convert it to std::string using wordsBase
      if constexpr (std::is_same_v<DecayedT, ENUM_STRING>) {
        std::string word_str = wordsBase->string(arg);
        return formatCell(word_str,
                          i); // Recursively format the converted string
      }
      // 2. Standard formatting logic for all other types
      else {
        // If it's the last (+1) argument, print WITHOUT alignment
        if (i == m_width.size()) {
          return std::format("{}", arg);
        }

        if constexpr (std::is_floating_point_v<DecayedT>) {
          // For double/float: right-align, width m_width[i], 2 decimal places
          return std::format("{:>{}.2f}", arg, m_width[i]);
        } else if constexpr (std::is_convertible_v<T, std::string_view>) {
          return std::format("{:>{}}", arg, m_width[i]);
        } else {
          return std::format("{:>{}}", WordsBase::intToString(arg), m_width[i]);
        }
      }
    }

    // Helper function performing pack expansion using compile-time indices
    template <typename Tuple, size_t... EnIndices, size_t... WidthIndices>
    std::string createWithTitleImpl(Tuple &&t,
                                    std::index_sequence<EnIndices...>,
                                    std::index_sequence<WidthIndices...>) {
      constexpr size_t N = sizeof...(EnIndices);
      // 1. Call init() by extracting the first N elements and the next N
      // elements from the tuple We add N to WidthIndices to shift into the
      // second half of the tuple
      init({static_cast<ENUM_STRING>(std::get<EnIndices>(t))...},
           {static_cast<int>(std::get<WidthIndices + N>(t))...});

      // 2. Call string() with the very last element (index 2N)
      return string(std::get<2 * N>(t));
    }

  public:
    TableFormat() {}
    TableFormat(std::initializer_list<ENUM_STRING> en,
                std::initializer_list<int> width) {
      init(en, width);
    }
    void init(std::initializer_list<ENUM_STRING> en,
              std::initializer_list<int> width) {
      m_rows = 0;
      m_en = en;
      m_width = width;
      assert(en.size() == width.size());
      int i = -1;
      for (auto a : en) {
        i++;
        m_width[i] = std::max(m_width[i], utf8Length(wordsBase->string(a)));
      }
    }

    template <typename... Args> std::string string(Args... args) {
      const size_t args_count = sizeof...(args);
      size_t i = 0;
      m_rows++;
      if constexpr (args_count == 1) {
        using FirstArgT =
            std::decay_t<std::tuple_element_t<0, std::tuple<Args...>>>;
        if constexpr (std::is_same_v<FirstArgT, ENUM_STRING>) {
          ENUM_STRING en = [args...](auto first, auto...) {
            return first;
          }(args...);

          std::string s;
          for (; i <= std::size(m_en); ++i) {
            s += (i ? " " : "");
            s += formatCell(i == std::size(m_en) ? en : m_en[i], i);
          }
#ifdef NOGTK
          return s;
#else
          return "<b>" + s + "</b>";
#endif
        }
      }
      assert(args_count == m_width.size() || args_count == m_width.size() + 1);

      std::string s = "\n";

      (
          [&](const auto &arg) {
            s += (i ? " " : "");
            s += formatCell(arg, i);
            i++;
          }(args),
          ...);
      return s;
    }

    // New function: accepts an odd number of arguments (2n + 1)
    template <typename... Args> std::string createWithTitle(Args... args) {
      // Compile-time check ensuring the number of arguments is odd and at least
      // 1
      static_assert(sizeof...(Args) % 2 != 0,
                    "Argument count must be odd (2n + 1)");

      constexpr size_t N = (sizeof...(Args) - 1) / 2;

      // Pack all arguments into a tuple for convenient slicing without copying
      auto args_tuple = std::forward_as_tuple(args...);

      // Call the private helper function, passing index sequences for packet
      // splitting
      return createWithTitleImpl(
          args_tuple, std::make_index_sequence<N>{}, // Indices from 0 to N-1
          std::make_index_sequence<N>{});            // Indices from N to 2N-1
    }
  };

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
  using BarrierCallback = std::move_only_function<void() noexcept>;
  std::unique_ptr<std::barrier<BarrierCallback>> m_barrier;
  std::vector<VectorBool> m_chain;
  std::vector<char> m_chainChar;
  MapIntInt m_wordLengthMap[LANGUAGES];
  VectorIntInt m_iiv;
  std::atomic<int> m_i;
  VVString m_vvs;
  SetString m_ss;
  VectorVectorInt m_layerIndex[2], m_vii;
  TableFormat m_table;
  int m_j;
  VectorChainLayerResult m_chainLayerResult;
  VectorInt *m_chainPtr[2];

  std::string getStatusString();
  std::string getTimeString();
  void mergeLayers() noexcept;
  void mergeLayers2() noexcept;

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

  void findChainPreProseeding();
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

  void fillDictionaryLength(int nthread, size_t length,
                            const VString &bounds = {});
  int findLongestChain(int nthread);

  bool chainAddIfFound(int nthread, const std::string &s, VectorInt &v,
                       const VectorInt &otherSideOrdered = {});
  int chainAddIfFoundIndex(const std::string &s, VectorInt &v,const VectorInt &otherSideOrdered = {});
  std::pair<VectorInt, bool> chainLayer(int nthread,
                                        const VectorInt &layerIndex,
                                        const VectorInt &otherSideOrdered = {});
  VectorInt chainLayer1(int nthread, const VectorInt &layerIndex);
  void chainLayer2(int nthread, const VectorInt &layerIndex,
                                        const VectorInt &otherSideOrdered = {});
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
  std::array<VectorInt, 2> relations2(int nthread, VectorInt const &v1,
                                      VectorInt const &v2);
  VectorInt relations(int nthread, VectorInt const &v1, VectorInt const &v2);
};
