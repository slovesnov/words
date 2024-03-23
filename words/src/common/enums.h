/*
 * enums.h
 *
 *  Created on: 21.12.2016
 *      Author: alexey slovesnov
 */

#ifndef ENUMS_H_
#define ENUMS_H_

enum ENUM_STRING {
	SEARCH,
	PROGRAM,
	VERSION,
	AUTHOR,
	COPYRIGHT,
	HOMEPAGE_STRING,
	HOMEPAGE_ONLINE_STRING,
	EMAIL_STRING,
	INVALID_WORD_FOUND,
	FOUND_SYMBOL_OUT_OF_ALPHABET,
	STRING_ERROR,
	LINE,
	PROCEED_SYMBOLS,
	WORDS,
	PALINDROME_HELP,
	NUMBER_OF_WORDS,
	KEYBOARD_WORD_SIMPLE_HELP,
	KEYBOARD_WORD_DIAGONAL_HELP,
	WORD_SEQUENCE_FULL_HELP,
	DIFFERENT_CHARACTERS,
	CHARACTERS,
	VOWELS,
	CONSONANTS,
	SEARCH_IN_ANY_PLACE_OF_WORD,
	SEARCH_IN_BEGIN_OF_WORD,
	SEARCH_IN_END_OF_WORD,
	MINIMUM,
	MAXIMUM,
	SEQUENCE,
	WORD_SEQUENCE_HELP,
	CHARACTERS_SEQUENCE_HELP,
	TWO_CHARACTERS_DISTRIBUTION_HELP,
	ANAGRAM_HELP,
	PANGRAM_HELP,
	CROSSWORD_HELP,
	TEMPLATE_HELP,
	TEMPLATE1,
	TEMPLATE2,
	TEMPLATE21,
	TEMPLATE3,
	REGULAR_EXPRESSION_HELP,
	DOUBLE_WORD_SEQUENCE_HELP,
	FROM,
	TO,
	NUMBER_OF_MATCHES,
	TIME_OF_LAST_OPERATION,
	TWO_DICTIONARIES_SIMPLE_HELP,
	TWO_DICTIONARIES_KEYBOARD_WORD_HELP,
	WORD_LENGTH_FREQUENCY,
	DICTIONARY_CHECK_FINISHED_SUCCESSFULLY,
	SORTED_BY_ALPHABET,
	SORTED_BY_FREQUENCY,
	CHARACTER_FREQUENCY_PERCENTS,
	CHARACTER_FREQUENCY_AT_THE_BEGINNING_PERCENTS,
	CHARACTER_FREQUENCY_AT_THE_END_PERCENTS,
	AVERAGE_WORD_LENGTH_EQUALS,
	FREQUENCY_OF_KEYBOARD_CHARACTERS,
	LENGTH,
	THE_LONGEST_WORD_IS,
	DICTIONARY, //rename from "current dictionary" because too many tools in status bar
	TWO_DICTIONARIES_TRANSLIT_HELP,
	CONSONANT_VOWEL_CHARACTER_SEQUENCE_HELP,
	DENSITY_HELP,
	SORT_BY_ALPHABET,
	SORT_BY_LENGTH,
	SORT_BY_WORDS,
	SORT_BY_PERCENT_OF_VOWELS,
	SORT_BY_PERCENT_OF_CONSONANTS,
	SORT_BY_DIFFERENT_NUMBER_OF_CHARACTERS,
	WORDS_NOT_IN_ALPHABET_ORDER,
	MODIFICATION_HELP,
	EVERY_MODIFICATION_CHANGES_WORD,
	NEW_VERSION_MESSAGE,
	CHAIN_HELP,
	NO_CHAINS_FOUND,
	RESULTS_FILTER,
	FOUND,
	NOT_FOUND,
	WITH_FILTER,
	SEPARATOR_SYMBOL,
	EXECUTABLE_FILE_SIZE,
	SPLITS_NOT_FOUND,
	LETTER_GROUP_SPLIT_HELP,
	PAIRS,
	TRIPLETS,

	STRING_SIZE
};

enum ENUM_MENU {
	MENU_SEARCH,
	MENU_ANAGRAM,
	MENU_PANGRAM,
	MENU_TEMPLATE,
	MENU_PALINDROME,
	MENU_CROSSWORD,
	MENU_REGULAR_EXPRESSIONS,
	MENU_MODIFICATION,
	MENU_CHAIN,
	MENU_CHARACTER_SEQUENCE,
	MENU_LETTER_GROUP_SPLIT,
	MENU_SIMPLE_WORD_SEQUENCE,
	MENU_DOUBLE_WORD_SEQUENCE,
	MENU_WORD_SEQUENCE_FULL,
	MENU_KEYBOARD_WORD_SIMPLE,
	MENU_KEYBOARD_WORD_COMPLEX,
	MENU_CONSONANT_VOWEL_SEQUENCE,
	MENU_DENSITY,

	MENU_WORDS_FROM_TWO_DICTIONARIES,
	MENU_TWO_DICTIONARIES_SIMPLE,
	MENU_TWO_DICTIONARIES_TRANSLIT,
	MENU_TWO_DICTIONARIES_KEYBOARD_WORD,

	MENU_EDIT,
	MENU_EDIT_SELECT_ALL_AND_COPY_TO_CLIPBOARD,
	MENU_EDIT_SELECT_ALL,
	MENU_EDIT_COPY_TO_CLIPBOARD,

	MENU_ADDITIONS,
	MENU_LOAD_DICTIONARY,
	MENU_LOAD_ENGLISH_DICTIONARY,
	MENU_LOAD_RUSSIAN_DICTIONARY,

	MENU_DICTIONARY_STATISTICS,
	MENU_WORD_FREQUENCY,
	MENU_CHECK_DICTIONARY,
	MENU_TWO_CHARACTERS_DISTRIBUTION,

	MENU_LANGUAGE,
	MENU_ENGLISH_LANGUAGE,
	MENU_RUSSIAN_LANGUAGE,

	MENU_HELP,
	MENU_ABOUT,
	MENU_HOMEPAGE
};

enum ENUM_COMBOBOX {
	COMBOBOX_SORT,
	COMBOBOX_SORT_ORDER,
	COMBOBOX_DICTIONARY,
	COMBOBOX_HELPER0,
	COMBOBOX_HELPER1, //should goes immediately after COMBOBOX_HELPER0
	COMBOBOX_HELPER2, //should goes immediately after COMBOBOX_HELPER1
	COMBOBOX_FILTER,
	COMBOBOX_SIZE
};

enum ENUM_SETTINGS {
	SETTINGS_ALPHABET,
	SETTINGS_VOWELS,
	SETTINGS_CONSONANTS,
	SETTINGS_KEYBOARD_ROW1,
	SETTINGS_KEYBOARD_ROW2, //should goes immediately after SETTINGS_KEYBOARD_ROW1
	SETTINGS_KEYBOARD_ROW3 //should goes immediately after SETTINGS_KEYBOARD_ROW2
};

enum ENUM_VOWELS_CONSONANTS {
	VOWELS_CONSONANTS_VOWELS, VOWELS_CONSONANTS_CONSONANTS,

	VOWELS_CONSONANTS_SIZE
};

enum ENUM_MODIFICATION_TYPE {
	MODIFICATION_TYPE_REPLACE,
	MODIFICATION_TYPE_INSERT,
	MODIFICATION_TYPE_SUBSTRING,

	MODIFICATION_TYPE_SIZE
};

#ifdef CGI
enum POST_ENUM{
	POST_SEARCHTYPE,
	POST_ENTRY,
	POST_DICTIONARY,
	POST_SORTTYPE,
	POST_SORTORDER,
	POST_LANGUAGE,
	POST_COMBO0,
	POST_COMBO1,
	POST_COMBO2,
	POST_CHECK,
	POST_SIZE
};

enum CGI_STRING_ENUM{
	CGI_STRING_ERROR_INVALID_REGULAR_EXPRESSION,
	CGI_STRING_ERROR_INVALID_MODIFICATION_STRING,

	CGI_STRING_SIZE
};
#endif

#endif /* ENUMS_H_ */
