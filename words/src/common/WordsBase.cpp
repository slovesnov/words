/*
 * WordsBase.cpp
 *
 *  Created on: 20.11.2017
 *      Author: alexey slovesnov
 */

#include "WordsBase.h"
#include "consts.h"
#include <cassert>
#include <ranges>
#include <unordered_map>

using uchar = unsigned char;

#ifdef NOGTK
#define RETURN_ON_USER_BREAK
#else
#define RETURN_ON_USER_BREAK                                                   \
  if (userBreakThread()) {                                                     \
    pr("user break", nthread);                                                 \
    return;                                                                    \
  }
#define RETURN_ON_USER_BREAK1                                                  \
  if (userBreakThread()) {                                                     \
    pr("user break sort");                                                     \
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

const std::map<ENUM_MENU, bool (WordsBase::*)(const std::string &)>
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
#ifdef NOGTK
// use cgi project
#include "cgi.h"

// should match with POST_ENUM
const std::string POST_NAME[] = {
    "searchType", "entry",  "dictionary", "sortType", "sortOrder",
    "language",   "combo0", "combo1",     "combo2",   "check"};

const ENUM_MENU COMBO_MENU[] = {MENU_ANAGRAM,
                                MENU_PANGRAM,
                                MENU_TEMPLATE,
                                MENU_PALINDROME,
                                MENU_CROSSWORD,
                                MENU_REGULAR_EXPRESSIONS,
                                MENU_MODIFICATION,
                                MENU_CHAIN,
                                MENU_CHARACTER_SEQUENCE,
                                MENU_SIMPLE_WORD_SEQUENCE,
                                MENU_DOUBLE_WORD_SEQUENCE,
                                MENU_WORD_SEQUENCE_FULL,
                                MENU_KEYBOARD_WORD_SIMPLE,
                                MENU_KEYBOARD_WORD_COMPLEX,
                                MENU_CONSONANT_VOWEL_SEQUENCE,
                                MENU_DENSITY,

                                MENU_TWO_DICTIONARIES_SIMPLE,
                                MENU_TWO_DICTIONARIES_TRANSLIT,
                                MENU_TWO_DICTIONARIES_KEYBOARD_WORD,

                                MENU_DICTIONARY_STATISTICS,
                                MENU_WORD_FREQUENCY,
                                MENU_CHECK_DICTIONARY,
                                MENU_TWO_CHARACTERS_DISTRIBUTION};
#endif

WordsBase *wordsBase;

WordsBase::WordsBase() {
  int i, j;
  wordsBase = this;
#ifndef USE_STANDARD_REGEX
  for (auto &a : m_regex) {
    a = nullptr;
  }
#endif

  for (i = 0; i < LANGUAGES; i++) {
    LNG[i] = LANGUAGE[i].substr(0, 2);
    m_settings[i] = readFile(i, "settings");
#ifndef NOGTK
    m_template[i] = readFile(i, "template");
    assert(m_template[i].size() == SIZE(TEMPLATE_MENU));
#endif

    // load dictionaries
    m_longestWordLength[i] = 0;
    std::ifstream file(path(i, "words"));
    assert(file.is_open());
    std::string line;
    while (std::getline(file, line)) {
      m_dictionary[i].insert(line);
      j = line.size();
      if (j > m_longestWordLength[i]) {
        m_longestWordLength[i] = j;
      }
    }
    file.close();
  }

  int num_threads = g_get_num_processors(); // std::hardware_concurrency();
  m_thread_result.resize(num_threads);
  for (i = 0; i < LANGUAGES; i++) {
    // auto begin = clock();
    StringSet const &main_set = m_dictionary[i];
    size_t total_size = main_set.size();
    size_t chunk_size = total_size / num_threads;
    size_t remainder = total_size % num_threads;
    auto current_it = main_set.begin();

    for (j = 0; j < num_threads; ++j) {
      size_t current_chunk =
          chunk_size + (j == num_threads - 1 ? remainder : 0);
      if (current_chunk == 0)
        break;

      m_it[i].push_back(current_it);
      current_it = std::next(current_it, current_chunk);
    }
    // pr(current_it==main_set.end());
    m_it[i].push_back(current_it);

    // m_it[i].size()=threads+1
    // pr(total_size, timeElapse(begin), m_it[i].size());
  }

  // test();
#ifdef NOGTK
  // cgi();TODO uncomment on real cgi query, and comment next lines

  // setDictionaryIndex(1);

  // m_result.clear();
  // twoDictionaries(0);

  // m_result.clear();
  // twoDictionaries(1);
  // count longest constants needs when dictinary changed
  // checkLFAllFiles();
  // system("chcp 1251>nul");
  // showLongestAnagram();
  // showLongestPangram();
  // showLongestSimpleWordSequence();
  // showLongestDoubleWordSequence();
#endif
}

void WordsBase::test() {}

WordsBase::~WordsBase() {
#ifndef NOGTK
  for (int i = 0; i < SIZEI(m_regex); i++) {
    freeRegex(i);
  }
#endif
}

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
    if (s.length() < m_entryText.length()) {
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
    if (s.length() > m_entryText.length()) {
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
      if (!k || ((j += k) >= m_entryText.length() && i + 1 < s.length())) {
        return false;
      }
    }
    return true;
  } else {
    return spanIncluding(s, m_entryText);
  }
}

bool WordsBase::checkCharacterSequence(const std::string &s) {
  auto j = m_comboValue[COMBOBOX_HELPER2];
  if (j == 0) {
    auto min = m_comboValue[COMBOBOX_HELPER0];
    auto max = m_comboValue[COMBOBOX_HELPER1];
    j = 0;
    std::string::size_type pos = 0;
    while ((pos = s.find(m_entryText, pos)) != std::string::npos) {
      if (!m_radioValue) {
        return true;
      }
      j++;
      if (j > max) {
        return false;
      }
      pos += m_entryText.length();
    }
    return j >= min;
  } else if (j == 1) {
    return s.compare(0, m_entryText.length(), m_entryText) == 0;
  } else {
    unsigned i = s.length();
    if (i < m_entryText.length()) {
      return false;
    }
    return s.compare(i - m_entryText.length(), m_entryText.length(),
                     m_entryText) == 0;
  }
}

bool WordsBase::checkCrossword(const std::string &s) {
  unsigned i;
  if (s.length() != m_entryText.length()) {
    return false;
  }
  for (i = 0; i < m_entryText.length(); ++i) {
    if (m_entryText[i] != '*' && m_entryText[i] != s[i]) {
      return false;
    }
  }
  return true;
}

bool WordsBase::checkConsonantVowelSequence(const std::string &s) {
  const int searchType = m_comboValue[COMBOBOX_HELPER0];
  const int n = m_comboValue[COMBOBOX_HELPER1];
  const char *q =
      getSettings(m_comboValue[COMBOBOX_HELPER2] == 0 ? SETTINGS_VOWELS
                                                      : SETTINGS_CONSONANTS)
          .c_str();
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
  const char *q =
      m_settings[getDictionaryIndex()]
                [m_comboValue[COMBOBOX_HELPER1] == 0 ? SETTINGS_VOWELS
                                                     : SETTINGS_CONSONANTS]
                    .c_str();
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

bool WordsBase::checkRegularExpression(const std::string &s) {
#ifdef USE_STANDARD_REGEX
  std::ptrdiff_t const matches(
      std::distance(std::sregex_iterator(s.begin(), s.end(), m_regex),
                    std::sregex_iterator()));
  return matches >= m_comboValue[COMBOBOX_HELPER0] &&
         matches <= m_comboValue[COMBOBOX_HELPER1];
#else
  if (!m_radioValue) {
    return g_regex_match(m_regex[0], s.c_str(), GRegexMatchFlags(0), NULL);
  }
  GMatchInfo *matchInfo;
  g_regex_match(m_regex[0], s.c_str(), GRegexMatchFlags(0), &matchInfo);
  int i;
  const int max = m_comboValue[COMBOBOX_HELPER1];
  for (i = 0; g_match_info_matches(matchInfo) && i <= max; i++) {
    g_match_info_next(matchInfo, NULL);
  }
  g_match_info_free(matchInfo);

  return i >= m_comboValue[COMBOBOX_HELPER0] && i <= max;
#endif
}

void WordsBase::setKeyboardOneRow() {
  int i, k;
  size_t j;
  uchar uc;
  for (i = 0; i < KEYBOARD_ROW_SIZE; i++) {
    std::string const &rs = getSettings(KEYBOARD_ROW[i]);
    for (j = 0; j < rs.length(); j++) {
      uc = rs[j];

      m_keyboardOneRow[uc][0] = rs;
      for (k = 0; k < KEYBOARD_ROW_SIZE; k++) {
        if (getSettings(KEYBOARD_ROW[k]).length() > j) {
          m_keyboardOneRow[uc][1] += getSettings(KEYBOARD_ROW[k])[j];
        }
      }
    }
  }
}

void WordsBase::setKeyboardRowDiagonals() {
  int i, k, l;
  size_t j;
  uchar uc;
  char c;
  const int ROW_LEN = getSettings(SETTINGS_KEYBOARD_ROW1).length() + 1;
  const int SZ = ROW_LEN * 5;
  std::vector<char> pu(SZ, 0);
  for (i = 0; i < KEYBOARD_ROW_SIZE; i++) {
    std::string const &rs = getSettings(KEYBOARD_ROW[i]);
    for (j = 0; j < rs.length(); j++) {
      pu[(i + 1) * ROW_LEN + (j + 1)] = rs[j];
    }
  }

  for (i = 0; i < KEYBOARD_ROW_SIZE; i++) {
    std::string const &rs = getSettings(KEYBOARD_ROW[i]);
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
  auto v = {"cgi_language", "language", "settings", "template"};

  auto checkFile = [this](const std::string &filepath) {
    std::string s = readBinaryFileToString(filepath);
    if (s.contains('\r')) {
      pr("file " + filepath + " has CR symbol");
    }
  };

  for (int n = 0; n < 2; n++) {
    for (const auto &name : v) {
      checkFile(path(n, name));
    }
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

// like fillResultFromMap
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
  MapStringTwoStringVectorsI mit;
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
          if ((mit = map.find(t)) == map.end()) {
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
    for (auto const &e : getDictionary()) {
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
      m_result.push_back(SearchResult(s, v.begin()->length(), v.size()));
    }
    map.clear();
    RETURN_ON_USER_BREAK
  }
}

void WordsBase::findSimpleWordSequence(int nthread) {
  int i, j;
  std::string s;
  MapStringTwoStringVectors map;
  MapStringTwoStringVectorsI mit;
  for (i = m_comboValue[COMBOBOX_HELPER0]; i <= m_comboValue[COMBOBOX_HELPER1];
       i++) {
    for (auto const &e : getDictionary()) {
      if (int(e.length()) >= i) {
        for (j = 0; j < 2; j++) {
          s = j ? e.substr(0, i) : e.substr(e.length() - i);
          if ((mit = map.find(s)) == map.end()) {
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

    fillResultFromMap(map, i);
    map.clear();
  }
}

void WordsBase::findDoubleWordSequence(int nthread) {
  int i, j;
  std::string s, t, q;
  MapStringTwoStringVectors map;
  MapStringTwoStringVectorsI mit;

  for (i = m_comboValue[COMBOBOX_HELPER0]; i <= m_comboValue[COMBOBOX_HELPER1];
       i++) {
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
        if ((mit = map.find(t)) == map.end()) {
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

      RETURN_ON_USER_BREAK
    }

    fillResultFromMap(map, i);
    map.clear();
  }
}

void WordsBase::findWordSequenceFull(int nthread) {
  std::string s;
  for (auto const &e : getDictionary()) {
    if (checkPalindrome(e)) {
      continue;
    }
    s = e;
    std::reverse(s.begin(), s.end());
    if (s > e && getDictionary().contains(s)) {
      m_result.push_back(SearchResult(e + " " + s, e.length(), 2));
    }
  }
}

void WordsBase::findModification(int nthread) {
  std::string s;
  for (auto const &e : getDictionary()) {
    s = m_modifications.apply(e, m_checkValue);
    if (!s.empty() && s != e && getDictionary().contains(s)) {
      m_result.push_back(SearchResult(e + " " + s, e.length(), 1));
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
  StringSet const &r = getDictionary();

  if (differenceOnlyOneChar(m_chainHelper[0], m_chainHelper[1])) {
    m_out = localeToUtf8(m_chainHelper[0] + " " + m_chainHelper[1]) + OPEN_S +
            m_language[WORDS] + "2)";
    return;
  }

  for (auto &e : r) {
    if (e.length() == m_chainHelper[0].length()) {
      dl[e] = 0;
    }
  }

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
          for (auto &ia : getAlphabet()) {
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
    m_out = m_language[NO_CHAINS_FOUND];
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
}

void WordsBase::findLetterGroupSplit(int nthread) {
  std::string s, s1, t, lng;
  size_t i, j;
  StringSet const &r = getDictionary();
  auto charset = getOrderedString(m_entryText);

  const size_t size = m_entryText.length();
  eqmap.clear();
  eqmap.resize(size);

  for (auto &s : r) {
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

  auto v = getAllPairs(charset);
  size_t n[] = {v.size(), 0};

  if (!v.empty()) {
    m_out = localeToUtf8(pairsToString(v)) + "----------------\n";
  }

  for (i = 1; i < size; i++) {
    auto &m = eqmap[i];
    for (auto &e : m) {
      t = sub(charset, e.first);
      if (t != invalidDifference) {
        auto v = getAllPairs(t, e.first);
        if (!v.empty()) {
          n[1]++;
          m_out += localeToUtf8(getUserString(e.first) + " " +
                                pairsToString(v, v.size() != 1));
        }
      }
    }
  }
  if (m_out.empty()) {
    m_out = m_language[SPLITS_NOT_FOUND];
  } else {
    for (i = 0; i < 2; i++) {
      m_addstatus += m_language[i ? TRIPLETS : PAIRS] + " " +
                     intToStringLocaled(n[i]) + (i ? "" : ", ");
    }
  }
}

void WordsBase::fillResultFromMap(const MapStringTwoStringVectors &map,
                                  size_t len) {
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
      m_result.push_back(SearchResult(s, v0.begin()->length(), i + j));
    }
  }
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
  clock_t begin = clock();
  const int di = getDictionaryIndex();
  s = getTwoDictionariesPath(translit);
  for (auto &s : readFile(s)) {
    if (to.empty()) {
      fromIndex = INDEX_OF(s, LNG);
      assert(fromIndex != -1);
      alphabetFrom = m_settings[fromIndex][SETTINGS_ALPHABET];
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

  StringSet const &dt = m_dictionary[fromIndex == 1 ? 0 : 1];

  i = m_longestWordLength[fromIndex];
  VVString k(i);
  IntVector id(i);

  auto z = m_it[fromIndex];
  for (auto it = z[nthread]; it != z[nthread + 1]; it++) {
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
      if (dt.contains(s)) {
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

  pr(nthread, timeElapse(begin), m_thread_result[nthread].size());
}

void WordsBase::keyboardWords(int nthread) {
  StringSet const &df = m_dictionary[0];
  StringSet const &dt = m_dictionary[1];
  int i;
  char a[256] = {0}, b[128], *p;
  std::string s;
  std::string::size_type j, len;
  const int di = getDictionaryIndex();

  assert(SETTINGS_KEYBOARD_ROW2 == SETTINGS_KEYBOARD_ROW1 + 1);
  assert(SETTINGS_KEYBOARD_ROW3 == SETTINGS_KEYBOARD_ROW1 + 2);

  for (i = SETTINGS_KEYBOARD_ROW1; i <= SETTINGS_KEYBOARD_ROW3; i++) {
    s = m_settings[0][i];
    assert(m_settings[1][i].length() >= s.length());
    for (j = 0; j < s.length(); j++) {
      // all english keys have russian char on the same keyboard key
      a[uchar(s[j])] = m_settings[1][i][j];
    }
  }

  for (auto &e : df) {
    p = b;
    len = e.length();

    for (j = 0; j < len; j++) {
      *p++ = a[uchar(e[j])];
    }
    *p = 0;

    if (dt.contains(b)) {
      /* fixed 4.3 first word should be in current dictionary language,
       * for correct sorting vowels/consonant percent*/
      if (di == 0) {
        s = e + " " + b;
      } else {
        s = b + (" " + e);
      }
      m_result.push_back(SearchResult(s, len, 1));
    }
  }
}

void WordsBase::wordFrequency(int nthread) {
  int i;
  const int MAX = getMaximumWordLength();
  IntVector m(MAX, 0);
  StringSet const &r = getDictionary();
  IntIntVector v;

  for (auto &e : r) {
    m[e.length() - 1]++; // use length-1
  }

  for (i = 0; i < MAX; ++i) {
    if (m[i] > 0) {
      v.push_back({m[i], i + 1}); // store actual word length
    }
  }

  m_out = m_language[WORD_LENGTH_FREQUENCY];

  std::sort(v.begin(), v.end(), sortIntInt);

  for (auto &e : v) {
    // use separator for intToString for understandable view
    m_out +=
        format("\n%2d %6.3lf%% %6s/%s", e.second, 100. * e.first / r.size(),
               toString(e.first, ',').c_str(), toString(r.size(), ',').c_str());
  }
}

void WordsBase::checkDictionary(int nthread) {
  // NOTE!!! SHOULD CHECK DICTIONARY FILE NOT!!! DICTIONARY SET. Check for
  // duplicates and valid line numbers
  std::string s, p = path(getDictionaryIndex(), "words");
  int line = 0, errors = 0;
  char buffer[256];
  const int MAX_ERRORS = 100;

  auto addError = [&line, &errors, this](ENUM_STRING error) {
    m_out += (m_out.empty() ? "" : "\n") + m_language[STRING_ERROR] + ", " +
             m_language[error] + " " + m_language[LINE] + " " +
             intToStringLocaled(line);
    if (++errors == MAX_ERRORS) {
      throw std::runtime_error("");
    }
  };

  FILE *file = std::fopen(p.c_str(), "rb");
  if (!file) {
    m_out = m_language[STRING_ERROR];
    return;
  }
  try {
    while (std::fgets(buffer, sizeof(buffer), file) != nullptr) {
      line++;
      std::string a(buffer);
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
            if (spanIncluding(a, getAlphabet())) {
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
    }
    std::fclose(file);
    if (m_out.empty()) {
      m_out = m_language[DICTIONARY_CHECK_FINISHED_SUCCESSFULLY];
    }
  } catch (const std::runtime_error &) {
    std::fclose(file);
  }
}

void WordsBase::twoCharactersDistribution(int nthread) {
  int i, j;
  const int s = getAlphabetSize();
  auto a = create2dArray<int>(s, s, 0);
  std::string ss;
  StringIntVectorCI p;

  int total = 0;
  for (auto const &e : getDictionary()) {
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

  StringIntVector v;
  for (i = 0; i < s; i++) {
    for (j = 0; j < s; j++) {
      if (a[i][j] != 0) {
        ss = getAlphabetChar(i);
        ss += getAlphabetChar(j);
        v.push_back({ss, a[i][j]});
      }
    }
  }
  std::sort(v.begin(), v.end(), sortStringInt);
  p = v.begin();
  i = format("%.2f", (p->second * 100.) / total).length();
  j = intToStringLocaled(p->second).length();
  ss = "%s %" + std::to_string(i) + ".2f%% = %" + std::to_string(j) + "s / %s";
  for (p = v.begin(); p != v.end(); p++) {
    if (p != v.begin()) {
      m_out += "\n";
    }
    m_out += localeToUtf8(format(ss.c_str(), p->first.c_str(),
                                 (p->second * 100.) / total,
                                 intToStringLocaled(p->second).c_str(),
                                 intToStringLocaled(total).c_str()));
  }
  m_addstatus = m_language[PAIRS] + " " + intToStringLocaled(v.size());
}

void WordsBase::dictionaryStatistics(int nthread) {
  int i, j;
  unsigned k;
  double v;
  const int SZ_CAPTION = 3;
  std::string caption[SZ_CAPTION];
  std::string additionalCaption[2];
  std::string longestWord;
  const int a = getAlphabetSize();
  auto m = create2dArray<int>(SZ_CAPTION, a + 1, 0);
  StringSet const &r = getDictionary();

  for (i = 0; i < SZ_CAPTION; i++) {
    caption[i] =
        m_language[i == 0
                       ? CHARACTER_FREQUENCY_PERCENTS
                       : (i == 1 ? CHARACTER_FREQUENCY_AT_THE_BEGINNING_PERCENTS
                                 : CHARACTER_FREQUENCY_AT_THE_END_PERCENTS)];
  }

  for (i = 0; i < SIZEI(additionalCaption); i++) {
    additionalCaption[i] =
        " " + m_language[i == 0 ? SORTED_BY_ALPHABET : SORTED_BY_FREQUENCY];
  }

  m[1][a] = m[2][a] = r.size();

  for (auto &e : r) {
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

  std::string s, s2;
  m_out = m_language[PROCEED_SYMBOLS] + " " + intToStringLocaled(m[0][a]) +
          ", " + m_language[WORDS] + " " + intToStringLocaled(m[1][a]);

  m_out += "\n" + m_language[AVERAGE_WORD_LENGTH_EQUALS] +
           format(" %.2lf", (double(m[0][a])) / m[1][a]);

  IntDouble ve[a];
  const int COLUMNS = a / 3;
  std::vector<double> frequency(getAlphabetSize());
  const char FORMAT[] = "%c %6.3lf  ";

  for (j = 0; j < SZ_CAPTION; j++) {
    m_out += "\n\n" + caption[j] + additionalCaption[0];

    for (s = "", i = 0; i < a; ++i) {
      v = 100. * m[j][i] / m[j][a];
      s += localeToUtf8(format(FORMAT, getAlphabetChar(i), v));

      if ((i + 1) % COLUMNS == 0 || i == a - 1) {
        m_out += "\n" + s;
        s = "";
      }

      ve[i] = {i, v};
      frequency[i] = v;
    }

    m_out += "\n\n" + caption[j] + additionalCaption[1];

    std::sort(ve, ve + a, sortIntDouble);

    s = "";
    for (i = 0; i < a; i++) {
      s += localeToUtf8(
          format(FORMAT, getAlphabetChar(ve[i].first), ve[i].second));

      if ((i + 1) % COLUMNS == 0 || i == a - 1) {
        m_out += "\n" + s;
        s = "";
      }
    }

    if (j == 0) {
      m_out += "\n\n" + m_language[FREQUENCY_OF_KEYBOARD_CHARACTERS];
      for (i = 0; i < 3; i++) {
        s = "";
        s2 = m_settings[getDictionaryIndex()][SETTINGS_KEYBOARD_ROW1 + i];
        for (k = 0; k < s2.length(); k++) {
          s += localeToUtf8(
              format(FORMAT, s2[k], frequency[alphabetIndex(s2[k])]));
        }
        m_out += "\n" + s;
      }
    }
  }

  m_out += "\n\n" + m_language[THE_LONGEST_WORD_IS] +
           format(" - %s (%s %d).", localeToUtf8(longestWord).c_str(),
                  m_language[LENGTH].c_str(), longestWord.length());
}

void WordsBase::sortFilterResults() {
  int i;
  std::string s;

#ifndef NOGTK
  bool find = m_comboValue[COMBOBOX_FILTER] == 0;
#endif

  m_out.clear();
  if (!m_outSplitted) {
    std::sort(m_result.begin(), m_result.end(),
              SORT_FUNCTION[m_comboValue[COMBOBOX_SORT] * 2 +
                            m_comboValue[COMBOBOX_SORT_ORDER]]);
  }
  for (auto const &e : m_result) {
    s = localeToUtf8(e.s);
#ifndef NOGTK
    if (find != testFilterRegex(s)) {
      continue;
    }
    m_filteredWordsCount++;
#endif
    if (!m_out.empty()) {
      m_out += "\n";
    }

    m_out += s;
    if (!m_outSplitted) {
      m_out += OPEN_S + m_language[CHARACTERS] + format(" %d", e.length);

      if (e.words > 1) {
        m_out += format(" %s %d", m_language[WORDS].c_str(), e.words);
      }

      i = m_comboValue[COMBOBOX_SORT] - (NUMBER_OF_SORTS - 3);
      if (i == 0 || i == 1) { // sort by vowels, consonants
        m_out += " " + m_language[VOWELS + i] +
                 format(" %.1lf%%",
                        e.percent[i]); // show percent of vowels or consonants
      } else if (i == 2) {
        m_out += " " + m_language[DIFFERENT_CHARACTERS] +
                 format(" %d", e.differentCharacters);
      }

      m_out += ")";
    }
    RETURN_ON_USER_BREAK1
  }
}

void WordsBase::loadLanguage() {
  std::string s;
  int i = 0;
  bool b = false;
  auto v = readFile(m_languageIndex, "language");
  m_language.clear();
  for (auto const &e : v) {
    if (b) {
      m_language.push_back(localeToUtf8(e));
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
#ifndef NOGTK
    setMenuLabel(ENUM_MENU(i), s);
#endif
    // first item
    if (!i) {
      m_language.push_back(utf8ToLowerCase(s));
    }
    i++;
  }

  assert(m_language.size() == STRING_SIZE);

#ifdef NOGTK
  for (auto &s : readFile(m_languageIndex, "cgi_language"))
    m_cgiLanguage.push_back(localeToUtf8(s));
  assert(m_cgiLanguage.size() == CGI_STRING_SIZE);
#else
  m_programVersion =
      m_language[PROGRAM] + " " + m_language[VERSION] + " " + WORDS_VERSION;
#endif
  m_language[MODIFICATION_HELP] =
      format(m_language[MODIFICATION_HELP].c_str(),
             m_language[EVERY_MODIFICATION_CHANGES_WORD].c_str());
  /* use only first symbol. In Russian language separator is space, so ignore
   * comment in language.txt file. Comment in ru/language.txt is important
   * because otherwise it'll we empty string and looks like a bug
   */
  m_language[SEPARATOR_SYMBOL] = m_language[SEPARATOR_SYMBOL].substr(0, 1);
}

bool WordsBase::prepare() {
  std::string s;
  int i;
  std::string::size_type pb, pe;

  if (m_menuClick == MENU_KEYBOARD_WORD_SIMPLE) {
    setKeyboardOneRow();
    return true;
  } else if (m_menuClick == MENU_KEYBOARD_WORD_COMPLEX) {
    setKeyboardRowDiagonals();
    return true;
  }

  if (!ONE_OF(m_menuClick, TEMPLATE_MENU)) {
    return true;
  }

  // MENU_REGULAR_EXPRESSIONS MENU_MODIFICATION MENU_CHAIN MENU_TEMPLATE
  // MENU_CROSSWORD MENU_CHARACTER_SEQUENCE
  if (m_entryText.empty()) {
    return false;
  }

  if (m_menuClick ==
      MENU_REGULAR_EXPRESSIONS) { // finish with MENU_REGULAR_EXPRESSIONS
    // russain char to lowercase, other ignorecase options in
    // regcomp/g_regex_new functions
    m_entryText = localeToLowerCase(m_entryText, true);
#ifdef USE_STANDARD_REGEX
    try {
      m_regex =
          std::regex(m_entryText.c_str(), std::regex_constants::icase |
                                              std::regex_constants::extended);
    } catch (std::regex_error &) {
      return false;
    }
#else
    // Note G_REGEX_RAW support 's' in locale, otherwise 's' should be a utf8
    // string
    freeRegex(0);
    m_regex[0] = g_regex_new(m_entryText.c_str(),
                             GRegexCompileFlags(G_REGEX_RAW | G_REGEX_CASELESS),
                             GRegexMatchFlags(0), NULL);
    if (!m_regex[0]) {
      return false;
    }
#endif

    return true;
  }

  // MENU_MODIFICATION MENU_CHAIN MENU_TEMPLATE MENU_CROSSWORD
  // MENU_CHARACTER_SEQUENCE
  m_entryText = localeToLowerCase(m_entryText);

  if (m_menuClick == MENU_MODIFICATION) { // finish with MENU_MODIFICATION
    if (!m_modifications.parse(m_entryText)) {
#ifdef NOGTK
      printf(
          m_cgiLanguage[CGI_STRING_ERROR_INVALID_MODIFICATION_STRING].c_str());
#endif
      return false;
    }
    return true;
  }

  // MENU_CHAIN MENU_TEMPLATE MENU_CROSSWORD MENU_CHARACTER_SEQUENCE
  s = getAlphabet();
  if (m_menuClick == MENU_CROSSWORD) {
    s += '*';
  } else if (m_menuClick == MENU_CHAIN) {
    s += ' ';
  }
  if (!spanIncluding(m_entryText, s)) {
    return false;
  }

  if (m_menuClick == MENU_TEMPLATE) {
    memset(m_templateHelper, 0, 256);
    const char *p;
    for (p = m_entryText.c_str(); *p != 0; p++) {
      m_templateHelper[uchar(*p)]++;
    }

    size_t i, j;
    m_template_a = create2dArray<char>(m_entryText.length(), 256, 0);
    j = 0;
    for (auto &a : m_template_a) {
      s = m_entryText.substr(j);
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
    s = m_entryText;
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
  if (!m_outSplitted) {
    s = m_language[NUMBER_OF_WORDS] + " " +
        intToStringLocaled(m_result.size()) + ", ";
#ifndef NOGTK
    s += m_language[WITH_FILTER] + " " +
         intToStringLocaled(m_filteredWordsCount) + ", ";
#endif
  }
  return s + m_language[TIME_OF_LAST_OPERATION] + " " + getTimeString();
}

std::string WordsBase::getTimeString() {
  return format("%.2lf", double(m_end - m_begin) / CLOCKS_PER_SEC);
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
    auto z = m_it[getDictionaryIndex()];
    for (auto it2 = z[nthread]; it2 != z[nthread + 1]; it2++) {
      auto &e = *it2;
      if ((this->*f)(e)) {
        m_thread_result[nthread].push_back(SearchResult(e, e.length(), 1));
      }
      RETURN_ON_USER_BREAK
    }
  }

  //  auto it1 = menu2BoolString.find(m_menuClick);
  // if (it1 != menu2BoolString.end()) {
  //   auto f = it1->second;
  //   for (auto &e : getDictionary()) {
  //     if ((this->*f)(e)) {
  //       m_result.push_back(SearchResult(e, e.length(), 1));
  //     }
  //     RETURN_ON_USER_BREAK
  //   }
  // }

  pr(nthread, timeElapse(begin),m_thread_result[nthread].size());
}

void WordsBase::run() {
  std::vector<std::jthread> workers;
  const int threads = 
  oneOf(m_menuClick, MENU_TWO_DICTIONARIES_SIMPLE,
                                MENU_TWO_DICTIONARIES_TRANSLIT) ||
                                  !menu2VoidInt.contains(m_menuClick)
                              ? g_get_num_processors()
                              : 1;
  pr(threads);
  for (int i = 0; i < threads; ++i) {
    workers.emplace_back(run_thread, this, i);
  }
  for (auto &t : workers) {
    if (t.joinable()) {
      // pri;
      t.join();
      // pri;
    }
  }

  //TODO
  if (threads > 1) {
    m_result = m_thread_result | std::views::join |
               std::ranges::to<SearchResultVector>();
  }

  bool b = m_token.stop_requested();

  if ((m_outSplitted = m_result.empty())) {
    auto v = split(m_out, "\n");
    for (auto &e : v) {
      m_result.push_back(SearchResult(utf8ToLocale(e), 0, 0));
    }
  }

  if (b) { // was user break
    //		printl("end set")
    m_end = clock();
  } else {
    sortFilterResults();
    m_end = clock();
#ifndef NOGTK
    endJobThread();
#endif
  }
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
bool WordsBase::setCheckFilterRegex() {
  if (m_filterText.empty()) {
    return true;
  }
  // need case insensitive filter, work ok in russian only for utf8
  auto s = localeToUtf8(m_filterText);
  freeRegex(1);
  m_regex[1] = g_regex_new(s.c_str(), GRegexCompileFlags(G_REGEX_CASELESS),
                           GRegexMatchFlags(0), NULL);
  return m_regex[1] != nullptr;
}

bool WordsBase::testFilterRegex(const std::string &s) {
  return m_regex[1] == nullptr || m_filterText.empty() ||
         g_regex_match(m_regex[1], s.c_str(), GRegexMatchFlags(0), NULL);
}
#endif

std::string WordsBase::intToStringLocaled(int v) {
  return toString(v, m_language[SEPARATOR_SYMBOL][0]);
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
      m_entryText = utf8ToLocale(s);
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

  loadLanguage(); // prepare can output error message so load before

  if (!prepare()) {
    printf(" %s", getTimeString().c_str());
    return;
  }

  run();
  printf("%s\n%s", getStatusString().c_str(), m_out.c_str());
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
  m_comboValue[COMBOBOX_DICTIONARY] = i;
}

int WordsBase::getDictionaryIndex() const {
  assert(m_comboValue[COMBOBOX_DICTIONARY] >= 0 &&
         m_comboValue[COMBOBOX_DICTIONARY] < LANGUAGES);
  return m_comboValue[COMBOBOX_DICTIONARY];
}

#ifndef USE_STANDARD_REGEX
void WordsBase::freeRegex(int i) {
  if (m_regex[i]) {
    g_regex_unref(m_regex[i]);
  }
}
#endif
