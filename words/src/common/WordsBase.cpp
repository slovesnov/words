/*
 * WordsBase.cpp
 *
 *  Created on: 20.11.2017
 *      Author: alexey slovesnov
 */

#include "WordsBase.h"
#include "consts.h"
#include "magic_enum.hpp" //TODO
#include <cassert>
#include <ranges>

std::mutex cout_mutex;

#ifdef NOGTK
#define RETURN_ON_USER_BREAK
#else
#define RETURN_ON_USER_BREAK                                                   \
  if (m_token.stop_requested()) {                                              \
    return;                                                                    \
  }
#endif

std::string LNG[LANGUAGES];
std::vector<MapStringStringVector> eqmap;
const std::unordered_map<ENUM_MENU, void (WordsBase::*)(int)> menu2VoidInt = {
    {MENU_ANAGRAM, &WordsBase::findAnagram}, // break implemented
    {MENU_SIMPLE_WORD_SEQUENCE,
     &WordsBase::findSimpleWordSequence}, // break implemented
    {MENU_DOUBLE_WORD_SEQUENCE,
     &WordsBase::findDoubleWordSequence}, // break implemented
    {MENU_WORD_SEQUENCE_FULL,
     &WordsBase::findWordSequenceFull},                // not implemented
    {MENU_MODIFICATION, &WordsBase::findModification}, // break implemented
    {MENU_CHAIN, &WordsBase::findChain},               // not implemented
    {MENU_LETTER_GROUP_SPLIT,
     &WordsBase::findLetterGroupSplit}, // not implemented
    {MENU_TWO_DICTIONARIES_SIMPLE,
     &WordsBase::twoDictionariesSimple}, // break implemented
    {MENU_TWO_DICTIONARIES_TRANSLIT,
     &WordsBase::twoDictionariesTranslit}, // break implemented
    {MENU_TWO_DICTIONARIES_KEYBOARD_WORD,
     &WordsBase::keyboardWords}, // not implemented
    {MENU_DICTIONARY_STATISTICS,
     &WordsBase::dictionaryStatistics},                   // not implemented
    {MENU_WORD_FREQUENCY, &WordsBase::wordFrequency},     // not implemented
    {MENU_CHECK_DICTIONARY, &WordsBase::checkDictionary}, // not implemented
    {MENU_TWO_CHARACTERS_DISTRIBUTION,
     &WordsBase::twoCharactersDistribution}, // not implemented
    {MENU_TWO_CHARACTERS_DISTRIBUTION_START,
     &WordsBase::twoCharactersDistribution}, // not implemented
    {MENU_TWO_CHARACTERS_DISTRIBUTION_END,
     &WordsBase::twoCharactersDistribution} // not implemented
};
/*TODO
MENU_CHAIN, findChain
MENU_LETTER_GROUP_SPLIT, findLetterGroupSplit
*/

const std::unordered_map<ENUM_MENU, void (WordsBase::*)()> menuPreProseeding = {
    {MENU_KEYBOARD_WORD_SIMPLE,
     &WordsBase::checkKeyboardWordSimplePreProseeding},
    {MENU_KEYBOARD_WORD_COMPLEX,
     &WordsBase::checkKeyboardWordComplexPreProseeding},
    {MENU_CHECK_DICTIONARY, &WordsBase::checkDictionaryPreProseeding}};

const std::unordered_map<ENUM_MENU, void (WordsBase::*)()> menuPostProseeding =
    {{MENU_WORD_FREQUENCY, &WordsBase::wordFrequencyPostProseeding},
     {MENU_DICTIONARY_STATISTICS,
      &WordsBase::dictionaryStatisticsPostProseeding},
     {MENU_SIMPLE_WORD_SEQUENCE,
      &WordsBase::simpleDoubleWordSequencePostProseeding},
     {MENU_DOUBLE_WORD_SEQUENCE,
      &WordsBase::simpleDoubleWordSequencePostProseeding},
     {MENU_TWO_CHARACTERS_DISTRIBUTION,
      &WordsBase::twoCharactersDistributionPostProseeding},
     {MENU_TWO_CHARACTERS_DISTRIBUTION_START,
      &WordsBase::twoCharactersDistributionPostProseeding},
     {MENU_TWO_CHARACTERS_DISTRIBUTION_END,
      &WordsBase::twoCharactersDistributionPostProseeding},
     {MENU_CHECK_DICTIONARY, &WordsBase::checkDictionaryPostProseeding}};

const std::unordered_map<ENUM_MENU, bool (WordsBase::*)(const std::string &)>
    menu2BoolString = {
        {MENU_PANGRAM, &WordsBase::checkPangram},
        {MENU_TEMPLATE, &WordsBase::checkTemplate},
        {MENU_PALINDROME, &WordsBase::checkPalindrome},
        {MENU_CROSSWORD, &WordsBase::checkCrossword},
        {MENU_REGULAR_EXPRESSIONS, &WordsBase::checkRegularExpression},
        {MENU_CHARACTER_SEQUENCE, &WordsBase::checkCharacterSequence},
        {MENU_KEYBOARD_WORD_SIMPLE, &WordsBase::checkKeyboardWordSimple},
        {MENU_KEYBOARD_WORD_COMPLEX, &WordsBase::checkKeyboardWordComplex},
        {MENU_CONSONANT_VOWEL_SEQUENCE,
         &WordsBase::checkConsonantVowelSequence},
        {MENU_DENSITY, &WordsBase::checkDensity}};

const std::unordered_map<ENUM_MENU, ENUM_STRING> menu2Settings = {
    {MENU_TEMPLATE, SETTINGS_TEMPLATE},
    {MENU_CROSSWORD, SETTINGS_CROSSWORD},
    {MENU_REGULAR_EXPRESSIONS, SETTINGS_REGULAR_EXPRESSIONS},
    {MENU_MODIFICATION, SETTINGS_MODIFICATION},
    {MENU_CHAIN, SETTINGS_CHAIN},
    {MENU_CHARACTER_SEQUENCE, SETTINGS_CHARACTER_SEQUENCE},
    {MENU_LETTER_GROUP_SPLIT, SETTINGS_LETTER_GROUP_SPLIT}};

#ifdef NOGTK
// use cgi project
#include "cgi.h"

#endif

WordsBase *wordsBase;

WordsBase::WordsBase() {
  int i, j;
  wordsBase = this;
  // clock_t begin = clock();

  int k;
  const int DICTIONARY_SIZE[] = {393'167, 2'415'401};
  for (i = 0; i < LANGUAGES; i++) {
    k = 0;
    // clock_t begin = clock();

    LNG[i] = LANGUAGE[i].substr(0, 2);

    // load dictionaries
    m_longestWordLength[i] = 0;
    std::ifstream file(path(i, "words"));
    assert(file.is_open());
    std::string line, s;
    if (i == DICTIONARY_RU)
      m_dictionary[DICTIONARY_RU_UTF8].resize(DICTIONARY_SIZE[i]);
    m_dictionary[i].resize(DICTIONARY_SIZE[i]);
    while (std::getline(file, line)) {
      j = line.size();
      if (j > m_longestWordLength[i]) {
        m_longestWordLength[i] = j;
      }
      assert(k != DICTIONARY_SIZE[i]);
      if (i == DICTIONARY_RU)
        m_dictionary[DICTIONARY_RU_UTF8][k] = fastLocaleToUtf8(line);
      m_dictionary[i][k] = std::move(line);
      k++;
    }
    file.close();

    // pr(i, timeElapse(begin), k, DICTIONARY_SIZE[i]);
    assert(k == DICTIONARY_SIZE[i]);
  }
  // pr(timeElapse(begin));

  int threads = g_get_num_processors(); // std::hardware_concurrency();
  m_tr.resize(threads);
  m_iv.resize(threads);
  m_ma.resize(threads);
  m_chdv.resize(threads);
  m_thread_result.resize(threads);

  loadLanguages();

  // test();
#ifdef NOGTK
  // cgi();TODO uncomment on real cgi query

  // setDictionaryIndex(1);

  // count longest constants needs when dictionary changed
  // checkLFAllFiles();
  // system("chcp 1251>nul");
  // showLongestAnagram();
  // showLongestPangram();
  // showLongestSimpleWordSequence();
  // showLongestDoubleWordSequence();
#endif
}

void WordsBase::test() {}

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
  return differentChars(s) >= m_comboValue[COMBOBOX_HELPER0];
}

bool WordsBase::checkTemplate(const std::string &s) {
  const char *p = s.c_str();
  int param = m_comboValue[COMBOBOX_HELPER0];
  if (param == 0) {
    if (s.length() < m_entryValue.length()) {
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
    if (s.length() > m_entryValue.length()) {
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
      if (!k || ((j += k) >= m_entryValue.length() && i + 1 < s.length())) {
        return false;
      }
    }
    return true;
  } else {
    return spanIncluding(s, m_entryValue);
  }
}

bool WordsBase::checkCharacterSequence(const std::string &s) {
  auto j = m_comboValue[COMBOBOX_HELPER2];
  if (j == 0) {
    auto min = m_comboValue[COMBOBOX_HELPER0];
    auto max = m_comboValue[COMBOBOX_HELPER1];
    j = 0;
    std::string::size_type pos = 0;
    while ((pos = s.find(m_entryValue, pos)) != std::string::npos) {
      if (!m_radioValue) {
        return true;
      }
      j++;
      if (j > max) {
        return false;
      }
      pos += m_entryValue.length();
    }
    return j >= min;
  } else if (j == 1) {
    return s.compare(0, m_entryValue.length(), m_entryValue) == 0;
  } else {
    unsigned i = s.length();
    if (i < m_entryValue.length()) {
      return false;
    }
    return s.compare(i - m_entryValue.length(), m_entryValue.length(),
                     m_entryValue) == 0;
  }
}

bool WordsBase::checkCrossword(const std::string &s) {
  unsigned i;
  if (s.length() != m_entryValue.length()) {
    return false;
  }
  for (i = 0; i < m_entryValue.length(); ++i) {
    if (m_entryValue[i] != '*' && m_entryValue[i] != s[i]) {
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

bool WordsBase::checkRegularExpression(const std::string &s,
                                       const SafeGRegex &r) {
#ifdef USE_STANDARD_REGEX
  // todo always m_regex
  std::ptrdiff_t const matches(std::distance(
      std::sregex_iterator(s.begin(), s.end(), m_regex[ENTRY_TEMPLATE]),
      std::sregex_iterator()));
  return matches >= m_comboValue[COMBOBOX_HELPER0] &&
         matches <= m_comboValue[COMBOBOX_HELPER1];
#else
  // Fast path (single match check)
  if (!m_radioValue) {
    return g_regex_match(r.get(), s.c_str(), GRegexMatchFlags(0), NULL);
  }

  // Optimized path to count matches without repeating GMatchInfo allocation
  int i = 0;
  int start_pos = 0;
  const int max = m_comboValue[COMBOBOX_HELPER1];
  const int string_len = s.length();

  GMatchInfo *matchInfo = nullptr;

  // Find the first match
  if (g_regex_match_full(r.get(), s.c_str(), string_len, start_pos,
                         GRegexMatchFlags(0), &matchInfo, nullptr)) {

    while (g_match_info_matches(matchInfo) && i <= max) {
      i++;

      // Get the coordinates of the current match
      int start_match, end_match;
      g_match_info_fetch_pos(matchInfo, 0, &start_match, &end_match);

      // Advance the start position for the next search iteration
      start_pos = end_match;

      // If the match was zero-length (e.g., ".*" pattern), advance by 1
      // character to prevent an infinite loop
      if (start_match == end_match) {
        start_pos++;
      }

      if (start_pos > string_len) {
        break;
      }

      // Reuse matchInfo without reallocating internal heap memory
      g_match_info_next(matchInfo, nullptr);
    }
  }

  if (matchInfo) {
    g_match_info_free(matchInfo);
  }

  return i >= m_comboValue[COMBOBOX_HELPER0] && i <= max;
#endif
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
  int threads = g_get_num_processors();
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
  for (i = 0; i < threads; ++i) {
    start = i * (filesize / threads);
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
  for (auto &a : m_chdv) {
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

#ifdef NOGTK
std::string readBinaryFileToString(const std::string &filename) {
  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  std::string s;
  if (!file.is_open()) {
    pr("error cann't open file " + filename);
  } else {
    s.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(s.data(), s.size());
  }
  return s;
}

void WordsBase::checkLFAllFiles() {
  auto name = "language";

  auto checkFile = [this](const std::string &filepath) {
    std::string s = readBinaryFileToString(filepath);
    if (s.contains('\r')) {
      pr("file " + filepath + " has CR symbol");
    }
  };

  for (int n = 0; n < 2; n++) {
    checkFile(path(n, name));
  }

  for (int n = 0; n < 2; n++) {
    checkFile(getTwoDictionariesPath(n));
  }
}

void outMax(std::string f, std::array<int, 2> r) {
  std::string s;
  for (char c : f.substr(strlen("showLongest"))) {
    if (std::isupper(c))
      s += '_';
    s += std::toupper(c);
  }
  std::cout << std::format("const int MAX{}_LENGTH={};//{{{}}}\n", s,
                           std::max(r[0], r[1]), join(r));
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
  std::string s;
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
          std::cout << std::format("length{} {}\n", i, s);
          // pr1("MAX_ANAGRAM_LENGTH={}; {}", i, s);
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
  int i;
  std::string s;
  // can use string or string_view
  using V = std::vector<std::string_view>;
  using MapStringV = std::map<std::string, V>;
  MapStringV map;
  MapStringV::iterator cit;
  const int min = m_comboValue[COMBOBOX_HELPER0];
  const int max = m_comboValue[COMBOBOX_HELPER1];

  // at first make several sets by length is slower

  for (i = min; i <= max; i++) {
    auto [it, end] = iterators(getDictionaryIndex(), nthread);
    for (; it != end; it++) {
      auto const &e = *it;
      if (int(e.length()) != i) {
        continue;
      }
      s = e;
      std::sort(s.begin(), s.end());
      cit = map.find(s);
      if (cit == map.end()) {
        map[s] = {e};
      } else {
        cit->second.push_back(e);
      }
    }
    for (auto &[_, v] : map) {
      if (v.size() < 2) {
        continue;
      }
      s = joinV(v);
      m_thread_result[nthread].push_back(
          SearchResult(s, v.begin()->length(), v.size()));
    }
    map.clear();
    RETURN_ON_USER_BREAK
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
    auto [it, end] = iterators(getDictionaryIndex(), nthread);
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
    auto [it, end] = iterators(getDictionaryIndex(), nthread);
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
  auto [it, end] = iterators(getDictionaryIndex(), nthread);
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
  auto [it, end] = iterators(getDictionaryIndex(), nthread);
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

void WordsBase::findChain(int nthread) {
  int i, j, k, l, n, mx[2];
  MapStringInt dl;
  VString v, w[2];
  MapStringIntI mi;
  char c;
  std::string s;
  StringSet vs[2];
  ChainNodeVectorCI cni1;
  ChainNode *cp;
  std::set<std::string> ex;

  // auto begin = clock();

  if (differenceOnlyOneChar(m_chainHelper[0], m_chainHelper[1])) {
    SearchResult::out =
        localeToUtf8(m_chainHelper[0] + " " + m_chainHelper[1]) + OPEN_S +
        string(WORDS) + "2)";
    return;
  }

  std::stringstream ss(m_textViewValue);
  while (ss >> s) {
    if (s.size() == m_chainHelper[0].length())
      ex.insert(s);
  }

  i = 0;
  for (auto &e : getDictionary()) {
    if (e.length() == m_chainHelper[0].length()) {
      if (ex.contains(e)) {
        i++;
      } else
        dl[e] = 0;
    }
  }

  // begin = clock();

  j = 1; // j is step
  for (i = 0; i < 2; i++) {
    w[i].push_back(m_chainHelper[i]);
    dl[m_chainHelper[i]] = i == 0 ? j : -j;
  }
  while (1) {
    j++;
    for (i = 0; i < 2; i++) {
      v.clear();
      for (auto &vit : w[i]) {
        std::string &vr = vit;
        for (auto &ic : vr) {
          c = ic;
          for (auto &ia : alphabet()) {
            if (ia != c) {
              ic = ia;
              mi = dl.find(vr);
              if (mi != dl.end()) {
                if (mi->second == 0) {
                  v.push_back(vr);
                  mi->second = i == 0 ? j : -j;
                } else {
                  if ((mi->second > 0 && i == 1) ||
                      (mi->second < 0 && i == 0)) {
                    s = vr;
                    ic = c;
                    if (mi->second > 0) {
                      // USE set not vector to avoid same words. Example "????
                      // ????"
                      vs[0].insert(s);
                      vs[1].insert(vr);
                    } else {
                      // USE set not vector to avoid same words. Example "????
                      // ????"
                      vs[0].insert(vr);
                      vs[1].insert(s);
                    }
                  }
                }
              }
            }
          }
          ic = c;
        }
      }
      if (v.empty() || !vs[0].empty()) {
        goto l210;
        // break two cycles
      }
      w[i] = v;
    } // for(i)
  }

l210:
  if (vs[0].empty()) {
    SearchResult::out = string(NO_CHAINS_FOUND);
    return;
  }

  j = 0;
  for (i = 0; i < 2; i++) {
    mi = dl.find(*vs[i].begin());
    assert(mi != dl.end());
    mx[i] = mi->second;
    j += i == 0 ? mx[i] : -mx[i];
  }
  // mi->second==1 is start word
  // mi->second==-1 is end word
  // so j-=2;
  j -= 2;

  VVString vl(j);
  std::vector<ChainNodeVector> ch(j);
  IntVector ni(j), nis(j);

  if (j == 1) { // special proceeding for j==1 on change check check
                // [????->????]
    for (auto &e : vs[0]) {
      ch[0].push_back(ChainNode(e));
    }
  } else {
    // fill vl
    for (mi = dl.begin(); mi != dl.end(); mi++) {
      if (mi->second > 1 && mi->second <= mx[0]) {
        vl[mi->second - 2].push_back(mi->first);
      } else if (mi->second < -1 &&
                 mi->second >= mx[1]) { // mi->second==1 is end word
        assert(j + mi->second + 1 >= 0);
        vl[j + mi->second + 1].push_back(mi->first);
      }
    }

    // fill ch[] in middle
    mi = dl.find(*vs[0].begin());
    assert(mi != dl.end());
    for (i = 0, l = mi->second - 2; i < 2; i++, l++) {
      for (auto &e : vs[i]) {
        ch[l].push_back(ChainNode(e));
      }
    }

    // make links for ch[] in middle
    l -= 2;
    for (auto &e : ch[l]) {
      for (cni1 = ch[l + 1].begin(), k = 0; cni1 != ch[l + 1].end();
           cni1++, k++) {
        if (differenceOnlyOneChar(e.s, cni1->s)) {
          // find all links, so no break here
          e.next.push_back(k);
        }
      }
    }

    for (k = -1; k < 2; k += 2) {
      i = mi->second;
      if (k == -1) {
        i -= 3;
      }
      for (; k == -1 ? i >= 0 : i < j; i += k) {
        for (auto &vci : vl[i]) {
          cp = 0;
          l = i - k;
          n = -1;
          for (auto &e : ch[l]) {
            n++;
            if (differenceOnlyOneChar(vci, e.s)) {
              // find all links, so no break here
              if (!cp) {
                ch[i].push_back(ChainNode(vci));
                cp = &(ch[i].back());
              }
              if (k == -1) {
                cp->next.push_back(n);
              } else {
                e.next.push_back(ch[i].size() - 1);
              }
            }
          }
        }
      }
    }
  } // else (j)

  // BEGIN OUTPUT ALL TREE PATH
  // init path
  for (i = 0; i < j; i++) {
    ni[i] = 0;
  }
  nis[0] = ch[0].size();
  for (i = 1; i < j; i++) {
    nis[i] = ch[i - 1][ni[i - 1]].next.size();
  }

  // iterate path
  v.clear();
  while (1) {
    // output tree path
    s = m_chainHelper[0];
    for (i = 0; i < j; i++) {
      if (i == 0) {
        k = ni[i];
      } else {
        k = ch[i - 1][ni[i - 1]].next[ni[i]];
      }
      const ChainNode &node = ch[i][k];
      // printf("%s %d %d,%d\n",node.s.c_str(),i,ni[i],nis[i]);
      s += " ";
      s += node.s;
    }
    s += " ";
    s += m_chainHelper[1];
    v.push_back(s);

    for (k = j - 1; k > -1; k--) {
      if (++ni[k] != nis[k]) {
        break;
      }
    }
    if (k == -1) {
      break;
    } else {
      for (i = k + 1; i < j; i++) {
        ni[i] = 0;
        nis[i] = ch[i - 1][ni[i - 1]].next.size();
      }
    }
  }

  for (auto &e : v) {
    m_result.push_back(SearchResult(e, m_chainHelper[0].length(), j + 2));
  }
  // pr(timeElapse(begin))
}

void WordsBase::findLetterGroupSplit(int nthread) {
  std::string s, s1, t, lng;
  size_t i, j;
  auto charset = getOrderedString(m_entryValue);

  auto begin = clock();
  const size_t size = m_entryValue.length();
  eqmap.clear();
  eqmap.resize(size);

  for (auto &s : getDictionary()) {
    j = s.length();
    if (j < size) {
      s1 = getOrderedString(s);
      auto &m = eqmap[j];
      auto it = m.find(s1);
      if (it == m.end()) {
        t = sub(charset, s1);
        if (t != invalidDifference) {
          m.insert({s1, {s}});
        }
      } else {
        it->second.push_back(s);
      }
    }
  }

  pr(timeElapse(begin));

  auto v = getAllPairs(charset);
  size_t n[] = {v.size(), 0};

  if (!v.empty()) {
    SearchResult::out = localeToUtf8(pairsToString(v)) + "----------------\n";
  }

  for (i = 1; i < size; i++) {
    auto &m = eqmap[i];
    for (auto &e : m) {
      t = sub(charset, e.first);
      if (t != invalidDifference) {
        auto v = getAllPairs(t, e.first);
        if (!v.empty()) {
          n[1]++;
          SearchResult::out += localeToUtf8(getUserString(e.first) + " " +
                                            pairsToString(v, v.size() != 1));
        }
      }
    }
  }
  if (SearchResult::out.empty()) {
    SearchResult::out = string(SPLITS_NOT_FOUND);
  } else {
    for (i = 0; i < 2; i++) {
      m_addstatus += string(i ? TRIPLETS : PAIRS) + " " +
                     intToStringLocaled(n[i]) + (i ? "" : ", ");
    }
  }
  pr(timeElapse(begin));
}

std::string WordsBase::getTwoDictionariesPath(bool translit) {
  return getResourcePath(LNG[0] + LNG[1] + "_" +
                         (translit ? "translit" : "simple") + ".txt");
}

void WordsBase::twoDictionaries(int nthread, bool translit) {
  VVString to;
  int i, j, m, l, len, n, fromIndex = -1;
  std::string s, alphabetFrom;
  VString v;
  const int di = getDictionaryIndex();
  s = getTwoDictionariesPath(translit);
  for (auto &s : readFile(s)) {
    if (to.empty()) {
      fromIndex = indexOf(s, LNG);
      assert(fromIndex != -1);
      alphabetFrom = m_settingsAll[fromIndex][SETTINGS_ALPHABET];
      to.resize(alphabetFrom.length());
    } else {
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
  IntVector id(i);

  auto [it, end] = iterators(fromIndex, nthread);
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

  auto [it, end] = iterators(DICTIONARY_EN, nthread);
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
  const int MAX = getMaximumWordLength();
  auto &m = m_iv[nthread];
  m.assign(MAX, 0);
  auto [it, end] = iterators(getDictionaryIndex(), nthread);
  for (; it != end; it++) {
    auto const &e = *it;
    m[e.length() - 1]++; // use length-1
  }
}

void WordsBase::wordFrequencyPostProseeding() {
  int i, j;
  size_t k;
  const int MAX = getMaximumWordLength();
  IntVector m(MAX, 0);
  IntIntVector v[2];
  std::string s, s1;
  for (auto &e : m_iv) {
    for (i = 0; i < MAX; ++i) {
      m[i] += e[i];
    }
  }
  for (i = 0; i < MAX; ++i) {
    if (m[i] > 0) {
      v[0].push_back({m[i], i + 1}); // store actual word length
      v[1].push_back({m[i], i + 1});
    }
  }
  std::sort(v[0].begin(), v[0].end(),
            [](auto &a, auto &b) { return a.first > b.first; });
  std::sort(v[1].begin(), v[1].end(),
            [](auto &a, auto &b) { return a.second < b.second; });

  size_t w = toString(v[0][0].first, ',').size(); // max len
  int sz = getDictionary().size();
  const int SP = 20;
  const std::string separator(SP, ' ');
  for (i = 0; i < int(v[0].size()); i++) {
    // use separator for intToString for understandable view
    for (j = 0; j < 2; j++) {
      auto &e = v[j][i];
      auto s =
          std::format("{:2d} {:6.3f}% {:>{}}/{}", e.second, 100. * e.first / sz,
                      toString(e.first, ','), w, toString(sz, ','));
      if (i == 0 && j == 0) {
        s1 = string(WORD_LENGTH_FREQUENCY);
        k = s.length() + SP - g_utf8_strlen(s1.c_str(), -1);
        if (k > 0) {
          s1 += std::string(k, ' ');
        }
        SearchResult::out = s1 + string(WORD_LENGTH_FREQUENCY1);
      }
      SearchResult::out += (j ? separator : "\n") + s;
    }
  }
}

void WordsBase::checkDictionary(int nthread) {
  std::string s, p = path(getDictionaryIndex(), "words");
  char buffer[256];
  const int THREADS = m_chdv.size();
  const int MAX_ERRORS = 100 / THREADS;
  int line = 0, errors = MAX_ERRORS;
  const bool lastThread = nthread == THREADS - 1;
  m_chdv[nthread] = "";

  auto addError = [&line, &errors, nthread, this](ENUM_STRING error) {
    m_chdv[nthread] += (m_chdv[nthread].empty() ? "" : "\n") +
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

void WordsBase::twoCharactersDistribution(int nthread) {
  int i;
  const int n = alphabetSize();
  auto &st = m_tr[nthread];
  st.clear(n);
  int &total = st.total;
  auto &a = st.a;
  auto [it, end] = iterators(getDictionaryIndex(), nthread);
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

std::vector<IntVector> &sum(ThreadResultVector &v) {
  auto &a = v[0].a;
  for (size_t i = 1; i < v.size(); i++) {
    v[i].add(a);
  }
  return a;
}

void WordsBase::twoCharactersDistributionPostProseeding() {
  int i, j;
  std::string s;
  StringIntVectorCI p;
  const int n = alphabetSize();
  StringIntVector v;
  auto &a = sum(m_tr);
  int total = 0;
  for (auto &e : m_tr) {
    total += e.total;
  }

  v.reserve(n * n);
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      if (a[i][j] != 0) {
        s = alphabet()[i];
        s += alphabet()[j];
        v.emplace_back(std::move(s), a[i][j]);
      }
    }
  }

  std::sort(v.begin(), v.end(), sortStringInt);
  p = v.begin();
  i = format("%.2f", (p->second * 100.) / total).length();
  j = intToStringLocaled(p->second).length();
  s = "%s %" + std::to_string(i) + ".2f%% = %" + std::to_string(j) + "s / %s";
  for (p = v.begin(); p != v.end(); p++) {
    if (p != v.begin()) {
      SearchResult::out += "\n";
    }
    SearchResult::out += localeToUtf8(
        format(s.c_str(), p->first.c_str(), (p->second * 100.) / total,
               intToStringLocaled(p->second).c_str(),
               intToStringLocaled(total).c_str()));
  }
  m_addstatus = string(PAIRS) + " " + intToStringLocaled(v.size());
}

void WordsBase::dictionaryStatistics(int nthread) {
  const int SZ_CAPTION = 3;
  const int a = alphabetSize();
  auto &st = m_tr[nthread];
  st.clear(SZ_CAPTION, a + 1);
  auto &m = st.a;
  std::string &longestWord = st.s;
  auto [it, end] = iterators(getDictionaryIndex(), nthread);
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

  for (i = 0; i < SIZEI(additionalCaption); i++) {
    additionalCaption[i] =
        " " + string(i == 0 ? SORTED_BY_ALPHABET : SORTED_BY_FREQUENCY);
  }

  SearchResult::out = string(PROCEED_SYMBOLS) + " " +
                      intToStringLocaled(m[0][a]) + ", " + string(WORDS) + " " +
                      intToStringLocaled(m[1][a]) + "\n" +
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
  bool find = m_comboValue[COMBOBOX_FILTER] == 0;
#endif
  if (m_result.empty()) {
    return;
  }
  SearchResult::out = "";
  auto begin = clock();
  if (e != JOB_TYPE_FILTER) {
    std::sort(m_result.begin(), m_result.end(),
              SORT_FUNCTION[m_comboValue[COMBOBOX_SORT] * 2 +
                            m_comboValue[COMBOBOX_SORT_ORDER]]);
  }
  auto te = timeElapse(begin);
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
      SearchResult::out += std::format(" {} {}", string(WORDS), e.words);
    }

    i = m_comboValue[COMBOBOX_SORT] - (NUMBER_OF_SORTS - 3);
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
  prsync("sort", te, timeElapse(begin))
}

void WordsBase::loadLanguages() {
  std::string s, search;
  int i, j, n;
  bool b;
  for (n = 0; n < LANGUAGES; n++) {
    b = false;
    i = j = 0;
    auto &ml = m_languageAll[n];
    auto v = readFile(n, "language");
    for (auto const &e : v) {
      if (b) {
        if (j < SETTINGS_SIZE) {
          m_settingsAll[n][j++] = e;
        } else {
          if (j - SETTINGS_SIZE == SEARCH) {
            ml[(j++) - SETTINGS_SIZE] = utf8ToLowerCase(search);
          }
          ml[(j++) - SETTINGS_SIZE] = localeToUtf8(e);
        }
        continue;
      }
      if (e.empty()) {
        b = true;
        continue;
      }
      if (e == SEPARATOR || e[0] == '}') {
        continue;
      }
      s = localeToUtf8(e);
      if (s.back() == '{') {
        s.pop_back();
      }
      assert(i < MENU_SIZE);
      m_menuAll[n][i] = s;
      //  first item "search"
      if (!i) {
        search = s;
      }
      i++;
    }
    assert(i == MENU_SIZE);
    assert(j - SETTINGS_SIZE == STRING_SIZE);

    ml[MODIFICATION_HELP] = format(ml[MODIFICATION_HELP].c_str(),
                                   ml[EVERY_MODIFICATION_CHANGES_WORD].c_str());
    /* use only first symbol. In Russian language separator is space, so
     * ignore comment in language.txt file. Comment in ru/language.txt is
     * important because otherwise it'll we empty string and looks like a bug
     */
    ml[SEPARATOR_SYMBOL] = ml[SEPARATOR_SYMBOL].substr(0, 1);
  }
}

bool WordsBase::prepare() {
  std::string s;
  int i;
  std::string::size_type pb, pe;

  if (!isEntryMenu()) {
    return true;
  }
  // should encode to locale string at first to get valid length
  m_entryValue = utf8ToLocale(getEntryString(ENTRY_TEMPLATE));

  if (m_entryValue.empty()) {
    return false;
  }

  if (m_menuClick == MENU_REGULAR_EXPRESSIONS) {
#ifdef USE_STANDARD_REGEX
    try {
      m_regex =
          std::regex(m_entryValue.c_str(), std::regex_constants::icase |
                                               std::regex_constants::extended);
      return true;
    } catch (std::regex_error &) {
      return false;
    }
#else
    return createRegex(ENTRY_TEMPLATE);
#endif
  }

  if (m_menuClick == MENU_MODIFICATION) { // finish with MENU_MODIFICATION
    if (!m_modifications.parse(m_entryValue)) {
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
  if (!spanIncluding(m_entryValue, s)) {
    return false;
  }

  if (m_menuClick == MENU_TEMPLATE) {
    memset(m_templateHelper, 0, 256);
    const char *p;
    for (p = m_entryValue.c_str(); *p != 0; p++) {
      m_templateHelper[uchar(*p)]++;
    }

    size_t i, j;
    m_template_a = create2dArray<char>(m_entryValue.length(), 256, 0);
    j = 0;
    for (auto &a : m_template_a) {
      s = m_entryValue.substr(j);
      for (i = 0; i < s.length(); i++) {
        uchar u = s[i];
        if (!a[u]) {
          a[u] = i + 1;
        }
      }
      j++;
    }
  } else if (m_menuClick == MENU_CHAIN) { // all symbols from alphabet or spaces
    // m_chainHelper is only from alphabet checked
    s = m_entryValue;
    pe = -1;
    for (i = 0; i < 2; i++) {
      pb = s.find_first_not_of(' ', pe + 1);
      if (pb == std::string::npos) {
        return false;
      }
      pe = s.find_first_of(' ', pb + 1);
      if (pe == std::string::npos) {
        if (i == 1) {
          m_chainHelper[i] = s.substr(pb);
        } else {
          return false;
        }
      } else {
        m_chainHelper[i] = s.substr(pb, pe - pb);
        if (i == 1) { // only two words allowed
          if (s.find_first_not_of(' ', pe + 1) != std::string::npos) {
            return false;
          }
        }
      }
    }

    if (m_chainHelper[0] == m_chainHelper[1] ||
        m_chainHelper[0].length() != m_chainHelper[1].length()) {
      return false;
    }

    return true;
  }

  return true;
}

std::string WordsBase::getStatusString() {
  std::string s = m_addstatus;
  if (!s.empty()) {
    s += ", ";
  }
  if (!m_result.empty()) {
    s = string(NUMBER_OF_WORDS) + " " + intToStringLocaled(m_result.size()) +
        ", ";
#ifndef NOGTK
    s += string(WITH_FILTER) + " " + intToStringLocaled(m_filteredWordsCount) +
         ", ";
#endif
  }
  return s + string(TIME_OF_LAST_OPERATION) + " " + getTimeString();
}

std::string WordsBase::getTimeString() {
  return format("%.2lf", double(m_end - m_begin) / CLOCKS_PER_SEC);
}

PairDCIDCI WordsBase::iterators(int n, int nthread) {
  return iterators(ENUM_DICTIONARY(n), nthread);
}

PairDCIDCI WordsBase::iterators(ENUM_DICTIONARY e, int nthread) {
  Dictionary const &d = m_dictionary[e];
  int total_size = d.size();
  const int total_threads = g_get_num_processors();

  int chunk_size = total_size / total_threads;
  int remainder = total_size % total_threads;

  int start_idx = nthread * chunk_size + std::min(nthread, remainder);
  int end_idx = start_idx + chunk_size + (nthread < remainder ? 1 : 0);

  return {d.begin() + start_idx, d.begin() + end_idx};
}

void WordsBase::run_thread(int nthread) {
  auto begin = clock();
  m_thread_result[nthread].clear();

  auto it = menu2VoidInt.find(m_menuClick);
  if (it != menu2VoidInt.end()) {
    auto f = it->second;
    (this->*f)(nthread);
  }

  auto it1 = menu2BoolString.find(m_menuClick);
  if (it1 != menu2BoolString.end()) {
    auto f = it1->second;
    if (m_menuClick == MENU_REGULAR_EXPRESSIONS) {
      SafeGRegex r; // have to create separate regex, for every thread otherwise
      // very slow
      createRegex(ENTRY_TEMPLATE, r);
      bool isEnglish = getDictionaryIndex() == DICTIONARY_EN;
      auto n = isEnglish ? DICTIONARY_EN : DICTIONARY_RU_UTF8;
      auto [it2, end] = iterators(n, nthread);
      for (; it2 != end; it2++) {
        if (checkRegularExpression(*it2, r)) {
          auto d = std::distance(m_dictionary[n].cbegin(), it2);
          auto &e = m_dictionary[getDictionaryIndex()][d];
          m_thread_result[nthread].push_back(SearchResult(e, e.length(), 1));
        }
        RETURN_ON_USER_BREAK
      }

    } else {
      auto [it2, end] = iterators(getDictionaryIndex(), nthread);
      for (; it2 != end; it2++) {
        auto &e = *it2;
        if ((this->*f)(e)) {
          m_thread_result[nthread].push_back(SearchResult(e, e.length(), 1));
        }
        RETURN_ON_USER_BREAK
      }
    }
  }

  prsync(nthread, timeElapse(begin));
}

void WordsBase::run(ENUM_JOB_TYPE e) {
  bool userbreak = false;
  if (e == JOB_TYPE_FULL) {
    auto it = menuPreProseeding.find(m_menuClick);
    if (it != menuPreProseeding.end()) {
      auto f = it->second;
      (this->*f)();
    }

    std::vector<std::jthread> workers;
    int threads = oneOf(m_menuClick, MENU_CHAIN, MENU_LETTER_GROUP_SPLIT)
                      ? 1
                      : g_get_num_processors();
    prsync(threads, magic_enum::enum_name(m_menuClick));
    for (int i = 0; i < threads; ++i) {
      workers.emplace_back(run_thread, this, i);
    }
    for (auto &t : workers) {
      if (t.joinable()) {
        t.join();
      }
    }

    it = menuPostProseeding.find(m_menuClick);
    if (it != menuPostProseeding.end()) {
      auto f = it->second;
      (this->*f)();
    }

    if (threads > 1 && !oneOf(m_menuClick, MENU_SIMPLE_WORD_SEQUENCE,
                              MENU_DOUBLE_WORD_SEQUENCE)) {
      m_result = m_thread_result | std::views::join |
                 std::ranges::to<SearchResultVector>();
    }

    userbreak = m_token.stop_requested();
  }

  if (!userbreak) {
    sortFilterResults(e);
    // can be inside sortFilterResults
    userbreak = m_token.stop_requested();
  }
  m_end = clock();
  // m_result.clear();??
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
  return !m_regex[ENTRY_FILTER] ||
         g_regex_match(m_regex[ENTRY_FILTER].get(), s.c_str(),
                       GRegexMatchFlags(0), NULL);
}
#endif

std::string WordsBase::intToStringLocaled(int v) {
  return toString(v, string(SEPARATOR_SYMBOL)[0]);
}

std::string WordsBase::path(int i, std::string s) {
  return getResourcePath(getShortLanguageString(i) + "/" + s + ".txt");
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
      m_entryValue = utf8ToLocale(s);
      break;

    case POST_DICTIONARY:
      m_comboValue[COMBOBOX_DICTIONARY] = j;
      break;

    case POST_SORTTYPE:
      m_comboValue[COMBOBOX_SORT] = j;
      break;

    case POST_SORTORDER:
      m_comboValue[COMBOBOX_SORT_ORDER] = j;
      break;

    case POST_LANGUAGE:
      m_languageIndex = j;
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

std::string WordsBase::sub(std::string const &minuend,
                           std::string const &subtrahend) {
  if (minuend.length() < subtrahend.length()) {
    return invalidDifference;
  }
  if (minuend.length() == subtrahend.length()) {
    return minuend == subtrahend ? "" : invalidDifference;
  }

  std::string difference;
  auto p1 = minuend.c_str();
  auto p = subtrahend.c_str();
  for (; *p1; p1++) {
    if (*p1 == *p) {
      p++;
      if (!*p) {
        return difference + (p1 + 1);
      }
    } else if (*p1 < *p) {
      difference += *p1;
    } else {
      return invalidDifference;
    }
  }
  return invalidDifference;
}

std::string WordsBase::getOrderedString(std::string const &s) {
  auto o = s;
  std::sort(o.begin(), o.end());
  return o;
}

// get list of dictionary words from ordered string
std::string WordsBase::getUserString(std::string const &s) {
  auto &a = eqmap[s.length()].find(s)->second;
  bool f = true;
  std::string o;
  for (auto &e : a) {
    if (!f) {
      o += ' ';
    }
    o += e;
    f = false;
  }
  if (a.size() != 1) {
    o = '{' + o + '}';
  }
  return o;
}

StringStringVector WordsBase::getAllPairs(std::string const &s,
                                          std::string const &low) {
  StringStringVector v;
  for (size_t i = 1; i < s.size(); i++) {
    for (auto &e : eqmap[i]) {
      if (low == invalidDifference || low <= e.first) {
        auto a = sub(s, e.first);
        if (a != invalidDifference && e.first <= a) {
          if (eqmap[a.length()].contains(a)) {
            v.push_back({getUserString(e.first), getUserString(a)});
          }
        }
      }
    }
  }
  return v;
}

// output all pairs to string
std::string WordsBase::pairsToString(StringStringVector const &v, bool p) {
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

std::string WordsBase::getShortLanguageString(int i) {
  assert(i >= 0 && i < LANGUAGES);
  return LNG[i];
}

void WordsBase::setDictionaryIndex(int i) {
  assert(i >= 0 && i < LANGUAGES);
  m_dictionaryIndex = i;
}

int WordsBase::getDictionaryIndex() const { return m_dictionaryIndex; }

std::string WordsBase::getEntryString(ENUM_ENTRY e) const { return ""; }

std::string WordsBase::getTextViewString() const { return ""; }

bool WordsBase::getCheck() const { return false; }

bool WordsBase::createRegex(ENUM_ENTRY e) {
  assert(int(e) < SIZEI(m_regex));
  return createRegex(e, m_regex[e]);
}

// utf8
bool WordsBase::createRegex(ENUM_ENTRY e, SafeGRegex &r) {
  GRegexCompileFlags f =
      (GRegexCompileFlags)(G_REGEX_OPTIMIZE | G_REGEX_NO_AUTO_CAPTURE);
  auto s = getEntryString(e);
  r.reset(g_regex_new(s.c_str(), f, GRegexMatchFlags(0), NULL));
  return r.get() != nullptr;
}

bool WordsBase::isEntryMenu() const { return entryEnumString() != STRING_SIZE; }

ENUM_STRING WordsBase::entryEnumString() const {
  auto it = menu2Settings.find(m_menuClick);
  return it == menu2Settings.end() ? STRING_SIZE : it->second;
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
  return m_languageAll[m_languageIndex][e];
}

const std::string &WordsBase::string(int i) const {
  assert(i >= 0 && i < STRING_SIZE);
  return string(ENUM_STRING(i));
}

const std::string &WordsBase::stringUsingDictionary(ENUM_STRING e) const {
  return m_languageAll[getDictionaryIndex()][e];
}

const std::string &WordsBase::settings(ENUM_SETTINGS e, int i) const {
  int n = int(e) + i;
  assert(n < SETTINGS_SIZE);
  return m_settingsAll[getDictionaryIndex()][ENUM_SETTINGS(n)];
}
