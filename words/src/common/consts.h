/*
 * consts.h
 *
 *  Created on: 18.12.2016
 *      Author: alexey slovesnov
 */

#ifndef CONSTS_H_
#define CONSTS_H_

#include "WordsBase.h"
#ifndef CGI
#include <gtk/gtk.h>//for GDK_KEY_...
#endif

/* fixed bug aslov lib (check new version) when no internet and router return some html on any page
 *
 */

//need for cgi & gtk
const double WORDS_VERSION = 4.43;

//BEGIN ENCODE ARRAYS
const ENUM_MENU FUNCTION_MENU[] = { MENU_PANGRAM, MENU_TEMPLATE,
		MENU_PALINDROME, MENU_CROSSWORD, MENU_REGULAR_EXPRESSIONS

		, MENU_CHARACTER_SEQUENCE,MENU_LETTER_GROUP_SPLIT, MENU_KEYBOARD_WORD_SIMPLE,
		MENU_KEYBOARD_WORD_COMPLEX, MENU_CONSONANT_VOWEL_SEQUENCE

		, MENU_DENSITY };
const BOOL_STRING_WORDSBASE_FUNCTION FUNCTION_ID[] = { &WordsBase::checkPangram,
		&WordsBase::checkTemplate, &WordsBase::checkPalindrome,
		&WordsBase::checkCrossword, &WordsBase::checkRegularExpression

		, &WordsBase::checkCharacterSequence,&WordsBase::checkLetterGroupSplit,
		&WordsBase::checkKeyboardWordSimple,
		&WordsBase::checkKeyboardWordComplex,
		&WordsBase::checkConsonantVowelSequence

		, &WordsBase::checkDensity };

const ENUM_MENU BOOL_VOID_MENU[] = { MENU_ANAGRAM, MENU_SIMPLE_WORD_SEQUENCE,
		MENU_DOUBLE_WORD_SEQUENCE, MENU_WORD_SEQUENCE_FULL, MENU_MODIFICATION,
		MENU_CHAIN,MENU_LETTER_GROUP_SPLIT

		, MENU_TWO_DICTIONARIES_SIMPLE, MENU_TWO_DICTIONARIES_TRANSLIT,
		MENU_TWO_DICTIONARIES_KEYBOARD_WORD

		, MENU_DICTIONARY_STATISTICS, MENU_WORD_FREQUENCY,
		MENU_CHECK_DICTIONARY, MENU_TWO_CHARACTERS_DISTRIBUTION,
		MENU_ENGLISH_LANGUAGE, MENU_RUSSIAN_LANGUAGE };
const BOOL_VOID_WORDSBASE_FUNCTION BOOL_VOID_FUNCTION[] = {
/* Note version 4.2 has russian dictionary with 190,000 words
 * version 4.3 has about 450,000 words (in russian dictionary) so times of calculations can increase
 * Also 4.3 make compiler project options changes for "optimization level" and "debug level" for gtk project, for cgi all ok
 * time measurement was for release version with NDEBUG is on
 */
&WordsBase::findAnagram //time depends on search option user break implemented
		, &WordsBase::findSimpleWordSequence //user break implemented
		, &WordsBase::findDoubleWordSequence //user break implemented
		, &WordsBase::findWordSequenceFull //version 4.3 time 0.18 no user break need
		, &WordsBase::findModification //user break implemented
		, &WordsBase::findChain //version 4.3 time 0.05 [depends on search option] no user break need
		, &WordsBase::findLetterGroupSplit

		, &WordsBase::twoDictionariesSimple //user break implemented
		, &WordsBase::twoDictionariesTranslit //user break implemented
		, &WordsBase::keyboardWords //version 4.3 time 0.08 no user break need

		, &WordsBase::dictionaryStatistics //version 4.3 time 0.15 no user break need
		, &WordsBase::wordFrequency //version 4.3 time 0.02 no user break need
		, &WordsBase::checkDictionary //version 4.3 time 0.22 no user break need
		, &WordsBase::twoCharactersDistribution //version 4.3 time 0.21 no user break need
		, &WordsBase::dummy, &WordsBase::dummy };
//END ENCODE ARRAYS

const ENUM_MENU NO_SORT_FUNCTIONS_MENU[] = { MENU_CHAIN,MENU_LETTER_GROUP_SPLIT,
		MENU_DICTIONARY_STATISTICS, MENU_WORD_FREQUENCY, MENU_CHECK_DICTIONARY,
		MENU_TWO_CHARACTERS_DISTRIBUTION };

const ENUM_MENU TEMPLATE_MENU[] = { MENU_TEMPLATE, MENU_CROSSWORD,
		MENU_REGULAR_EXPRESSIONS, MENU_MODIFICATION, MENU_CHAIN,
		MENU_CHARACTER_SEQUENCE,MENU_LETTER_GROUP_SPLIT };

const ENUM_SETTINGS KEYBOARD_ROW[] = { SETTINGS_KEYBOARD_ROW1,
		SETTINGS_KEYBOARD_ROW2, //should goes immediately after SETTINGS_KEYBOARD_ROW1
		SETTINGS_KEYBOARD_ROW3 //should goes immediately after SETTINGS_KEYBOARD_ROW2
		};
const int KEYBOARD_ROW_SIZE = SIZE(KEYBOARD_ROW);

#ifndef CGI

//BEGIN ENCODE ARRAYS
//Note encode arrays, size assert() checking in Frame::Frame()
const ENUM_MENU HELPER_MENU[] = { MENU_ANAGRAM, MENU_PANGRAM, MENU_TEMPLATE,
		MENU_PALINDROME, MENU_CROSSWORD, MENU_REGULAR_EXPRESSIONS,
		MENU_MODIFICATION, MENU_CHAIN, MENU_CHARACTER_SEQUENCE,MENU_LETTER_GROUP_SPLIT,
		MENU_SIMPLE_WORD_SEQUENCE, MENU_DOUBLE_WORD_SEQUENCE,
		MENU_WORD_SEQUENCE_FULL, MENU_KEYBOARD_WORD_SIMPLE,
		MENU_KEYBOARD_WORD_COMPLEX, MENU_CONSONANT_VOWEL_SEQUENCE, MENU_DENSITY,
		MENU_TWO_DICTIONARIES_SIMPLE, MENU_TWO_DICTIONARIES_TRANSLIT,
		MENU_TWO_DICTIONARIES_KEYBOARD_WORD, MENU_TWO_CHARACTERS_DISTRIBUTION };
const ENUM_STRING HELPER_STRING[] = { ANAGRAM_HELP, PANGRAM_HELP, TEMPLATE_HELP,
		PALINDROME_HELP, CROSSWORD_HELP, REGULAR_EXPRESSION_HELP,
		MODIFICATION_HELP, CHAIN_HELP, CHARACTERS_SEQUENCE_HELP,LETTER_GROUP_SPLIT_HELP,
		WORD_SEQUENCE_HELP, DOUBLE_WORD_SEQUENCE_HELP, WORD_SEQUENCE_FULL_HELP,
		KEYBOARD_WORD_SIMPLE_HELP, KEYBOARD_WORD_DIAGONAL_HELP,
		CONSONANT_VOWEL_CHARACTER_SEQUENCE_HELP, DENSITY_HELP,
		TWO_DICTIONARIES_SIMPLE_HELP, TWO_DICTIONARIES_TRANSLIT_HELP,
		TWO_DICTIONARIES_KEYBOARD_WORD_HELP, TWO_CHARACTERS_DISTRIBUTION_HELP };

const ENUM_MENU ICON_MENU[] = { MENU_SEARCH, MENU_EDIT, MENU_ADDITIONS,
		MENU_LANGUAGE, MENU_HELP, MENU_EDIT_SELECT_ALL_AND_COPY_TO_CLIPBOARD,
		MENU_EDIT_SELECT_ALL, MENU_EDIT_COPY_TO_CLIPBOARD,
		MENU_LOAD_ENGLISH_DICTIONARY, MENU_ENGLISH_LANGUAGE,
		MENU_LOAD_RUSSIAN_DICTIONARY, MENU_RUSSIAN_LANGUAGE, MENU_ABOUT,
		MENU_HOMEPAGE };
const std::string ICON_MENU_FILE_NAME[] = { "search.png", "edit.png", "add.png",
		"language.png", "help.png", "select_all_copy.png", "select_all.png",
		"copy.png", "en.gif", "en.gif", "ru.gif", "ru.gif", "word16.png",
		"web.png" };
//END ENCODE ARRAYS

const ENUM_MENU MENU_ACCEL[] = { MENU_EDIT_SELECT_ALL_AND_COPY_TO_CLIPBOARD,
		MENU_EDIT_SELECT_ALL, MENU_EDIT_COPY_TO_CLIPBOARD };
const int MENU_ACCEL_SIZE = SIZE(MENU_ACCEL);
const int ACCEL_KEY[] = {
GDK_KEY_B,
GDK_KEY_A,
GDK_KEY_C };

const ENUM_COMBOBOX HELPER_COMBOBOX[] = { COMBOBOX_HELPER0, COMBOBOX_HELPER1,
		COMBOBOX_HELPER2 };

const ENUM_MENU MENU_ADJUST_COMBO[] = { MENU_ANAGRAM, MENU_REGULAR_EXPRESSIONS,
		MENU_CHARACTER_SEQUENCE, MENU_SIMPLE_WORD_SEQUENCE,
		MENU_DOUBLE_WORD_SEQUENCE };

#endif /*#ifndef CGI*/

#endif /* CONSTS_H_ */
