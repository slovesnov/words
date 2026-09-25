/*
 * consts.h
 *
 *  Created on: 18.12.2016
 *      Author: alexey slovesnov
 */

#pragma once

const std::string WORDS_VERSION = "5.0.0";
const char MAIL[] = "slovesnov@yandex.ru";
const std::string URL = "https://slovesnov.rf.gd/";
const std::string SOURCE_URL = "https://github.com/slovesnov/words";
const std::string HOMEPAGE = URL + "?words";
const std::string HOMEPAGE_ONLINE = URL + "?words_online";
const char OPEN_BRACKET = '(';
const std::string OPEN_S = std::string(" ") + OPEN_BRACKET;
const int KEYBOARD_ROWS = 3;

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


    //cann't remove template types
const LookupTable<ENUM_STRING, std::string> MAP_URL = {
    {HOMEPAGE_STRING, URL},
    {HOMEPAGE_ONLINE_STRING, HOMEPAGE},
    {SOURCE_CODE, SOURCE_URL}};

    const LookupTable<ENUM_MENU, std::string> MENU_TO_ICON_FILE={
    {MENU_SEARCH, "search.png"},
    {MENU_EDIT, "edit.png"},
    {MENU_ADDITIONS, "add.png"},
    {MENU_LANGUAGE, "language.png"},
    {MENU_HELP, "help.png"},
    {MENU_EDIT_SELECT_ALL_AND_COPY_TO_CLIPBOARD, "select_all_copy.png"},
    {MENU_EDIT_SELECT_ALL, "select_all.png"},
    {MENU_EDIT_COPY_TO_CLIPBOARD, "copy.png"},
    {MENU_LOAD_ENGLISH_DICTIONARY, "en.gif"},
    {MENU_ENGLISH_LANGUAGE, "en.gif"},
    {MENU_LOAD_RUSSIAN_DICTIONARY, "ru.gif"},
    {MENU_RUSSIAN_LANGUAGE, "ru.gif"},
    {MENU_ABOUT, "word16.png"},
    {MENU_HOMEPAGE, "web.png"}};

const LookupTable<ENUM_MENU, ENUM_STRING> MENU_TO_HELP_STRING = {
    {MENU_ANAGRAM, ANAGRAM_HELP},
    {MENU_PANGRAM, PANGRAM_HELP},
    {MENU_TEMPLATE, TEMPLATE_HELP},
    {MENU_PALINDROME, PALINDROME_HELP},
    {MENU_CROSSWORD, CROSSWORD_HELP},
    {MENU_REGULAR_EXPRESSIONS, REGULAR_EXPRESSION_HELP},
    {MENU_MODIFICATION, MODIFICATION_HELP},
    {MENU_CHAIN, CHAIN_HELP},
    {MENU_CHARACTER_SEQUENCE, CHARACTERS_SEQUENCE_HELP},
    {MENU_WORDS_SPLIT, WORDS_SPLIT_HELP},
    {MENU_LETTER_GROUP_SPLIT, LETTER_GROUP_SPLIT_HELP},
    {MENU_SIMPLE_WORD_SEQUENCE, WORD_SEQUENCE_HELP},
    {MENU_DOUBLE_WORD_SEQUENCE, DOUBLE_WORD_SEQUENCE_HELP},
    {MENU_WORD_SEQUENCE_FULL, WORD_SEQUENCE_FULL_HELP},
    {MENU_KEYBOARD_WORD_SIMPLE, KEYBOARD_WORD_SIMPLE_HELP},
    {MENU_KEYBOARD_WORD_COMPLEX, KEYBOARD_WORD_DIAGONAL_HELP},
    {MENU_CONSONANT_VOWEL_SEQUENCE, CONSONANT_VOWEL_CHARACTER_SEQUENCE_HELP},
    {MENU_DENSITY, DENSITY_HELP},
    {MENU_TWO_DICTIONARIES_SIMPLE, TWO_DICTIONARIES_SIMPLE_HELP},
    {MENU_TWO_DICTIONARIES_TRANSLIT, TWO_DICTIONARIES_TRANSLIT_HELP},
    {MENU_TWO_DICTIONARIES_KEYBOARD_WORD, TWO_DICTIONARIES_KEYBOARD_WORD_HELP},
    {MENU_TWO_CHARACTERS_DISTRIBUTION, TWO_CHARACTERS_DISTRIBUTION_HELP},
    {MENU_TWO_CHARACTERS_DISTRIBUTION_START, TWO_CHARACTERS_DISTRIBUTION_HELP},
    {MENU_TWO_CHARACTERS_DISTRIBUTION_END, TWO_CHARACTERS_DISTRIBUTION_HELP}};


const LookupTable<ENUM_MENU, int> MENU_TO_ACCEL_KEY = {
    {MENU_EDIT_SELECT_ALL_AND_COPY_TO_CLIPBOARD, GDK_KEY_B},
    {MENU_EDIT_SELECT_ALL, GDK_KEY_A},
    {MENU_EDIT_COPY_TO_CLIPBOARD, GDK_KEY_C}};

    const LookupTable<ENUM_MENU, ENUM_STRING> MENU_TO_SETTINGS = {
    {MENU_TEMPLATE, SETTINGS_TEMPLATE},
    {MENU_CROSSWORD, SETTINGS_CROSSWORD},
    {MENU_REGULAR_EXPRESSIONS, SETTINGS_REGULAR_EXPRESSIONS},
    {MENU_MODIFICATION, SETTINGS_MODIFICATION},
    {MENU_CHAIN, SETTINGS_CHAIN},
    {MENU_CHARACTER_SEQUENCE, SETTINGS_CHARACTER_SEQUENCE},
    {MENU_LETTER_GROUP_SPLIT, SETTINGS_LETTER_GROUP_SPLIT}};


#endif /*#ifndef NOGTK*/
