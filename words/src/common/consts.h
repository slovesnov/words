/*
 * consts.h
 *
 *  Created on: 18.12.2016
 *      Author: alexey slovesnov
 */

#pragma once

#include "WordsBase.h"
#ifndef NOGTK
#include <gtk/gtk.h> //for GDK_KEY_...
#endif

// need for cgi & gtk
const std::string WORDS_VERSION = "4.5.0";

const char OPEN_BRACKET = '(';
const std::string OPEN_S = std::string(" ") + OPEN_BRACKET;

const ENUM_MENU TEMPLATE_MENU[] = {
    MENU_TEMPLATE,          MENU_CROSSWORD, MENU_REGULAR_EXPRESSIONS,
    MENU_MODIFICATION,      MENU_CHAIN,     MENU_CHARACTER_SEQUENCE,
    MENU_LETTER_GROUP_SPLIT};

const ENUM_SETTINGS KEYBOARD_ROW[] = {
    SETTINGS_KEYBOARD_ROW1,
    SETTINGS_KEYBOARD_ROW2, // should goes immediately after
                            // SETTINGS_KEYBOARD_ROW1
    SETTINGS_KEYBOARD_ROW3  // should goes immediately after
                            // SETTINGS_KEYBOARD_ROW2
};
const int KEYBOARD_ROW_SIZE = SIZE(KEYBOARD_ROW);

#ifndef NOGTK

const ENUM_MENU MENU_ACCEL[] = {MENU_EDIT_SELECT_ALL_AND_COPY_TO_CLIPBOARD,
                                MENU_EDIT_SELECT_ALL,
                                MENU_EDIT_COPY_TO_CLIPBOARD};
const int MENU_ACCEL_SIZE = SIZE(MENU_ACCEL);
const int ACCEL_KEY[] = {GDK_KEY_B, GDK_KEY_A, GDK_KEY_C};

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
