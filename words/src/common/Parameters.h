/*
 * Parameters.h
 *
 *  Created on: 04.11.2026
 *      Author: alexey slovesnov
 */

#pragma once

#include "consts.h"
#include <stop_token>
#define PCRE2_CODE_UNIT_WIDTH 8
#define PCRE2_STATIC
#include <pcre2.h>
using UniquePcre2Code = UniqueGtkResource<pcre2_code, pcre2_code_free>;
using UniquePcre2MatchData =
    UniqueGtkResource<pcre2_match_data, pcre2_match_data_free>;
#include "Modification.h"

class Parameters {
private:
  // only parameters
  VString m_tvwords; // locale
protected:

  // no Frame
  SetString m_ss;
  std::string m_ev; // locale m_ev=utf8ToLocale(m_entryValue[ENTRY_TEMPLATE])
  std::vector<std::vector<char>> m_template_a;
  std::array<char, 256> m_templateHelper;

  // Frame+WordsBase
  std::array<int, COMBOBOX_SIZE> m_comboValue;
  int m_sortlistValue;
  std::array<std::string, ENTRY_SIZE> m_entryValue; // utf8
  int m_radioValue;
  std::array<int, BUTTON_SIZE>
      m_buttonValue; // use array allow assign all values
  bool m_checkValue;
  VString m_evwords; // locale
  int m_filteredWordsCount;
  std::string m_addstatus;
  ENUM_MENU m_menuClick; // last search option
  VectorSearchResult m_result;
  std::string m_textViewValueUtf8; // do splitr so need utf8
  clock_t m_begin, m_end;

  Modification m_modifications;// no Frame
  UniquePcre2Code m_regex[2];
  UniquePcre2MatchData m_match[2];

  // separated
  std::mutex m_mutex;
  std::stop_token m_token; // not need copy?

  static std::array<std::string, SETTINGS_SIZE>
      m_settingsAll[LANGUAGES]; // locale
  static std::array<std::string, STRING_SIZE> m_languageAll[LANGUAGES]; // utf8
  static std::array<std::string, MENU_SIZE> m_menuAll[LANGUAGES];       // utf8
  static MapIntInt m_wordLengthMap[LANGUAGES];
  static int m_longestWordLength[LANGUAGES];

  int getLanguageIndex() const { return m_buttonValue[BUTTON_LANGUAGE]; }
  int getDictionaryIndex() const { return m_buttonValue[BUTTON_DICTIONARY]; }

  const std::string &string(ENUM_STRING e) const;
  const std::string &string(ENUM_MENU e) const;
  const std::string string(ENUM_STRING e, ENUM_STRING e1) const;
  const std::string &string(int i) const;
  const std::string &stringUsingDictionary(ENUM_STRING e,
                                           bool opposite = false) const;
  const std::string &settings(ENUM_SETTINGS e, int i = 0) const;

  const std::string &alphabet() const;
  int alphabetSize() const;
  int alphabetIndex(char c) const;

public:
  // public for modification class
  const std::string &vowelConsonant(bool consonant);
  bool isAlphabetChar(char c) const;

  Parameters();
  Parameters(const Parameters &p);
  void operator=(const Parameters &p);

protected:
  static std::string path(int i, std::string s);
  static VString readFile(int i, std::string s);
  static VString readFile(std::string path);
  const MapIntInt &getWordLengthMap() const;

  static std::string intToString(int v, char c = ',');
  std::string intToStringLocaled(int v);

  int getMaximumWordLength();
  bool createRegex(ENUM_ENTRY e);
  bool createRegex(ENUM_ENTRY e, UniquePcre2Code &r, UniquePcre2MatchData &m);

  bool prepare();
  std::string getStatusString();
  std::string getTimeString();

};