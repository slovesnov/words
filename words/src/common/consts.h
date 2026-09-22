/*
 * consts.h
 *
 *  Created on: 18.12.2016
 *      Author: alexey slovesnov
 */

#pragma once

// gtk & cgi
const std::string WORDS_VERSION = "4.5.0";

const char OPEN_BRACKET = '(';
const std::string OPEN_S = std::string(" ") + OPEN_BRACKET;
const int KEYBOARD_ROWS = 3;
const int SETTINGS_FIRST = SETTINGS_ALPHABET;
const int SETTINGS_LAST = SETTINGS_KEYBOARD_ROW3;
const int SETTINGS_SIZE = SETTINGS_LAST - SETTINGS_FIRST + 1;

#ifdef NOGTK
// should match with ENUM_POST
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
#else
const ENUM_COMBOBOX HELPER_COMBOBOX[] = {COMBOBOX_HELPER0, COMBOBOX_HELPER1,
                                         COMBOBOX_HELPER2};

const ENUM_MENU MENU_ADJUST_COMBO[] = {
    MENU_ANAGRAM, MENU_REGULAR_EXPRESSIONS, MENU_CHARACTER_SEQUENCE,
    MENU_SIMPLE_WORD_SEQUENCE, MENU_DOUBLE_WORD_SEQUENCE};

const ENUM_MENU MENU_WAITING[] = {MENU_DICTIONARY_STATISTICS,
                                  MENU_WORD_FREQUENCY,
                                  MENU_CHECK_DICTIONARY,
                                  MENU_TWO_CHARACTERS_DISTRIBUTION,
                                  MENU_TWO_CHARACTERS_DISTRIBUTION_START,
                                  MENU_TWO_CHARACTERS_DISTRIBUTION_END};

#endif /*#ifndef NOGTK*/
