/*
 * Parameters.cpp
 *
 *  Created on: 04.11.2026
 *      Author: alexey slovesnov
 */

#include "Parameters.h"
// #include <magic_enum.hpp>

std::mutex Parameters::m_mutex;
std::stop_token Parameters::m_token;
VectorSearchResult Parameters::m_result;
std::array<std::string, SETTINGS_SIZE>
    Parameters::m_settingsAll[LANGUAGES]; // locale
std::array<std::string, STRING_SIZE>
    Parameters::m_languageAll[LANGUAGES];                            // utf8
std::array<std::string, MENU_SIZE> Parameters::m_menuAll[LANGUAGES]; // utf8
MapIntInt Parameters::m_wordLengthMap[LANGUAGES];
int Parameters::m_longestWordLength[LANGUAGES];

Parameters::Parameters() : m_modifications(this) {}

void Parameters::operator=(const Parameters &p) {
  // pr(magic_enum::enum_name(p.m_menuClick));
  m_menuClick = p.m_menuClick;
  m_comboValue = p.m_comboValue;
  m_sortlistValue = p.m_sortlistValue;
  m_entryValueUtf8 = p.m_entryValueUtf8; // utf8
  m_radioValue = p.m_radioValue;
  m_buttonValue = p.m_buttonValue; // use array allow assign all values
  m_checkValue = p.m_checkValue;
  m_textViewValueUtf8 = p.m_textViewValueUtf8; // do splitr so need utf8
  m_spinValue = p.m_spinValue;
}

void Parameters::assignToWordsBase(const Parameters &p) {
  *this = p;
  prepare();
  createRegex(ENTRY_FILTER);
}

void Parameters::assignToFrame(const Parameters &p) {
  m_filteredWordsCount = p.m_filteredWordsCount;
  m_addstatus = p.m_addstatus;
}

const std::string &Parameters::string(ENUM_STRING e) const {
  return m_languageAll[getLanguageIndex()][e];
}

const std::string Parameters::string(ENUM_STRING e, ENUM_STRING e1) const {
  return string(e) + " " + string(e1);
}

const std::string &Parameters::string(ENUM_MENU e) const {
  return m_menuAll[getLanguageIndex()][e];
}

const std::string &Parameters::string(int i) const {
  assert(i >= 0 && i < STRING_SIZE);
  return string(ENUM_STRING(i));
}

const std::string &Parameters::stringUsingDictionary(ENUM_STRING e,
                                                     bool opposite) const {
  return m_languageAll[getDictionaryIndex() ^ opposite][e];
}

const std::string &Parameters::settings(ENUM_SETTINGS e, int i) const {
  int n = int(e) + i;
  assert(n < SETTINGS_SIZE);
  return m_settingsAll[getDictionaryIndex()][ENUM_SETTINGS(n)];
}

const std::string &Parameters::alphabet() const {
  return settings(SETTINGS_ALPHABET);
}

int Parameters::alphabetSize() const { return alphabet().size(); }

int Parameters::alphabetIndex(char c) const {
  auto i = alphabet().find(c);
  return i == std::string::npos ? -1 : i;
}

const std::string &Parameters::vowelConsonant(bool consonant) {
  return settings(consonant ? SETTINGS_CONSONANTS : SETTINGS_VOWELS);
}

bool Parameters::isAlphabetChar(char c) const { return alphabetIndex(c) != -1; }

int Parameters::indexOfCluster() const {
  return indexOf(m_menuClick, MENU_CLUSTER_SIZE_STATISTICS,
                 MENU_CLUSTER_DIAMETER_STATISTICS);
}

bool Parameters::oneOfCluster() const { return indexOfCluster() != -1; }

std::string Parameters::path(int i, std::string s) {
  return getResourcePath(LNG[i] + "/" + s + ".txt");
}

VString Parameters::readFile(int i, std::string s) {
  return readFile(path(i, s));
}

VString Parameters::readFile(std::string path) {
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

const MapIntInt &Parameters::getWordLengthMap() const {
  return m_wordLengthMap[getDictionaryIndex()];
}

std::string Parameters::intToStringLocaled(int v) {
  return toString(v, string(SEPARATOR_SYMBOL)[0]);
}

int Parameters::getMaximumWordLength() {
  return m_longestWordLength[getDictionaryIndex()];
}

bool Parameters::createRegex(ENUM_ENTRY e) {
  assert(int(e) < std::size(m_regex));
  return createRegex(e, m_regex[e], m_match[e]);
}

bool Parameters::createRegex(ENUM_ENTRY e, UniquePcre2Code &r,
                             UniquePcre2MatchData &m) {
  int errorcode;
  PCRE2_SIZE erroroffset;
  r.reset(pcre2_compile((PCRE2_SPTR)m_entryValueUtf8[e].c_str(),
                        PCRE2_ZERO_TERMINATED, PCRE2_UTF, &errorcode,
                        &erroroffset, NULL));
  if (r) {
    pcre2_jit_compile(r.get(), PCRE2_JIT_COMPLETE);
    m.reset(pcre2_match_data_create_from_pattern(r.get(), NULL));
  }

  return r != nullptr;
}

bool Parameters::prepare() {
  std::string s;
  // pr(magic_enum::enum_name(m_menuClick));

  if (!MENU_WITH_ENTRY.get(m_menuClick)) {
    return true;
  }
  std::string evUtf8 = utf8Trim(m_entryValueUtf8[ENTRY_TEMPLATE]);
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
    s = m_entryValueUtf8[ENTRY_TEMPLATE];
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

  if (m_menuClick == MENU_CHAIN) {
    // evUtf8 trimmed and has only alphabet chars
    m_evwords = getLocaleWords(evUtf8);
    auto words = getLocaleWords(m_textViewValueUtf8);

    if (m_evwords.size() != 1 && m_evwords.size() != 2) {
      return false;
    }

    if (!getWordLengthMap().contains(
            m_evwords[0].size())) { // no words such length
      return false;
    }

    // filter words
    m_ss.clear();
    for (auto &a : words) {
      if (a.size() == m_evwords[0].size()) {
        m_ss.insert(a);
      }
    }

    if (std::ranges::any_of(m_evwords, [this](auto &a) {
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
