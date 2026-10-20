/*
 * WordsBase.cpp
 *
 *  Created on: 20.11.2017
 *      Author: alexey slovesnov
 */

#include "WordsBase.h"
#include <cmath>
#include <execution>
#include <format>
#include <magic_enum.hpp>
#include <ranges>

#ifdef NOGTK
#define RETURN_ON_USER_BREAK
#define RETURN_ON_USER_BREAK_VALUE
#else
#define RETURN_ON_USER_BREAK                                                   \
  if (m_token.stop_requested()) {                                              \
    return;                                                                    \
  }
#define RETURN_ON_USER_BREAK_VALUE(v)                                          \
  if (m_token.stop_requested()) {                                              \
    return v;                                                                  \
  }
#endif

const LookupTable<ENUM_MENU, void (WordsBase::*)(int)> COMPLEX_FUNCTION = {
    {MENU_ANAGRAM, &WordsBase::findAnagram}, // break implemented
    {MENU_SIMPLE_WORD_SEQUENCE,
     &WordsBase::findSimpleWordSequence}, // break implemented
    {MENU_DOUBLE_WORD_SEQUENCE,
     &WordsBase::findDoubleWordSequence}, // break implemented
    {MENU_WORD_SEQUENCE_FULL,
     &WordsBase::findWordSequenceFull},                // not implemented
    {MENU_MODIFICATION, &WordsBase::findModification}, // break implemented
    {MENU_CHAIN, &WordsBase::findChain}, // if one word break implemented, if
                                         // two words not need
    {MENU_CLUSTER_STATISTICS,
     &WordsBase::clusterStatistics},                // break implemented
    {MENU_WORDS_SPLIT, &WordsBase::findWordsSplit}, // break implemented
    {MENU_LETTER_GROUP_SPLIT,
     &WordsBase::findLetterGroupSplit}, // break implemented
    {MENU_TWO_DICTIONARIES_STRICT,
     &WordsBase::twoDictionariesStrict}, // break implemented
    {MENU_TWO_DICTIONARIES_SIMPLE,
     &WordsBase::twoDictionariesSimple}, // break implemented
    {MENU_TWO_DICTIONARIES_TRANSLIT,
     &WordsBase::twoDictionariesTranslit}, // break implemented
    {MENU_TWO_DICTIONARIES_KEYBOARD_WORD,
     &WordsBase::keyboardWords}, // not implemented
    {MENU_DICTIONARY_STATISTICS,
     &WordsBase::dictionaryStatistics}, // not implemented
    {MENU_WORD_FREQUENCY,
     &WordsBase::wordFrequency}, // not implemented (too fast)
    {MENU_CHECK_DICTIONARY, &WordsBase::checkDictionary}, // not implemented
    {MENU_TWO_CHARACTERS_DISTRIBUTION,
     &WordsBase::twoCharactersDistribution}, // not implemented
    {MENU_TWO_CHARACTERS_DISTRIBUTION_START,
     &WordsBase::twoCharactersDistribution}, // not implemented
    {MENU_TWO_CHARACTERS_DISTRIBUTION_END,
     &WordsBase::twoCharactersDistribution} // not implemented
};

const LookupTable<ENUM_MENU, void (WordsBase::*)()> menuPreProseeding = {
    {MENU_CLUSTER_STATISTICS, &WordsBase::clusterStatisticsPreProseeding},
    {MENU_LETTER_GROUP_SPLIT, &WordsBase::findLetterGroupSplitPreProseeding},
    {MENU_KEYBOARD_WORD_SIMPLE,
     &WordsBase::checkKeyboardWordSimplePreProseeding},
    {MENU_KEYBOARD_WORD_COMPLEX,
     &WordsBase::checkKeyboardWordComplexPreProseeding},
    {MENU_CHECK_DICTIONARY, &WordsBase::checkDictionaryPreProseeding}};

const LookupTable<ENUM_MENU, void (WordsBase::*)()> menuPostProseeding = {
    {MENU_ANAGRAM, &WordsBase::anagramsPostProseeding},
    {MENU_CLUSTER_STATISTICS, &WordsBase::clusterStatisticsPostProseeding},
    {MENU_LETTER_GROUP_SPLIT, &WordsBase::findLetterGroupSplitPostProseeding},
    {MENU_SIMPLE_WORD_SEQUENCE,
     &WordsBase::simpleDoubleWordSequencePostProseeding},
    {MENU_DOUBLE_WORD_SEQUENCE,
     &WordsBase::simpleDoubleWordSequencePostProseeding},
    {MENU_DICTIONARY_STATISTICS,
     &WordsBase::dictionaryStatisticsPostProseeding},
    {MENU_CHECK_DICTIONARY, &WordsBase::checkDictionaryPostProseeding},
    {MENU_TWO_CHARACTERS_DISTRIBUTION,
     &WordsBase::twoCharactersDistributionPostProseeding},
    {MENU_TWO_CHARACTERS_DISTRIBUTION_START,
     &WordsBase::twoCharactersDistributionPostProseeding},
    {MENU_TWO_CHARACTERS_DISTRIBUTION_END,
     &WordsBase::twoCharactersDistributionPostProseeding}};

// regular expression special proceeding
const LookupTable<ENUM_MENU, bool (WordsBase::*)(const std::string &)>
    SIMPLE_FUNCTION = {
        {MENU_PANGRAM, &WordsBase::checkPangram},
        {MENU_TEMPLATE, &WordsBase::checkTemplate},
        {MENU_PALINDROME, &WordsBase::checkPalindrome},
        {MENU_CROSSWORD, &WordsBase::checkCrossword},
        {MENU_CHARACTER_SEQUENCE, &WordsBase::checkCharacterSequence},
        {MENU_KEYBOARD_WORD_SIMPLE, &WordsBase::checkKeyboardWordSimple},
        {MENU_KEYBOARD_WORD_COMPLEX, &WordsBase::checkKeyboardWordComplex},
        {MENU_CONSONANT_VOWEL_SEQUENCE,
         &WordsBase::checkConsonantVowelSequence},
        {MENU_DENSITY, &WordsBase::checkDensity}};

#ifdef NOGTK
// use cgi project
#include "cgi.h"
#endif

// #define ONE_THREADgf

WordsBase *wordsBase;

WordsBase::WordsBase() {
  int i, j;
  wordsBase = this;
  // clock_t begin = clock();

  int k;
  for (i = 0; i < LANGUAGES; i++) {
    k = 0;
    // load dictionaries
    m_longestWordLength[i] = 0;
    std::ifstream file(path(i, "words"));
    assert(file.is_open());
    std::string line, s;
    if (i == DICTIONARY_RU)
      m_dictionary[DICTIONARY_RU_UTF8].resize(NUMBER_OF_WORDS[i]);
    m_dictionary[i].resize(NUMBER_OF_WORDS[i]);
    while (std::getline(file, line)) {
      j = line.size();
      if (j > m_longestWordLength[i]) {
        m_longestWordLength[i] = j;
      }
      m_wordLengthMap[i][j]++;
      assert(k != NUMBER_OF_WORDS[i]);
      if (i == DICTIONARY_RU)
        m_dictionary[DICTIONARY_RU_UTF8][k] = fastLocaleToUtf8(line);
      m_dictionary[i][k] = std::move(line);
      k++;
    }
    file.close();
    assert(k == NUMBER_OF_WORDS[i]);
  }

#ifdef ONE_THREAD
  m_threads = 1;
#else
  m_threads = g_get_num_processors(); // std::hardware_concurrency();
#endif
  m_tr.resize(m_threads);
  m_ma.resize(m_threads);
  m_vs.resize(m_threads);
  m_thread_result.resize(m_threads);
  m_anagrams.resize(m_threads);
  m_eqmapt.resize(m_threads);
  m_si.resize(m_threads);
  m_chain.resize(m_threads);
  m_vvs.resize(m_threads);

  loadLanguages();

  for (n = 0; n < 2; n++) {
    j = -1;
    for (auto &s : m_languageAll[n]) {
      j++;
      if (utf8Length(s) > 1000) {
        pr(n, j, s);
        printlog(n, j, s);
      }
    }
  }

  // showLongestAnagram();
  // showLongestPangram();
  // showLongestSimpleWordSequence();
  // showLongestDoubleWordSequence();

  // clearlog();

  // test();
#ifdef NOGTK
  // cgi();TODO uncomment on real cgi query

  // setDictionaryIndex(1);

  // count longest constants needs when dictionary changed
  // system("chcp 1251>nul");
#endif
}

// void WordsBase::test() {}

bool WordsBase::checkKeyboardWordSimple(const std::string &s) {
  std::string *ps = m_keyboardOneRow[uchar(s[0])];
  return spanIncluding(s, ps[0]) || spanIncluding(s, ps[1]);
}

bool WordsBase::checkKeyboardWordComplex(const std::string &s) {
  const char *p = s.c_str();
  for (; p[1] != 0; p++) {
    if (!m_keyboardRowDiagonals[uchar(*p)].contains(p[1])) {
      return false;
    }
  }
  return true;
}

bool WordsBase::checkPangram(const std::string &s) {
  int i = differentChars(s);
  return i >= m_comboValue[COMBOBOX_HELPER0] &&
         i <= m_comboValue[COMBOBOX_HELPER1];
}

bool WordsBase::checkTemplate(const std::string &s) {
  const char *p = s.c_str();
  int param = m_comboValue[COMBOBOX_HELPER0];
  if (param == 0) {
    if (s.length() < m_ev.length()) {
      return false;
    }

    char a[256] = {0};
    for (; *p != 0; p++) {
      a[uchar(*p)]++;
    }
    int i;
    for (i = 0; i < 256; i++) {
      if (a[i] < m_templateHelper[i]) {
        return false;
      }
    }
    return true;
  } else if (param == 1) {
    if (s.length() > m_ev.length()) {
      return false;
    }
    char a[256] = {0};
    uchar u;
    for (; *p != 0; p++) {
      u = *p;
      if (++a[u] > m_templateHelper[u]) {
        return false;
      }
    }
    return true;
  } else if (param == 2) {
    size_t i, j, k;
    for (i = j = 0; i < s.length(); i++) {
      k = m_template_a[j][uchar(s[i])];
      if (!k || ((j += k) >= m_ev.length() && i + 1 < s.length())) {
        return false;
      }
    }
    return true;
  } else {
    return spanIncluding(s, m_ev);
  }
}

bool WordsBase::checkCharacterSequence(const std::string &s) {
  auto j = m_comboValue[COMBOBOX_HELPER2];
  if (j == 0) {
    auto min = m_comboValue[COMBOBOX_HELPER0];
    auto max = m_comboValue[COMBOBOX_HELPER1];
    j = 0;
    std::string::size_type pos = 0;
    while ((pos = s.find(m_ev, pos)) != std::string::npos) {
      if (!m_radioValue) {
        return true;
      }
      j++;
      if (j > max) {
        return false;
      }
      pos += m_ev.length();
    }
    return j >= min;
  } else if (j == 1) {
    return s.compare(0, m_ev.length(), m_ev) == 0;
  } else {
    unsigned i = s.length();
    if (i < m_ev.length()) {
      return false;
    }
    return s.compare(i - m_ev.length(), m_ev.length(), m_ev) == 0;
  }
}

bool WordsBase::checkCrossword(const std::string &s) {
  unsigned i;
  if (s.length() != m_ev.length()) {
    return false;
  }
  for (i = 0; i < m_ev.length(); ++i) {
    if (m_ev[i] != '*' && m_ev[i] != s[i]) {
      return false;
    }
  }
  return true;
}

bool WordsBase::checkConsonantVowelSequence(const std::string &s) {
  const int searchType = m_comboValue[COMBOBOX_HELPER0];
  const int n = m_comboValue[COMBOBOX_HELPER1];
  const char *q = vowelConsonant(m_comboValue[COMBOBOX_HELPER2]).c_str();
  int i, j;
  const int l = s.length();
  if (l < n) {
    return false;
  }
  if (searchType == 0) {
    for (i = 0; i < l - n + 1; i++) {
      for (j = 0; j < n; j++) {
        if (strchr(q, s[i + j]) == NULL) {
          break;
        }
      }
      if (j == n) {
        return true;
      }
    }
    return false;
  } else if (searchType == 1) {
    for (i = 0; i < n; i++) {
      if (strchr(q, s[i]) == NULL) {
        return false;
      }
    }
    return true;
  } else {
    for (i = l - n; i < l; i++) {
      if (strchr(q, s[i]) == NULL) {
        return false;
      }
    }
    return true;
  }
}

bool WordsBase::checkDensity(const std::string &s) {
  int i, p;
  const char *q = vowelConsonant(m_comboValue[COMBOBOX_HELPER1]).c_str();
  const int percentage = m_comboValue[COMBOBOX_HELPER0];
  for (p = i = 0; i < int(s.length()); i++) {
    if (strchr(q, s[i]) != NULL) {
      p++;
    }
  }
  return p * 100 <= percentage * int(s.length());
}

bool WordsBase::checkPalindrome(const std::string &s) {
  const char *p = s.c_str();
  const char *p1 = p + s.length() - 1;
  for (; p < p1; p++, p1--) {
    if (*p != *p1) {
      return false;
    }
  }
  return true;
}

bool WordsBase::checkRegularExpression(const std::string &s, pcre2_code *re,
                                       pcre2_match_data *match_data) {
  if (!m_radioValue) {
    int rc = pcre2_match(re, (PCRE2_SPTR)s.c_str(), s.length(), 0, 0,
                         match_data, NULL);
    return rc >= 0;
  }

  int i = 0;
  size_t start_pos = 0;
  const int max = m_comboValue[COMBOBOX_HELPER1];
  const size_t string_len = s.length();

  while (start_pos <= string_len && i <= max) {
    int rc = pcre2_match(re, (PCRE2_SPTR)s.c_str(), string_len, start_pos, 0,
                         match_data, NULL);

    if (rc < 0) {
      break;
    }

    i++;

    PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
    size_t start_match = ovector[0];
    size_t end_match = ovector[1];

    start_pos = end_match;

    if (start_match == end_match) {
      start_pos++;
    }
  }

  return i >= m_comboValue[COMBOBOX_HELPER0] && i <= max;
}

void WordsBase::checkKeyboardWordSimplePreProseeding() {
  int i, k;
  size_t j;
  uchar uc;
  for (i = 0; i < KEYBOARD_ROWS; i++) {
    auto &rs = settings(SETTINGS_KEYBOARD_ROW1, i);
    for (j = 0; j < rs.length(); j++) {
      uc = rs[j];
      m_keyboardOneRow[uc][0] = rs;
      for (k = 0; k < KEYBOARD_ROWS; k++) {
        if (settings(SETTINGS_KEYBOARD_ROW1, k).length() > j) {
          m_keyboardOneRow[uc][1] += settings(SETTINGS_KEYBOARD_ROW1, k)[j];
        }
      }
    }
  }
}

void WordsBase::checkKeyboardWordComplexPreProseeding() {
  int i, k, l;
  size_t j;
  uchar uc;
  char c;
  const int ROW_LEN = settings(SETTINGS_KEYBOARD_ROW1).length() + 1;
  const int SZ = ROW_LEN * 5;
  std::vector<char> pu(SZ, 0);
  for (i = 0; i < KEYBOARD_ROWS; i++) {
    auto &rs = settings(SETTINGS_KEYBOARD_ROW1, i);
    for (j = 0; j < rs.length(); j++) {
      pu[(i + 1) * ROW_LEN + (j + 1)] = rs[j];
    }
  }

  for (i = 0; i < KEYBOARD_ROWS; i++) {
    auto &rs = settings(SETTINGS_KEYBOARD_ROW1, i);
    for (j = 0; j < rs.length(); j++) {
      uc = rs[j];
      for (k = -1; k < 2; k++) {
        for (l = -1; l < 2; l++) {
          c = pu[((i + 1) + k) * ROW_LEN + (j + 1 + l)];
          if (c) {
            m_keyboardRowDiagonals[uc] += c;
          }
        }
      }
    }
  }
}

void WordsBase::checkDictionaryPreProseeding() {
  std::string filename = path(getDictionaryIndex(), "words");
  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    assert(0);
    return;
  }
  file.seekg(0, std::ios::end);
  int filesize = file.tellg();
  int i, start;
  char c;
  m_chd.clear();
  for (i = 0; i < m_threads; ++i) {
    start = i * (filesize / m_threads);
    // adjust start
    // last char in chunk '\n'
    if (i) {
      file.seekg(start, std::ios::beg);
      while (file.get(c) && c != '\n') {
        start++;
      }
      start++;
    }
    m_chd.push_back(start);
  }
  m_chd.push_back(filesize);
  file.close();
}

void WordsBase::checkDictionaryPostProseeding() {
  SearchResult::out = ""; // cann't use joinV
  for (auto &a : m_vs) {
    if (!a.empty()) {
      if (!SearchResult::out.empty()) {
        SearchResult::out += "\n";
      }
      SearchResult::out += a;
    }
  }
  if (SearchResult::out.empty()) {
    SearchResult::out = string(DICTIONARY_CHECK_FINISHED_SUCCESSFULLY);
  }
}

// if bounds not found in dictionary then they are added to dictionary
void WordsBase::fillDictionaryLength(int nthread, size_t length,
                                     const VString &bounds) {
  auto &words = m_vvs[nthread];
  words.clear();

  const auto &dict = getDictionary();
  words.reserve(dict.size() / 10 + bounds.size()); // Pre-allocate memory once

  size_t bounds_idx = 0;
  size_t bounds_size = bounds.size();
  size_t added_count =
      0; // Tracks the number of bounds words missing from the dictionary

  // 1. Fast inline sorting for bounds (maximum of 2 elements)
  const VString *sorted_bounds = &bounds;
  VString local_bounds;
  if (bounds_size == 2 && bounds[1] < bounds[0]) {
    local_bounds = {bounds[1], bounds[0]};
    sorted_bounds = &local_bounds;
  }

  // 2. Single-pass linear merge O(N)
  for (const auto &a : dict) {
    if (a.size() != length || m_ss.contains(a)) {
      continue;
    }

    // Insert bounds elements that come alphabetically before the current
    // dictionary word
    while (bounds_idx < bounds_size && (*sorted_bounds)[bounds_idx] < a) {
      words.push_back((*sorted_bounds)[bounds_idx]);
      ++bounds_idx;
      ++added_count; // The word was missing from the dictionary; increment
                     // count
    }

    // If the bounds element matches the dictionary word, skip it to prevent
    // duplicates
    if (bounds_idx < bounds_size && (*sorted_bounds)[bounds_idx] == a) {
      ++bounds_idx;
      // Do not increment added_count since the word is already present in the
      // dictionary
    }

    words.push_back(a);
  }

  // 3. Append remaining bounds elements that come alphabetically after all
  // dictionary words
  while (bounds_idx < bounds_size) {
    words.push_back((*sorted_bounds)[bounds_idx]);
    ++bounds_idx;
    ++added_count; // The word was missing from the dictionary; increment count
  }

  // 4. Update the execution chain layout
  m_chain[nthread].assign(words.size() + added_count, true);
}

class TableFormat {
private:
  std::vector<ENUM_STRING> m_en;
  std::vector<int> m_width;

  template <typename T> std::string formatCell(const T &arg, size_t i) const {
    using DecayedT = std::decay_t<T>;

    // 1. If it's an ENUM_STRING, convert it to std::string using wordsBase
    if constexpr (std::is_same_v<DecayedT, ENUM_STRING>) {
      std::string word_str = wordsBase->string(arg);
      return formatCell(word_str, i); // Recursively format the converted string
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
        return std::format("{:>{}}", toString(arg, ','), m_width[i]);
      }
    }
  }

public:
  TableFormat(std::initializer_list<ENUM_STRING> en,
              std::initializer_list<int> width)
      : m_en(en), m_width(width) {
    assert(en.size() == width.size());
    int i = -1;
    for (auto a : en) {
      i++;
      m_width[i] = std::max(m_width[i], utf8Length(wordsBase->string(a)));
    }
  }

  template <typename... Args> std::string string(Args... args) const {
    const size_t args_count = sizeof...(args);
    size_t i = 0;

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
};

int WordsBase::findLongestChain(int nthread, int wordIndex) {
  auto &words = m_vvs[nthread];
  int i, j, k, total, prev;
  std::string s, words_str;
  VectorInt layerIndex = {wordIndex};
  VString v;
  int size = words.size();
  int w = std::to_string(size).length();
  w += (w - 1) / 3; // separators
  const int MAX_WORDS = 5;
  const int BF_WIDTH = 6;

  TableFormat table({NSTRING, LAYER, BRANCHING_FACTOR}, {2, w, BF_WIDTH});
  if (m_menuClick == MENU_CHAIN) {
    s = table.string(WORDS_NOMINATIVE_CASE) +
        table.string(1, layerIndex.size(), "", localeToUtf8(words[wordIndex]));
    updateProceedingText(s, false);
  }

  total = 0;
  prev = 1;
  for (i = 2;; i++) {
    layerIndex = chainLayer(nthread, layerIndex).first;
    j = layerIndex.size();
    if (!j) {
      break;
    }
    total += j;
    if (m_menuClick == MENU_CHAIN) {
      words_str = "";
      for (k = 0; k < std::min(MAX_WORDS, j); k++) {
        words_str += (k ? " " : "") + localeToUtf8(words[layerIndex[k]]);
      }
      if (j > MAX_WORDS) {
        words_str += "…";
      }
      s = table.string(i, layerIndex.size(), double(j) / prev, words_str);
      updateProceedingText(s);
      prev = j;
    }
    RETURN_ON_USER_BREAK_VALUE(-1)
  }
  return total;
}

bool WordsBase::chainAddIfFound(int nthread, const std::string &s, VectorInt &v,
                                const VectorInt &otherSideOrdered) {
  int i = indexOfOrdered(s, m_vvs[nthread]);
  if (i != -1) {
    if (m_chain[nthread][i]) {
      m_chain[nthread][i] = false;
      v.push_back(i);
      return false;
    } else if (std::ranges::binary_search(otherSideOrdered, i)) { // todo
      return true;
    }
  }
  return false;
}

std::pair<VectorInt, bool>
WordsBase::chainLayer(int nthread, const VectorInt &layerIndex,
                      const VectorInt &otherSideOrdered) {
  VectorInt total_v;
  const auto &words = m_vvs[nthread];

  for (const auto &li : layerIndex) {
    VectorInt v;
    std::string s = words[li];

    for (size_t i = 0; i < s.size(); ++i) {
      char original_char = s[i];
      for (const char &c : alphabet()) {
        if (c != original_char) {
          s[i] = c;
          if (chainAddIfFound(nthread, s, v, otherSideOrdered)) {
            return {{}, true};
          }
        }
      }
      s[i] = original_char;
    }

    total_v.insert(total_v.end(), v.begin(), v.end());
  }

  return {total_v, false};
}

int WordsBase::differentChars(std::string_view s) {
  bool seen[256] = {false};
  int c = 0;
  for (const char ch : s) {
    const uchar b = uchar(ch);
    if (!seen[b]) {
      seen[b] = true;
      c++;
    }
  }
  return c;
}

std::string readBinaryFileToString(const std::string &filename) {
  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  std::string s;
  if (!file.is_open()) {
#ifndef NDEBUG
    pr("error cann't open file " + filename);
#endif
  } else {
    s.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(s.data(), s.size());
  }
  return s;
}

#ifdef NOGTK
void WordsBase::setDictionaryIndex(int i) {
  assert(i >= 0 && i < LANGUAGES);
  m_buttonValue[BUTTON_DICTIONARY] = i;
}
void outMax(std::string f, std::array<int, 2> r) {
  std::string s;
  for (char c : f.substr(strlen("showLongest"))) {
    if (std::isupper(c))
      s += '_';
    s += std::toupper(c);
  }
  std::cout << std::format(
      "const std::array<int,LANGUAGES> MAX{}_LENGTH={{{},{}}}\n", s, r[0],
      r[1]);
}

// like simpleDoubleWordSequencePostProseeding()
bool outMap(const MapStringTwoStringVectors &map, size_t len) {
  int i, j;
  std::string s;
  for (auto &[_, v] : map) {
    auto &v0 = v[0];
    auto &v1 = v[1];
    i = v0.size();
    j = v1.size();
    if (i != 0 && j != 0) {
      if (i == 1 && j == 1 && v0[0] == v1[0] && v0[0].length() == len) {
        continue;
      }
      s = joinV(v0) + " - " + joinV(v1);
      std::cout << std::format("{} {}\n", s, len);
      return true;
    }
  }
  return false;
}

void WordsBase::showLongestAnagram() {
  int i, n;
  std::array<int, 2> r;
  std::string s, s1;
  MapStringStringVector map;
  MapStringStringVectorI cit;
  VVString vvs;

  for (n = 0; n < 2; n++) {
    setDictionaryIndex(n);
    map.clear();
    vvs.clear();
    vvs.resize(m_longestWordLength[n]);
    for (const auto &e : getDictionary()) {
      vvs[m_longestWordLength[n] - e.length()].push_back(e);
    }
    i = m_longestWordLength[n];
    for (auto &v : vvs) {
      for (auto const &e : v) {
        s = e;
        std::sort(s.begin(), s.end());
        cit = map.find(s);
        if (cit == map.end()) {
          VString v;
          v.push_back(e);
          map[s] = v;
        } else {
          cit->second.push_back(e);
        }

        for (auto &[_, v] : map) {
          if (v.size() < 2) {
            continue;
          }
          s = joinV(v);
          r[n] = i;
          // s1 = std::format("length{} {}\n", i, s);
          // std::cout << s1;
          // printlog(s1);
          goto l425;
        }
      }
      i--;
    }

  l425:
  }
  outMax(__func__, r);
}

void WordsBase::showLongestPangram() {
  int i, n, l;
  std::array<int, 2> r;
  std::string s;
  for (n = 0; n < 2; n++) {
    i = 10;
    setDictionaryIndex(n);
    for (auto &e : getDictionary()) {
      if (int(e.length()) > i) {
        l = differentChars(e);
        if (l > i) {
          i = l;
          r[n] = i;
          s = std::format("pangram diff chars{} {}\n", i, e);
        }
      }
    }
    std::cout << s;
    printlog(s);
  }
  outMax(__func__, r);
}

void WordsBase::showLongestSimpleWordSequence() {
  int i, j, n;
  std::array<int, 2> r;
  std::string s, s1;
  MapStringTwoStringVectors map;
  for (n = 0; n < 2; n++) {
    setDictionaryIndex(n);
    for (i = m_longestWordLength[n]; i > 0; i--) {
      for (auto const &e : getDictionary()) {
        if (int(e.length()) >= i) {
          for (j = 0; j < 2; j++) {
            s = j ? e.substr(0, i) : e.substr(e.length() - i);
            auto mit = map.find(s);
            if (mit == map.end()) {
              TwoStringVectors v;
              v[j].push_back(e);
              map[s] = v;
            } else {
              mit->second[j].push_back(e);
            }
          }
        }
      }
      if (outMap(map, i)) {
        r[n] = i;
        map.clear();
        break;
      }
      map.clear();
    }
  }
  outMax(__func__, r);
}

void WordsBase::showLongestDoubleWordSequence() {
  int i, j, n;
  std::array<int, 2> r;
  std::string s, t, q;
  MapStringTwoStringVectors map;
  for (n = 0; n < 2; n++) {
    setDictionaryIndex(n);
    for (i = m_longestWordLength[n]; i > 0; i--) {
      for (auto const &e : getDictionary()) {
        if (int(e.length()) >= i) {
          s = e.substr(0, i);
          q = e.substr(e.length() - i);
          if (s == q) {
            j = 2;
            t = s + " " + q;
          } else if (s < q) {
            j = 0;
            t = s + " " + q;
          } else {
            j = 1;
            t = q + " " + s;
          }
          auto mit = map.find(t);
          if (mit == map.end()) {
            TwoStringVectors v;
            if (j == 2) {
              v[0].push_back(e);
              v[1].push_back(e);
            } else {
              v[j].push_back(e);
            }
            map[t] = v;
          } else {
            if (j == 2) {
              mit->second[0].push_back(e);
              mit->second[1].push_back(e);
            } else {
              mit->second[j].push_back(e);
            }
          }
        }
      }

      if (outMap(map, i)) {
        r[n] = i;
        map.clear();
        break;
      }
      map.clear();
    }
  }
  outMax(__func__, r);
}
#endif

void WordsBase::findAnagram(int nthread) {
  const int min = m_comboValue[COMBOBOX_HELPER0];
  const int max = m_comboValue[COMBOBOX_HELPER1];
  auto [it, end] = iterators(nthread);
  auto &local_map = m_anagrams[nthread];
  local_map.clear();
  for (; it != end; it++) {
    if (int(it->length()) < min || int(it->length()) > max) {
      continue;
    }
    std::string signature = *it;
    std::sort(signature.begin(), signature.end());
    local_map[signature].push_back(*it);
    RETURN_ON_USER_BREAK
  }
}

void WordsBase::anagramsPostProseeding() {
  MapAnagram final_anagrams;
  for (auto &local_map : m_anagrams) {
    for (auto &[signature, words] : local_map) {
      auto &final_words = final_anagrams[signature];
      final_words.insert(final_words.end(),
                         std::make_move_iterator(words.begin()),
                         std::make_move_iterator(words.end()));
    }
    local_map.clear();
  }

  for (auto &[signature, words] : final_anagrams) {
    if (words.size() > 1) {
      std::string s = joinV(words);
      m_result.push_back(
          SearchResult(s, words.begin()->length(), words.size()));
    }
  }
}

void WordsBase::findSimpleWordSequence(int nthread) {
  const int min = m_comboValue[COMBOBOX_HELPER0];
  const int max = m_comboValue[COMBOBOX_HELPER1];
  int i = min - 1, j;
  std::string s;
  auto &v = m_ma[nthread];
  v.resize(max - min + 1);
  for (auto &map : v) {
    i++;
    map.clear();
    auto [it, end] = iterators(nthread);
    for (; it != end; it++) {
      auto const &e = *it;
      if (int(e.length()) >= i) {
        for (j = 0; j < 2; j++) {
          s = j ? e.substr(0, i) : e.substr(e.length() - i);
          auto mit = map.find(s);
          if (mit == map.end()) {
            TwoStringVectors v;
            v[j].push_back(e);
            map[s] = v;
          } else {
            mit->second[j].push_back(e);
          }
        }
      }
      RETURN_ON_USER_BREAK
    }
  }
}

void WordsBase::findDoubleWordSequence(int nthread) {
  const int min = m_comboValue[COMBOBOX_HELPER0];
  const int max = m_comboValue[COMBOBOX_HELPER1];
  int i = min - 1, j;
  std::string q, s, t;
  auto &v = m_ma[nthread];
  v.resize(max - min + 1);
  for (auto &map : v) {
    i++;
    map.clear();
    auto [it, end] = iterators(nthread);
    for (; it != end; it++) {
      auto const &e = *it;
      if (int(e.length()) >= i) {
        t = e.substr(0, i);
        q = e.substr(e.length() - i);
        if (t == q) {
          j = 2;
          s = t + " " + q;
        } else if (t < q) {
          j = 0;
          s = t + " " + q;
        } else {
          j = 1;
          s = q + " " + t;
        }
        auto mit = map.find(s);
        if (mit == map.end()) {
          TwoStringVectors v;
          if (j == 2) {
            v[0].push_back(e);
            v[1].push_back(e);
          } else {
            v[j].push_back(e);
          }
          map[s] = v;
        } else {
          if (j == 2) {
            mit->second[0].push_back(e);
            mit->second[1].push_back(e);
          } else {
            mit->second[j].push_back(e);
          }
        }
      }
      RETURN_ON_USER_BREAK
    }
  }
}

void WordsBase::simpleDoubleWordSequencePostProseeding() {
  const int min = m_comboValue[COMBOBOX_HELPER0];
  const int max = m_comboValue[COMBOBOX_HELPER1];
  int n;
  for (n = 0; n <= max - min; n++) {
    auto &map = m_ma[0][n];
    for (size_t i = 1; i < m_ma.size(); i++) {
      for (auto &[e, a] : m_ma[i][n]) {
        auto it = map.find(e);
        if (it == map.end()) {
          map[e] = std::move(a);
        } else {
          auto &dest_array = it->second;

          auto &dest_v0 = dest_array[0];
          auto &src_v0 = a[0];
          dest_v0.reserve(dest_v0.size() + src_v0.size());
          dest_v0.insert(dest_v0.end(), std::make_move_iterator(src_v0.begin()),
                         std::make_move_iterator(src_v0.end()));

          auto &dest_v1 = dest_array[1];
          auto &src_v1 = a[1];
          dest_v1.reserve(dest_v1.size() + src_v1.size());
          dest_v1.insert(dest_v1.end(), std::make_move_iterator(src_v1.begin()),
                         std::make_move_iterator(src_v1.end()));
        }
      }
    }

    const size_t len = m_comboValue[COMBOBOX_HELPER0];
    m_result.reserve(m_result.size() + map.size() / 2);
    for (auto &[_, v] : map) {
      const auto &v0 = v[0];
      const auto &v1 = v[1];

      const size_t size_v0 = v0.size();
      const size_t size_v1 = v1.size();

      if (size_v0 != 0 && size_v1 != 0) {
        if (size_v0 == 1 && size_v1 == 1) {
          if (v0[0] == v1[0] && v0[0].length() == len) {
            continue;
          }
        }

        size_t word_len = v0[0].length();
        std::string s = joinV(v0) + " - " + joinV(v1); //" - "
        m_result.emplace_back(std::move(s), word_len, size_v0 + size_v1);
      }
    }
  }
}

void WordsBase::findWordSequenceFull(int nthread) {
  std::string s;
  auto [it, end] = iterators(nthread);
  for (; it != end; it++) {
    auto const &e = *it;
    if (checkPalindrome(e)) {
      continue;
    }
    s = e;
    std::reverse(s.begin(), s.end());
    if (s > e && std::ranges::binary_search(getDictionary(), s)) {
      m_thread_result[nthread].push_back(
          SearchResult(e + " " + s, e.length(), 2));
    }
  }
}

void WordsBase::findModification(int nthread) {
  std::string s;
  auto [it, end] = iterators(nthread);
  for (; it != end; it++) {
    auto const &e = *it;
    s = m_modifications.apply(e, m_checkValue);
    if (!s.empty() && s != e &&
        std::ranges::binary_search(getDictionary(), s)) {
      m_thread_result[nthread].push_back(
          SearchResult(e + " " + s, e.length(), 1));
    }
    RETURN_ON_USER_BREAK
  }
}

std::array<VectorInt, 2> WordsBase::relations(int nthread, VectorInt const &v1,
                                              VectorInt const &v2) {
  auto &words = m_vvs[nthread];
  VectorBool c[] = {VectorBool(v1.size(), false), VectorBool(v2.size(), false)};
  int i, j;
  i = -1;
  for (auto &a : v1) {
    i++;
    j = -1;
    for (auto &b : v2) {
      j++;
      if (differenceOnlyOneChar(words[a], words[b])) {
        c[0][i] = true;
        c[1][j] = true;
      }
    }
  }

  std::array<VectorInt, 2> r;
  for (i = 0; i < 2; i++) {
    auto &v = r[i];
    j = -1;
    for (auto const &a : c[i]) {
      j++;
      if (a) {
        v.push_back(i == 0 ? v1[j] : v2[j]);
      }
    }
  }
  return r;
}

void WordsBase::findChain(int nthread) {
  int i, j;
  VectorVectorInt layerIndex[2];
  VectorInt *ptr[2];
  auto &words = m_vvs[nthread];
  std::string s, s1;

  fillDictionaryLength(nthread, m_evwords[0].size(), m_evwords);
  i = -1;
  for (auto &a : m_evwords) {
    i++;
    layerIndex[i].push_back({});
    chainAddIfFound(nthread, a, layerIndex[i][0]);
  }

  // auto begin = clock();
  if (m_evwords.size() == 1) {
    findLongestChain(nthread, layerIndex[0][0][0]);
    return;
  }

  if (differenceOnlyOneChar(m_evwords[0], m_evwords[1])) {
    SearchResult::out = localeToUtf8(m_evwords[0] + " " + m_evwords[1]) +
                        OPEN_S + string(WORDS_GENITIVE_CASE) + "2)";
    return;
  }

  while (true) {
    i = -1;
    for (auto &a : ptr) {
      i++;
      a = &layerIndex[i][layerIndex[i].size() - 1];
    }
    j = ptr[1]->size() < ptr[0]->size();

    auto p = chainLayer(nthread, *ptr[j], *ptr[!j]);
    if (p.second) {
      break;
    }
    if (p.first.empty()) {
      SearchResult::out = string(NO_CHAINS_FOUND);
      return;
    }
    // std::sort(p.first.begin(), p.first.end()); // todo
    layerIndex[j].push_back(p.first);
  }

  i = -1;
  for (auto &a : ptr) {
    i++;
    a = &layerIndex[i][layerIndex[i].size() - 1];
  }
  auto res = relations(nthread, *ptr[0], *ptr[1]);

  for (i = 0; i < 2; i++) {
    auto &li = layerIndex[i];
    j = li.size() - 1;
    li[j] = res[i];
  }

  for (i = 0; i < 2; i++) {
    auto &li = layerIndex[i];
    for (j = li.size() - 1; j > 1; j--) {
      res = relations(nthread, li[j - 1], li[j]);
      li[j - 1] = res[0];
    }
  }
  layerIndex[0].insert(layerIndex[0].end(), layerIndex[1].rbegin(),
                       layerIndex[1].rend());
  TableFormat table({NSTRING}, {2});
  s = table.string(WORDS_NOMINATIVE_CASE);
  i = 0;
  for (auto &e : layerIndex[0]) {
    i++;
    s1 = "";
    j = -1;
    for (auto &b : e) {
      j++;
      s1 += (j ? " " : "") + localeToUtf8(words[b]);
    }
    s1 += " " + intToString(e.size());
    s += table.string(i, s1);
  }
  SearchResult::out = s;
  // for (auto &e : v) {
  //   m_result.push_back(SearchResult(e, m_evwords[0].length(), j + 2));
  // }
}

void WordsBase::clusterStatistics(int nthread) {
  int i, j, length;
  std::string s;
  while ((i = m_i.fetch_add(1)) < std::ssize(m_iiv)) {
    length = m_iiv[i].first;
    MapIntInt map;
    j = 0;
    fillDictionaryLength(nthread, length);
    while ((j = indexOf(true, m_chain[nthread], j)) != -1) {
      m_chain[nthread][j] = false;
      int i = findLongestChain(nthread, j);
      if (i == -1) { // user break
        return;
      }
      map[i + 1]++;
    }

    std::string s = "";
    for (const auto &[words, count] : map) {
      s += std::format(" {}:{}", intToString(words), intToString(count));
    }
    m_si.push_back({s, length});
    s = squareBrackets(length) + s + "\n";
    updateProceedingText(s);
  }
}

void WordsBase::clusterStatisticsPreProseeding() {
  m_ss.clear(); // used in fillDictionaryLength() so clear
  m_iiv = {getWordLengthMap().begin(), getWordLengthMap().end()};
  m_si.clear();
  std::sort(m_iiv.begin(), m_iiv.end(),
            [](auto &a, auto &b) { return a.second > b.second; });
  m_i.store(0);
}

void WordsBase::clusterStatisticsPostProseeding() {
  std::sort(m_si.begin(), m_si.end(),
            [](auto &a, auto &b) { return a.second < b.second; });
  int i = -1;
  std::string s;
  for (auto &p : m_si) {
    i++;
    s += (i ? "\n" : "") + squareBrackets(p.second) + p.first;
  }
  updateProceedingText(s, false);
}

void WordsBase::findLetterGroupSplitPreProseeding() {
  const size_t size = m_ev.length();
  m_eqmap.clear();
  m_eqmap.resize(size);
  m_latch = std::make_unique<std::latch>(m_threads);
}

void WordsBase::findLetterGroupSplit(int nthread) {
  std::string s, s1, t, lng;
  size_t i, j;
  int k, l;
  auto charset = getOrderedString(m_ev);

  const size_t size = m_ev.length();
  auto &eqm = m_eqmapt[nthread];
  eqm.clear();
  eqm.resize(size);

  auto [it, end] = iterators(nthread);
  for (; it != end; it++) {
    auto &s = *it;
    j = s.length();
    if (j < size) {
      s1 = getOrderedString(s);
      auto &m = eqm[j];
      auto it = m.find(s1);
      if (it == m.end()) {
        t = sub(charset, s1);
        if (t != INVALID_DIFFERENCE) {
          m.insert({s1, {t, {s}}});
        }
      } else {
        it->second.add(s);
      }

      if (m_token.stop_requested()) {
        m_latch->count_down();
        return;
      }
    }
  }

  {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (j = 1; j < size; j++) {
      auto &m = m_eqmap[j];
      for (auto &[k, v] : eqm[j]) {
        auto it = m.find(k);
        if (it == m.end()) {
          m.insert({k, v});
        } else {
          auto &v1 = it->second.anagrams;
          auto &v2 = v.anagrams;
          v1.reserve(v1.size() + v2.size());
          v1.insert(v1.end(), std::make_move_iterator(v2.begin()),
                    std::make_move_iterator(v2.end()));
        }
      }
    }
  }
  m_latch->arrive_and_wait();

  s = "";
  k = -1;
  l = 0;
  for (i = 1; i < size; i++) {
    auto &m = m_eqmap[i];
    for (auto &e : m) {
      if ((++k) % m_threads == nthread) {
        auto v = m_eqmap.allPairs(e.second.sub, e.first);
        if (!v.empty()) {
          l++;
          s += localeToUtf8(m_eqmap.get(e.first) + " " +
                            pairsToString(v, v.size() != 1));
        }
      }
      RETURN_ON_USER_BREAK
    }
  }
  m_si[nthread] = {s, l};
}

void WordsBase::findLetterGroupSplitPostProseeding() {
  int i;
  auto charset = getOrderedString(m_ev);
  auto v = m_eqmap.allPairs(charset);
  size_t n[] = {v.size(), 0};
  if (!v.empty()) {
    SearchResult::out = localeToUtf8(pairsToString(v)) + "----------------\n";
  }

  for (auto &a : m_si) {
    SearchResult::out += a.first;
    n[1] += a.second;
  }

  if (SearchResult::out.empty()) {
    SearchResult::out = string(SPLITS_NOT_FOUND);
  } else {
    for (i = 0; i < 2; i++) {
      m_addstatus += string(i ? TRIPLETS : PAIRS) + " " +
                     intToStringLocaled(n[i]) + (i ? "" : ", ");
    }
  }
}

void WordsBase::twoDictionaries(int nthread, int nn) {
  VVString to;
  int i, j, m, l, len, n, fromIndex = -1;
  std::string s, alphabetFrom;
  VString v, d;
  const int di = getDictionaryIndex();
  d = readFile(getResourcePath(LNG2TXT));
  for (auto &s : d) {
    if (s.size() == 2) {
      if (nn == 0) {
        fromIndex = indexOf(s, LNG);
        assert(fromIndex != -1);
        alphabetFrom = m_settingsAll[fromIndex][SETTINGS_ALPHABET];
        to.resize(alphabetFrom.length());
      } else if (nn < 0) {
        break;
      }
      nn--;
    } else if (nn == -1) {
      v = split(s, ' ');
      assert(v.size() > 1);
      i = 0;
      for (auto &a : v) {
        if (i) {
          to[j].push_back(a == "'" ? "" : a); //"'" = empty string
        } else {
          assert(a.size() == 1);
          j = indexOf(a[0], alphabetFrom);
          assert(j >= 0);
        }
        i++;
      }
    }
  }

  Dictionary const &dt = m_dictionary[!fromIndex];
  i = m_longestWordLength[fromIndex];
  VVString k(i);
  VectorInt id(i);

  auto [it, end] = iterators(nthread, fromIndex);
  for (; it != end; it++) {
    auto const &e = *it;
    len = e.length();
    for (n = 1, i = 0; i < len; i++) {
      j = indexOf(e[i], alphabetFrom);
      assert(j >= 0);
      // Check whether char has no correspondence in config file.
      // For example symbol '-' in russian alphabet, has no correspondence in
      // english alphabet
      k[i] = to[j];
      l = k[i].size();
      if (l == 0) {
        goto l1531;
        // faster then use break and check i<int(e.length()) after cycle
      }
      id[i] = l;
      n *= l;
    }

    for (j = 0; j < n; j++) {
      s.clear();
      l = j;
      for (i = 0; i < len; i++) {
        m = id[i];
        s += k[i][l % m];
        l /= m;
      }
      if (std::ranges::binary_search(dt, s)) {
        // first word should be in current dictionary language, for correct
        // sorting vowels/consonant percent
        if (di == fromIndex) {
          s = e + " " + s;
        } else {
          s = s + " " + e;
        }
        assert(len == int(e.length()));
        m_thread_result[nthread].push_back(SearchResult(
            s, len, 1)); // mark as 1 word only to not show number of words
      }
    }
  l1531:
    RETURN_ON_USER_BREAK
  }
}

void WordsBase::keyboardWords(int nthread) {
  int i;
  char a[256] = {0}, b[128], *p;
  std::string s;
  std::string::size_type j, len;
  const int di = getDictionaryIndex();
  Dictionary const &dt = m_dictionary[1];

  for (i = SETTINGS_KEYBOARD_ROW1; i <= SETTINGS_KEYBOARD_ROW3; i++) {
    s = m_settingsAll[0][i];
    for (j = 0; j < s.length(); j++) {
      // all english keys have russian char on the same keyboard key
      a[uchar(s[j])] = m_settingsAll[1][i][j];
    }
  }

  auto [it, end] = iterators(nthread, DICTIONARY_EN);
  for (; it != end; it++) {
    auto const &e = *it;
    p = b;
    len = e.length();

    const unsigned char *src =
        reinterpret_cast<const unsigned char *>(e.data());
    const unsigned char *end = src + len;

    while (src < end) {
      *p++ = a[*src++];
    }
    *p = 0;

    if (std::ranges::binary_search(dt, b)) {
      s = di ? b + (" " + e) : e + " " + b;
      m_thread_result[nthread].push_back(SearchResult(s, len, 1));
    }
  }
}

void WordsBase::wordFrequency(int nthread) {
  int i, j, k;
  const int MAX = getMaximumWordLength();
  VectorInt m(MAX, 0);
  VectorIntInt v[2];
  std::string s, s1;

  for (auto &a : v) {
    a = {getWordLengthMap().begin(), getWordLengthMap().end()};
  }

  std::sort(v[0].begin(), v[0].end(),
            [](auto &a, auto &b) { return a.second > b.second; });
  std::sort(v[1].begin(), v[1].end(),
            [](auto &a, auto &b) { return a.first < b.first; });

  size_t w = intToString(v[0][0].second).size(); // max len
  int sz = getDictionary().size();
  const int SP = 10;
  const std::string separator(SP, ' ');
  double ev = 0, disp = 0;
  for (i = 0; i < std::ssize(v[0]); i++) {
    // use separator for intToString for understandable view
    for (j = 0; j < 2; j++) {
      auto &e = v[j][i];
      auto pr = double(e.second) / sz;
      if (j == 0) {
        ev += pr * e.first;
      }
      auto s = std::format("{:2d} {:6.3f}% {:>{}}/{}", e.first, 100. * pr,
                           intToString(e.second), w, intToString(sz));
      if (i == 0 && j == 0) {
        s1 = string(WORD_LENGTH_FREQUENCY);
        k = s.length() + SP - utf8Length(s1);
        if (k <= 0) {
          k = 1;
        }
        SearchResult::out =
            s1 + std::string(k, ' ') + string(WORD_LENGTH_FREQUENCY1);
      }
      SearchResult::out += (j ? separator : "\n") + s;
    }
  }
  for (auto &e : v[0]) {
    auto pr = double(e.second) / sz;
    double q = e.first - ev;
    disp += pr * q * q;
  }
  i = std::ssize(v[0]);
  disp *= i / (i - 1);

  i = -1;
  for (auto &a : {EXPECTED_VALUE, STANDARD_DEVIATION, VARIANCE}) {
    i++;
    SearchResult::out += std::format(
        "\n{} {:.3f}", string(a), i ? (i == 1 ? std::sqrt(disp) : disp) : ev);
  }
}

void WordsBase::checkDictionary(int nthread) {
  std::string s, p = path(getDictionaryIndex(), "words");
  char buffer[256];
  const int THREADS = m_vs.size();
  const int MAX_ERRORS = 100 / THREADS;
  int line = 0, errors = MAX_ERRORS;
  const bool lastThread = nthread == THREADS - 1;
  m_vs[nthread] = "";

  auto addError = [&line, &errors, nthread, this](ENUM_STRING error) {
    m_vs[nthread] += (m_vs[nthread].empty() ? "" : "\n") +
                     capitalizeFirstUtf8(string(STRING_ERROR)) + " " +
                     string(error) + ", " + string(THREAD) + " " +
                     std::to_string(nthread + 1) + ", " + string(LINE) + " " +
                     intToStringLocaled(line) + "." +
                     (nthread == 0 ? "" : " " + string(THREAD_NOTE));
    if (!(--errors)) {
      throw std::runtime_error("");
    }
  };

  FILE *file = std::fopen(p.c_str(), "rb");
  if (file) {
    try {
      int pos = m_chd[nthread];
      fseek(file, pos, SEEK_SET);
      while (std::fgets(buffer, sizeof(buffer), file) != nullptr) {
        line++;
        std::string a(buffer);
        // ftell long function so count pos manuallly
        pos += a.size();
        if (a.empty()) {
          addError(EMPTY_WORD_FOUND);
        } else {
          if (a.back() == '\n') { // last line can be ended with \n or not
            a.pop_back();
          }
          if (a.empty()) {
            addError(EMPTY_WORD_FOUND);
          } else {
            if (a.back() == '\r') {
              addError(CR_SYMBOL_FOUND);
            } else {
              if (spanIncluding(a, alphabet())) {
                if (a <= s) { // as well check duplicate words
                  addError(WORDS_NOT_IN_ALPHABET_ORDER);
                } else {
                  s = a;
                }
              } else {
                addError(FOUND_SYMBOL_OUT_OF_ALPHABET);
              }
            }
          }
        }
        // pos==m_chd[nthread + 1] finish, but if not lastThread we should do
        // additional check between chunks
        if ((lastThread && pos == m_chd[nthread + 1]) ||
            (!lastThread && pos > m_chd[nthread + 1]))
          break;
      }
    } catch (const std::runtime_error &) {
    }
    std::fclose(file);
  } else {
    assert(0);
  }
}

void WordsBase::findWordsSplit(int nthread) {
  std::string t;
  int words;
  std::size_t i, j;
  auto [it, end] = iterators(nthread);
  m_thread_result[nthread].clear();
  for (; it != end; it++) {
    auto const &e = *it;
    if (int(e.size()) != m_comboValue[COMBOBOX_HELPER0]) {
      continue;
    }
    t = "";
    words = 0;

    for (i = 1; i < e.size(); i++) {
      std::string_view s1(e.data(), i);
      std::string_view s2(e.data() + i, e.size() - i);
      if (std::ranges::binary_search(getDictionary(), s1) &&
          std::ranges::binary_search(getDictionary(), s2)) {
        t += std::format("{}{{{} {}}}", words ? " " : "", s1, s2);
        words += 2;
      }
    }

    for (i = 1; i < e.size() - 1; i++) {
      for (j = i + 1; j < e.size(); j++) {
        std::string_view s1(e.data(), i);
        std::string_view s2(e.data() + i, j - i);
        std::string_view s3(e.data() + j, e.size() - j);

        if (std::ranges::binary_search(getDictionary(), s1) &&
            std::ranges::binary_search(getDictionary(), s2) &&
            std::ranges::binary_search(getDictionary(), s3)) {
          t += std::format("{}{{{} {} {}}}", words ? " " : "", s1, s2, s3);
          words += 3;
        }
      }
    }
    if (words) {
      // m_result.push_back(SearchResult(t, e.length(), words));
      m_thread_result[nthread].push_back(SearchResult(t, e.length(), words));
    }
    RETURN_ON_USER_BREAK
  }
}

void WordsBase::twoCharactersDistribution(int nthread) {
  int i;
  const int n = alphabetSize();
  auto &st = m_tr[nthread];
  st.clear(n);
  int &total = st.total;
  auto &a = st.a;
  auto [it, end] = iterators(nthread);
  for (; it != end; it++) {
    auto const &e = *it;
    if (e.length() < 2) {
      continue;
    }
    if (m_menuClick == MENU_TWO_CHARACTERS_DISTRIBUTION) {
      for (i = 0; i < int(e.length()) - 1; i++) {
        a[alphabetIndex(e[i])][alphabetIndex(e[i + 1])]++;
        total++;
      }
    } else {
      i = m_menuClick == MENU_TWO_CHARACTERS_DISTRIBUTION_START
              ? 0
              : e.length() - 2;
      a[alphabetIndex(e[i])][alphabetIndex(e[i + 1])]++;
      total++;
    }
  }
}

void WordsBase::twoCharactersDistributionPostProseeding() {
  int i, j;
  VectorStringIntCI p;
  const int n = alphabetSize();
  VectorStringInt v;
  auto &a = sum(m_tr);
  int total = 0;
  for (auto &e : m_tr) {
    total += e.total;
  }

  v.reserve(n * n);
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      if (a[i][j] != 0) {
        std::string s = {alphabet()[i], alphabet()[j]};
        v.emplace_back(std::move(s), a[i][j]);
      }
    }
  }
  std::sort(v.begin(), v.end(), sortStringInt);

  if (v.empty())
    return;

  p = v.begin();

  i = static_cast<int>(
      std::formatted_size("{:.2f}", (p->second * 100.0) / total));
  j = static_cast<int>(
      std::formatted_size("{}", intToStringLocaled(p->second)));

  for (p = v.begin(); p != v.end(); p++) {
    if (p != v.begin()) {
      SearchResult::out += "\n";
    }

    double percentage = (p->second * 100.0) / total;

    std::string formatted_str = std::format(
        "{} {:>{}.2f}% = {:>{}} / {}", p->first, percentage, i,
        intToStringLocaled(p->second), j, intToStringLocaled(total));

    SearchResult::out += localeToUtf8(formatted_str);
  }

  m_addstatus =
      std::format("{} {}", string(PAIRS), intToStringLocaled(v.size()));
}

void WordsBase::dictionaryStatistics(int nthread) {
  const int SZ_CAPTION = 3;
  const int a = alphabetSize();
  auto &st = m_tr[nthread];
  st.clear(SZ_CAPTION, a + 1);
  auto &m = st.a;
  std::string &longestWord = st.s;
  auto [it, end] = iterators(nthread);
  for (; it != end; it++) {
    auto const &e = *it;
    m[0][a] += e.length();
    if (e.length() > longestWord.length()) {
      longestWord = e;
    }
    for (auto c : e) {
      m[0][alphabetIndex(c)]++;
    }
    m[1][alphabetIndex(e.front())]++;
    m[2][alphabetIndex(e.back())]++;
  }
}

void WordsBase::dictionaryStatisticsPostProseeding() {
  int i, j;
  unsigned k;
  double v;
  std::string s, s2, longestWord;
  const int SZ_CAPTION = 3;
  const int a = alphabetSize();
  auto &m = sum(m_tr);
  Dictionary const &r = getDictionary();
  m[1][a] = m[2][a] = r.size();

  for (auto &e : m_tr) {
    if (e.s.size() > longestWord.size()) {
      longestWord = e.s;
    }
  }

  std::string caption[SZ_CAPTION];
  std::string additionalCaption[2];
  for (i = 0; i < SZ_CAPTION; i++) {
    caption[i] =
        string(i == 0 ? CHARACTER_FREQUENCY_PERCENTS
                      : (i == 1 ? CHARACTER_FREQUENCY_AT_THE_BEGINNING_PERCENTS
                                : CHARACTER_FREQUENCY_AT_THE_END_PERCENTS));
  }

  for (i = 0; i < std::ssize(additionalCaption); i++) {
    additionalCaption[i] =
        " " + string(i == 0 ? SORTED_BY_ALPHABET : SORTED_BY_FREQUENCY);
  }

  SearchResult::out =
      string(PROCEED_SYMBOLS) + " " + intToStringLocaled(m[0][a]) + ", " +
      string(WORDS_GENITIVE_CASE) + " " + intToStringLocaled(m[1][a]) + "\n" +
      string(AVERAGE_WORD_LENGTH_EQUALS) +
      std::format(" {:.2f}", (double(m[0][a])) / m[1][a]);

  IntDouble ve[a];
  const int COLUMNS = a / 3;
  std::vector<double> frequency(alphabetSize());
  constexpr std::string_view F = "{} {:>6.3f}  ";

  for (j = 0; j < SZ_CAPTION; j++) {
    SearchResult::out += "\n\n" + caption[j] + additionalCaption[0];

    for (s = "", i = 0; i < a; ++i) {
      v = 100. * m[j][i] / m[j][a];
      s += std::format(F, localeToUtf8(alphabet().substr(i, 1)), v);

      if ((i + 1) % COLUMNS == 0 || i == a - 1) {
        SearchResult::out += "\n" + s;
        s = "";
      }

      ve[i] = {i, v};
      frequency[i] = v;
    }

    SearchResult::out += "\n\n" + caption[j] + additionalCaption[1];
    std::sort(ve, ve + a, sortIntDouble);

    s = "";
    for (i = 0; i < a; i++) {
      s2 = localeToUtf8(alphabet().substr(ve[i].first, 1));
      s += std::format(F, s2, ve[i].second);

      if ((i + 1) % COLUMNS == 0 || i == a - 1) {
        SearchResult::out += "\n" + s;
        s = "";
      }
    }

    if (j == 0) {
      SearchResult::out += "\n\n" + string(FREQUENCY_OF_KEYBOARD_CHARACTERS);
      for (i = 0; i < 3; i++) {
        s = "";
        s2 = settings(SETTINGS_KEYBOARD_ROW1, i);
        for (k = 0; k < s2.length(); k++) {
          s += std::format(F, localeToUtf8(s2.substr(k, 1)),
                           frequency[alphabetIndex(s2[k])]);
        }
        SearchResult::out += "\n" + s;
      }
    }
  }

  SearchResult::out += std::format(
      "\n\n{} - {} ({} {}).", string(THE_LONGEST_WORD_IS),
      localeToUtf8(longestWord), string(LENGTH), longestWord.length());
}

void WordsBase::sortFilterResults(ENUM_JOB_TYPE e) {
  int i;
  std::string s;

#ifndef NOGTK
  bool find = !m_buttonValue[BUTTON_FOUND];
#endif
  if (m_result.empty()) {
    return;
  }
  SearchResult::out = "";
  // auto begin = clock();
  if (e != JOB_TYPE_FILTER) {
    // mtsort slowdown
    std::sort(
        m_result.begin(), m_result.end(),
        SORT_FUNCTION[m_sortlistValue * 2 + m_buttonValue[BUTTON_SORT_ORDER]]);
  }
  // auto te = timeElapse(begin);
  for (auto const &e : m_result) {
    s = fastLocaleToUtf8(e.s);
#ifndef NOGTK
    if (find != testFilterRegex(s)) {
      continue;
    }
    m_filteredWordsCount++;
#endif
    if (!SearchResult::out.empty()) {
      SearchResult::out += "\n";
    }
    SearchResult::out +=
        s + OPEN_S + string(CHARACTERS) + std::format(" {}", e.length);

    if (e.words > 1) {
      SearchResult::out +=
          std::format(" {} {}", string(WORDS_GENITIVE_CASE), e.words);
    }

    i = m_sortlistValue - (NUMBER_OF_SORTS - 3);
    if (i == 0 || i == 1) { // sort by vowels, consonants
      SearchResult::out +=
          " " + string(VOWELS + i) +
          std::format(" {:.1f}%",
                      e.percent[i]); // show percent of vowels or consonants
    } else if (i == 2) {
      SearchResult::out += " " + string(DIFFERENT_CHARACTERS) +
                           std::format(" {}", e.differentCharacters);
    }
    SearchResult::out += ")";
    RETURN_ON_USER_BREAK
  }
  // prsync("sort", te, timeElapse(begin));
}

void WordsBase::loadLanguages() {
  int i, j, n;
  VString v;
  const int START = 1;

  for (n = 0; n < LANGUAGES; n++) {
    i = 0;
    j = -1;
    v = readFile(n, "language");
    for (auto &s : v) {
      j++;
      if (breakMenu(s)) {
        /* use only first symbol. In Russian language separator is space, so
         * ignore comment in language.txt file. Comment in ru/language.txt is
         * important because otherwise it'll we empty string and looks like a
         * bug
         */
        s = s.substr(0, 1);
        break;
      }
      assert(!s.empty());
      if (s == SEPARATOR || s[0] == '}') {
        continue;
      }
      if (s.back() == '{') {
        s.pop_back();
      }
      assert(i < MENU_SIZE);
      m_menuAll[n][i++] = s;
    }
    assert(i == MENU_SIZE);
    assert(v.size() - j == int(SETTINGS_SIZE) + int(STRING_SIZE));
    auto cut_start = v.begin() + j + START;
    auto cut_end = cut_start + SETTINGS_SIZE;
    std::move(cut_start, cut_end, m_settingsAll[n].begin());
    v.erase(cut_start, cut_end);
    std::move(v.begin() + j, v.end(), m_languageAll[n].begin());
    for (auto &a : m_settingsAll[n])
      a = utf8ToLocale(a);
  }
}

bool WordsBase::prepare() {
  std::string s;
  // pr(magic_enum::enum_name(m_menuClick));

  if (!MENU_WITH_ENTRY.get(m_menuClick)) {
    return true;
  }
  std::string evUtf8 = utf8Trim(m_entryValue[ENTRY_TEMPLATE]);
  auto result = tryUtf8ToLocale(evUtf8);
  if (!result.has_value()) {
    return false;
  }
  m_ev = *result;
  if (m_ev.empty()) {
    return false;
  }

  if (m_menuClick == MENU_REGULAR_EXPRESSIONS) {
    return createRegex(ENTRY_TEMPLATE);
  }

  if (m_menuClick == MENU_MODIFICATION) { // finish with MENU_MODIFICATION
    s = m_entryValue[ENTRY_TEMPLATE];
    if (!m_modifications.parse(m_ev)) {
#ifdef NOGTK
      printf(string(CGI_STRING_ERROR_INVALID_MODIFICATION_STRING).c_str());
#endif
      return false;
    }
    return true;
  }

  // MENU_CHAIN MENU_TEMPLATE MENU_CROSSWORD MENU_CHARACTER_SEQUENCE
  s = alphabet();
  if (m_menuClick == MENU_CROSSWORD) {
    s += '*';
  } else if (m_menuClick == MENU_CHAIN) {
    s += ' ';
  }
  if (!spanIncluding(m_ev, s)) {
    return false;
  }

  if (m_menuClick == MENU_TEMPLATE) {
    memset(m_templateHelper, 0, 256);
    const char *p;
    for (p = m_ev.c_str(); *p != 0; p++) {
      m_templateHelper[uchar(*p)]++;
    }

    size_t i, j;
    m_template_a = create2dArray<char>(m_ev.length(), 256, 0);
    j = 0;
    for (auto &a : m_template_a) {
      s = m_ev.substr(j);
      for (i = 0; i < s.length(); i++) {
        uchar u = s[i];
        if (!a[u]) {
          a[u] = i + 1;
        }
      }
      j++;
    }
  } else if (m_menuClick == MENU_CHAIN) {
    // evUtf8 trimmed and has only alphabet chars
    m_evwords = getLocaleWords(evUtf8);
    m_tvwords = getLocaleWords(m_textViewValueUtf8);

    if (m_evwords.size() != 1 && m_evwords.size() != 2) {
      return false;
    }

    if (!getWordLengthMap().contains(
            m_evwords[0].size())) { // no words such length
      return false;
    }

    // filter words
    m_ss.clear();
    for (auto &a : m_tvwords) {
      if (a.size() == m_evwords[0].size()) {
        m_ss.insert(a);
      }
    }

    if (std::any_of(m_evwords.begin(), m_evwords.end(), [this](auto &a) {
          return m_ss.contains(a);
        })) { // one of the words in filterd words
      return false;
    }

    if (m_evwords.size() == 1) {
      return true;
    } else if (m_evwords.size() == 2) {
      return m_evwords[0] != m_evwords[1] &&
             m_evwords[0].size() == m_evwords[1].size();
    }
  }

  return true;
}

std::string WordsBase::getStatusString() {
  std::string s = m_addstatus;
  if (!s.empty()) {
    s += ", ";
  }
  if (!m_result.empty()) {
    s = string(LINES) + " " + intToStringLocaled(m_result.size()) + ", ";
#ifndef NOGTK
    s += string(WITH_FILTER) + " " + intToStringLocaled(m_filteredWordsCount) +
         ", ";
#endif
  }
  return s + string(TIME) + " " + getTimeString();
}

std::string WordsBase::getTimeString() {
  return std::format(
      "{:.2f}", double(m_end - m_begin) /
                    CLOCKS_PER_SEC); // locale independent
                                     //  return format("%.2lf", double(m_end -
                                     //  m_begin) / CLOCKS_PER_SEC);
}

IntInt WordsBase::iteratorsSize(int nthread, int total_size) {
  int chunk_size = total_size / m_threads;
  int remainder = total_size % m_threads;
  int start_idx = nthread * chunk_size + std::min(nthread, remainder);
  int end_idx = start_idx + chunk_size + (nthread < remainder ? 1 : 0);
  return {start_idx, end_idx};
}

PairDCIDCI WordsBase::iterators(int nthread, int n) {
  if (n == -1) {
    n = getDictionaryIndex();
  }
  Dictionary const &d = m_dictionary[n];
  const auto &p = iteratorsSize(nthread, d.size());
  return {d.begin() + p.first, d.begin() + p.second};
}

void WordsBase::run_thread(int nthread) {
  // auto begin = clock();
  m_thread_result[nthread].clear();
  if (auto it = COMPLEX_FUNCTION.get(m_menuClick)) {
    (this->*(*it))(nthread);
  } else if (m_menuClick == MENU_REGULAR_EXPRESSIONS) {
    UniquePcre2Code r;
    UniquePcre2MatchData m;
    if (!createRegex(ENTRY_TEMPLATE, r, m)) {
      assert(0);
      return;
    }

    bool isEnglish = getDictionaryIndex() == DICTIONARY_EN;
    auto n = isEnglish ? DICTIONARY_EN : DICTIONARY_RU_UTF8;
    auto [it2, end] = iterators(nthread, n);
    for (; it2 != end; it2++) {
      if (checkRegularExpression(*it2, r.get(), m.get())) {
        auto d = std::distance(m_dictionary[n].cbegin(), it2);
        auto &e = m_dictionary[getDictionaryIndex()][d];
        m_thread_result[nthread].push_back(SearchResult(e, e.length(), 1));
      }
      RETURN_ON_USER_BREAK
    }

  } else if (auto it = SIMPLE_FUNCTION.get(m_menuClick)) {
    auto [it2, end] = iterators(nthread);
    for (; it2 != end; it2++) {
      auto &e = *it2;
      if ((this->*(*it))(e)) {
        m_thread_result[nthread].push_back(SearchResult(e, e.length(), 1));
      }
      RETURN_ON_USER_BREAK
    }
  }
  // prsync(nthread, timeElapse(begin));
}

void WordsBase::run(ENUM_JOB_TYPE e) {
  bool userbreak = false;
  // auto begin = clock();
  int threads =
      oneOf(m_menuClick, MENU_CHAIN, MENU_WORD_FREQUENCY) ? 1 : m_threads;
  // pr(magic_enum::enum_name(m_menuClick), threads);
  if (e == JOB_TYPE_FULL) {
    if (auto it = menuPreProseeding.get(m_menuClick)) {
      (this->*(*it))();
    }

    std::vector<std::jthread> workers;
    for (int i = 0; i < threads; ++i) {
      workers.emplace_back(run_thread, this, i);
    }

    for (auto &t : workers) {
      if (t.joinable()) {
        t.join();
      }
    }

    if (auto it = menuPostProseeding.get(m_menuClick)) {
      (this->*(*it))();
    } else if (threads > 1) {
      m_result = m_thread_result | std::views::join |
                 std::ranges::to<VectorSearchResult>();
    }

    userbreak = m_token.stop_requested();
  }

  if (!userbreak) {
    sortFilterResults(e);
    // can be inside sortFilterResults
    userbreak = m_token.stop_requested();
  }
  m_end = clock();
  // pr(timeElapse(begin));
}

bool WordsBase::differenceOnlyOneChar(const std::string &a,
                                      const std::string &b) {
  auto ib = b.begin();
  int i = 0;
  for (auto &ia : a) {
    if (ia != *ib) {
      if (++i > 1) {
        return false;
      }
    }
    ib++;
  }
  return i == 1;
}

#ifndef NOGTK
bool WordsBase::testFilterRegex(const std::string &s) {
  if (!m_regex[ENTRY_FILTER] || !m_match[ENTRY_FILTER]) {
    return true;
  }

  int rc = pcre2_match(m_regex[ENTRY_FILTER].get(), (PCRE2_SPTR)s.c_str(),
                       s.length(), 0, 0, m_match[ENTRY_FILTER].get(), NULL);

  return rc >= 0;
}

#endif

std::string WordsBase::intToString(int v, char c) { return toString(v, c); }

std::string WordsBase::intToStringLocaled(int v) {
  return toString(v, string(SEPARATOR_SYMBOL)[0]);
}

std::string WordsBase::path(int i, std::string s) {
  return getResourcePath(LNG[i] + "/" + s + ".txt");
}

VString WordsBase::readFile(int i, std::string s) {
  return readFile(path(i, s));
}

VString WordsBase::readFile(std::string path) {
  VString lines;
  std::ifstream file(path);
  if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      lines.emplace_back(std::move(line));
    }
    file.close();
  } else {
    assert(0);
  }
  return lines;
}

#ifdef NOGTK
std::string WordsBase::getResourcePath(std::string name) {
  return "words/" + name;
  // return "../htdocs/words/words/" + name;
}

void WordsBase::cgi() {
  int i, j;
  std::string s;
  m_begin = clock();
  Cgi c;
  for (auto a : c) {
    i = INDEX_OF(a.first, POST_NAME);
    // assert(i!=POST_SIZE); later in switch
    s = a.second;
    if (i != POST_ENTRY) {
      // std::stoi throws exception if not valid number
      j = std::stoi(s.c_str());
    }
    switch (i) {
    case POST_SEARCHTYPE:
      m_menuClick = COMBO_MENU[j];
      break;

    case POST_ENTRY:
      m_ev = utf8ToLocale(s);
      break;

    case POST_DICTIONARY:
      m_comboValue[COMBOBOX_DICTIONARY] = j;
      break;

    case POST_SORTTYPE:
      m_sortlistValue = j;
      break;

    case POST_SORTORDER:
      m_comboValue[COMBOBOX_SORT_ORDER] = j;
      break;

    case POST_LANGUAGE:
      getLanguageIndex() = j;
      break;

    case POST_COMBO0:
    case POST_COMBO1:
    case POST_COMBO2:
      m_comboValue[COMBOBOX_HELPER0 + i - POST_COMBO0] = j;
      break;

    case POST_CHECK:
      m_checkValue = j != 0;
      break;

    default:
      assert(0);
    }
  }

  if (!prepare()) {
    printf(" %s", getTimeString().c_str());
    return;
  }

  run();
  printf("%s\n%s", getStatusString().c_str(), SearchResult::out.c_str());
}

#endif

// output all pairs to string
std::string WordsBase::pairsToString(VectorStringString const &v, bool p) {
  std::string sout = "";
  bool first = true;
  if (p) {
    sout += "[";
  }
  for (auto &e : v) {
    if (first) {
      first = false;
    } else {
      sout += p ? ", " : "\n";
    }
    sout += e.first + " " + e.second;
  }
  if (p) {
    sout += "]";
  }
  sout += "\n";
  return sout;
}

bool WordsBase::createRegex(ENUM_ENTRY e) {
  assert(int(e) < std::size(m_regex));
  return createRegex(e, m_regex[e], m_match[e]);
}

bool WordsBase::createRegex(ENUM_ENTRY e, UniquePcre2Code &r,
                            UniquePcre2MatchData &m) {
  int errorcode;
  PCRE2_SIZE erroroffset;
  r.reset(pcre2_compile((PCRE2_SPTR)m_entryValue[e].c_str(),
                        PCRE2_ZERO_TERMINATED, PCRE2_UTF, &errorcode,
                        &erroroffset, NULL));
  if (r) {
    pcre2_jit_compile(r.get(), PCRE2_JIT_COMPLETE);
    m.reset(pcre2_match_data_create_from_pattern(r.get(), NULL));
  }

  return r != nullptr;
}

const std::string &WordsBase::alphabet() const {
  return settings(SETTINGS_ALPHABET);
}

int WordsBase::alphabetSize() const { return alphabet().size(); }

int WordsBase::alphabetIndex(char c) const {
  auto i = alphabet().find(c);
  return i == std::string::npos ? -1 : i;
}

const std::string &WordsBase::vowelConsonant(bool consonant) {
  return settings(consonant ? SETTINGS_CONSONANTS : SETTINGS_VOWELS);
}

bool WordsBase::isAlphabetChar(char c) const { return alphabetIndex(c) != -1; }

const std::string &WordsBase::string(ENUM_STRING e) const {
  return m_languageAll[getLanguageIndex()][e];
}

const std::string WordsBase::string(ENUM_STRING e, ENUM_STRING e1) const {
  return string(e) + " " + string(e1);
}

const std::string &WordsBase::string(int i) const {
  assert(i >= 0 && i < STRING_SIZE);
  return string(ENUM_STRING(i));
}

const std::string &WordsBase::string(ENUM_MENU e) const {
  return m_menuAll[getLanguageIndex()][e];
}

const std::string &WordsBase::stringUsingDictionary(ENUM_STRING e,
                                                    bool opposite) const {
  return m_languageAll[getDictionaryIndex() ^ opposite][e];
}

const std::string &WordsBase::settings(ENUM_SETTINGS e, int i) const {
  int n = int(e) + i;
  assert(n < SETTINGS_SIZE);
  return m_settingsAll[getDictionaryIndex()][ENUM_SETTINGS(n)];
}

void WordsBase::updateProceedingThread() {}

void WordsBase::updateProceedingText(std::string s, bool add) {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    SearchResult::out = (add ? SearchResult::out : "") + s;
  }
  updateProceedingThread();
}

const MapIntInt &WordsBase::getWordLengthMap() const {
  return m_wordLengthMap[getDictionaryIndex()];
}
