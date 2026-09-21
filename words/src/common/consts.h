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

const ENUM_STRING KEYBOARD_ROW[] = {
    SETTINGS_KEYBOARD_ROW1,
    SETTINGS_KEYBOARD_ROW2, // should goes immediately after
                            // SETTINGS_KEYBOARD_ROW1
    SETTINGS_KEYBOARD_ROW3  // should goes immediately after
                            // SETTINGS_KEYBOARD_ROW2
};
static_assert(SETTINGS_KEYBOARD_ROW2 == SETTINGS_KEYBOARD_ROW1 + 1);
static_assert(SETTINGS_KEYBOARD_ROW3 == SETTINGS_KEYBOARD_ROW1 + 2);
const int KEYBOARD_ROW_SIZE = SIZE(KEYBOARD_ROW);

#ifndef NOGTK

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
