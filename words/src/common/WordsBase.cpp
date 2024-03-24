/*
 * WordsBase.cpp
 *
 *  Created on: 20.11.2017
 *      Author: alexey slovesnov
 */

#include "consts.h"
#include "WordsBase.h"
#include <memory> //unique_ptr
#include "aslov.h"

typedef unsigned char uchar;

#ifdef CGI
	#define	RETURN_ON_USER_BREAK(a)
#else
#define	RETURN_ON_USER_BREAK(a) if( userBreakThread()){return a;}
#endif

#ifdef _WIN32
const char EL = '\n';
#else
	const char EL='\r';
#endif

#ifdef CGI
	#include "cgi.h"

	//should match with POST_ENUM
	const std::string POST_NAME[]={"searchType","entry","dictionary","sortType","sortOrder","language","combo0","combo1","combo2","check"};

	const ENUM_MENU COMBO_MENU[]={
		MENU_ANAGRAM,
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
		MENU_TWO_CHARACTERS_DISTRIBUTION
	};
#endif

WordsBase *wordsBase;

WordsBase::WordsBase() {
	int i, j;
	FILE *f;
	char *p;
	char buff[MAX_BUFF_LEN];
	m_template_a=nullptr;

#ifndef CGI
	char *p1, *p2;
#endif

	wordsBase = this;
#ifndef CGI
	m_filterRegex = nullptr;
#endif

	//load settings
	for (i = 0; i < LANGUAGES; i++) {
		f = open(i, "settings");
		assert(f!=NULL);
		while (fgets(buff, MAX_BUFF_LEN, f) != NULL) {
			p = strchr(buff, ' ');
			assert(p!=NULL);
			m_settings[i].push_back(std::string(buff, p - buff));
		}
		fclose(f);
	}
#ifndef CGI
	//load templates
	for (i = 0; i < LANGUAGES; i++) {
		f = open(i, "template");
		assert(f!=NULL);
		p = fgets(buff, MAX_BUFF_LEN, f); //skip first line
		assert(p);
		p = fgets(buff, MAX_BUFF_LEN, f);
		assert(p);
		for (; (p1 = strchr(p, ' ')) != NULL || (p1 = strchr(p, EL)) != NULL;
				p = p1 + 1) {
			assert(p1);

			//parse substring with quotes
			p2 = strchr(p, '"');
			j = p2 && p2 < p1;
			if (j) {
				p = p2 + 1;
				p1 = strchr(p, '"');
				assert(p1);
			}

			m_template[i].push_back(std::string(p, p1 - p));

			if (j) {
				p1++;
			}
		}

		assert(m_template[i].size()==SIZE(TEMPLATE_MENU));
		fclose(f);
	}
#endif

//#define MODIFY_DICTIONARY

#ifdef MODIFY_DICTIONARY
	int cn = 0;
	//modify word is equivalent delete & add
	const int MODIFY_INDEX = 1;
	const VString deleteWords = { "аьберт", "госкомсанэпидназдор",
			"окт€брьскский", "ухойдакать" };
	const VString addWords = { "рухнет", "прескриптивизм" };

	/*
	 const int MODIFY_INDEX = 0;
	 const VString deleteWords = {  };
	 const VString addWords = { "tokmak"};
	 */

#endif
	//load dictionaries
	for (i = 0; i < LANGUAGES; i++) {
		m_longestWordLength[i] = 0;
		f = open(i, "words");
		assert(f!=NULL);
		while (fgets(buff, MAX_BUFF_LEN, f) != NULL) {
			removeLastCRLF(buff);
#ifdef MODIFY_DICTIONARY
			if (i == MODIFY_INDEX) {
				std::string s = buff;
				if (oneOf(s, deleteWords)) {
					cn++;
					continue;
				}
			}
#endif
			m_dictionary[i].insert(buff);
			j = strlen(buff);
			if (j > m_longestWordLength[i]) {
				m_longestWordLength[i] = j;
			}
		}
		fclose(f);
	}
#ifdef MODIFY_DICTIONARY
	printl("deleted",cn,'/',deleteWords.size())
	cn = 0;
	for (auto &a : addWords) {
		if (m_dictionary[MODIFY_INDEX].insert(a).second) {
			cn++;
		}
	}
	printl("added",cn,'/',addWords.size())
	//store dictionary
	f = open(MODIFY_INDEX, "words1", 2);
	assert(f!=NULL);
	for (auto &a : m_dictionary[MODIFY_INDEX]) {
		fprintf(f, "%s\n", a.c_str());
	}
	fclose(f);

#endif

#ifdef CGI
	cgi();
#endif
}

WordsBase::~WordsBase() {
#ifndef CGI
	if (m_filterRegex) {
		g_regex_unref(m_filterRegex);
	}
#endif
	clear_m_template_a();
}

bool WordsBase::spanIncluding(const char *p, const std::string &pattern) {
	for (; *p != 0; p++) {
		if (pattern.find(*p) == std::string::npos) {
			return false;
		}
	}
	return true;
}

bool WordsBase::checkKeyboardWordSimple(const std::string &s) {
	const char *p = s.c_str();
	std::string *ps = m_keyboardOneRow[uchar(*p)];
	return spanIncluding(p, ps[0]) || spanIncluding(p, ps[1]);
}

bool WordsBase::checkKeyboardWordComplex(const std::string &s) {
	const char *p = s.c_str();
	for (; p[1] != 0; p++) {
		if (m_keyboardRowDiagonals[uchar(*p)].find(p[1]) == std::string::npos) {
			return false;
		}
	}
	return true;
}

bool WordsBase::checkPangram(const std::string &s) {
	const char *p;
	std::set<char> set;
	for (p = s.c_str(); *p != 0; p++) {
		set.insert(*p);
	}
	return int(set.size()) >= m_comboValue[COMBOBOX_HELPER0];
}

bool WordsBase::checkTemplate(const std::string &s) {
	const char *p = s.c_str();
	int param = m_comboValue[COMBOBOX_HELPER0];
	if (param == 0) {
		if (strlen(p) < m_entryText.length()) {
			return false;
		}

		char a[256];
		memset(a, 0, 256);
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
		if (strlen(p) > m_entryText.length()) {
			return false;
		}
		char a[256];
		memset(a, 0, 256);
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
			k = m_template_a[j * 256 + uchar(s[i])];
			if (!k
					|| ((j += k) >= m_entryText.length() && i + 1 < s.length())) {
				return false;
			}
		}
		return true;
	} else {
		return spanIncluding(p, m_entryText);
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

bool WordsBase::checkLetterGroupSplit(const std::string &s) {
	return s.length() > 2;
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
	const char *q = getSettings(
			m_comboValue[COMBOBOX_HELPER2] == 0 ?
					SETTINGS_VOWELS : SETTINGS_CONSONANTS).c_str();
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
	const char *q = m_settings[getDictionaryIndex()][
			m_comboValue[COMBOBOX_HELPER1] == 0 ?
					SETTINGS_VOWELS : SETTINGS_CONSONANTS].c_str();
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
	const char *p1 = p + strlen(p) - 1;
	for (; p < p1; p++, p1--) {
		if (*p != *p1) {
			return false;
		}
	}
	return true;
}

bool WordsBase::checkRegularExpression(const std::string &s) {
#ifdef _WIN32
	GMatchInfo *matchInfo;
	g_regex_match(m_regex, s.c_str(), GRegexMatchFlags(0), &matchInfo);
	int i;
	const int max = m_comboValue[COMBOBOX_HELPER1];
	for (i = 0; g_match_info_matches(matchInfo) && i <= max; i++) {
		g_match_info_next(matchInfo, NULL);
	}
	g_match_info_free(matchInfo);

	return i >= m_comboValue[COMBOBOX_HELPER0] && i <= max;
#else
	//gcc on sourceforge 4.8.5 doesn't support std::sregex_iterator
	std::ptrdiff_t const matches(std::distance(
			std::sregex_iterator(s.begin(), s.end(), m_regex),
			std::sregex_iterator()));
	return matches>=m_comboValue[COMBOBOX_HELPER0] && matches<=m_comboValue[COMBOBOX_HELPER1];
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
	char *pu = new char[SZ];
	memset(pu, 0, SZ);
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
	delete[] pu;
}

bool WordsBase::findAnagram() {
	StringSet const &r = getDictionary();
	StringSetCI it;
	int i;
	std::string s;
	MapStringStringVector map;
	MapStringStringVectorI cit;

	for (i = m_comboValue[COMBOBOX_HELPER0];
			i <= m_comboValue[COMBOBOX_HELPER1]; i++) {
		for (it = r.begin(); it != r.end(); it++) {
			if (int(it->length()) != i) {
				continue;
			}
			s = *it;
			std::sort(s.begin(), s.end());
			cit = map.find(s);
			if (cit == map.end()) {
				VString v;
				v.push_back(*it);
				map[s] = v;
			} else {
				cit->second.push_back(*it);
			}
		}
		for (cit = map.begin(); cit != map.end(); cit++) {
			VString &rv = cit->second;
			if (rv.size() < 2) {
				continue;
			}
			s = "";
			for (auto svi : rv) {
				if (!s.empty()) {
					s += " ";
				}
				s += svi;
			}
			m_result.push_back(
					SearchResult(s, rv.begin()->length(), rv.size()));
		}

		map.clear();

		RETURN_ON_USER_BREAK(true)

	}

	return false;

}

bool WordsBase::findSimpleWordSequence() {
	StringSet const &r = getDictionary();
	StringSetCI it;
	int i, j;
	std::string s;
	MapStringTwoStringVectors map;
	MapStringTwoStringVectorsI mit;
	for (i = m_comboValue[COMBOBOX_HELPER0];
			i <= m_comboValue[COMBOBOX_HELPER1]; i++) {
		for (it = r.begin(); it != r.end(); it++) {
			if (int(it->length()) >= i) {
				for (j = 0; j < 2; j++) {
					s = j == 1 ?
							it->substr(0, i) : it->substr(it->length() - i);
					if ((mit = map.find(s)) == map.end()) {
						TwoStringVectors v;
						v.add(j, *it);
						map[s] = v;
					} else {
						mit->second.add(j, *it);
					}
				}
			}

			RETURN_ON_USER_BREAK(true)

		}

		fillResultFromMap(map, i);
		map.clear();
	}
	return false;
}

bool WordsBase::findDoubleWordSequence() {
	StringSet const &r = getDictionary();
	StringSetCI it;
	int i, j;
	std::string s, t, q;
	MapStringTwoStringVectors map;
	MapStringTwoStringVectorsI mit;

	for (i = m_comboValue[COMBOBOX_HELPER0];
			i <= m_comboValue[COMBOBOX_HELPER1]; i++) {
		for (it = r.begin(); it != r.end(); it++) {

			if (int(it->length()) >= i) {
				s = it->substr(0, i);
				q = it->substr(it->length() - i);
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
						v.add(0, *it);
						v.add(1, *it);
					} else {
						v.add(j, *it);
					}
					map[t] = v;
				} else {
					if (j == 2) {
						mit->second.add(0, *it);
						mit->second.add(1, *it);
					} else {
						mit->second.add(j, *it);
					}
				}

			}

			RETURN_ON_USER_BREAK(true)
		}

		fillResultFromMap(map, i);
		map.clear();
	}
	return false;
}

bool WordsBase::findWordSequenceFull() {
	StringSet const &r = getDictionary();
	StringSetCI it;
	int i;
	std::string s;

	for (it = r.begin(); it != r.end(); it++) {
		if (checkPalindrome(*it)) {
			continue;
		}
		s.clear();
		for (i = it->length() - 1; i >= 0; i--) {
			s += (*it)[i];
		}
		if (s > *it && getDictionary().find(s) != getDictionary().end()) {
			m_result.push_back(SearchResult(*it + " " + s, it->length(), 2));
		}
	}
	return false;
}

bool WordsBase::findModification() {
	StringSet const &r = getDictionary();
	StringSetCI it;
	std::string s;
	for (it = r.begin(); it != r.end(); it++) {
		s = m_modifications.apply(*it, m_checkValue);

		if (!s.empty() && s != *it
				&& getDictionary().find(s) != getDictionary().end()) {
			m_result.push_back(SearchResult(*it + " " + s, it->length(), 1));
		}

		RETURN_ON_USER_BREAK(true)
	}
	return false;
}

bool WordsBase::findChain() {
	StringSet const &r = getDictionary();
	StringSetCI dci;
	int i, j, k, l, mx[2], *ni, *nis;
	MapStringInt dl;
	VString v, w[2];
	StringI ic;
	StringCI ia;
	MapStringIntI mi;
	char c;
	std::string s;
	StringSet vs[2];
	VString *vl;
	ChainNodeVector *ch;
	ChainNodeVectorI cni;
	ChainNodeVectorCI cni1;
	ChainNode *cp;

	if (differenceOnlyOneChar(m_chainHelper[0], m_chainHelper[1])) {
		m_out = localeToUtf8(m_chainHelper[0] + " " + m_chainHelper[1])+OPEN_S+m_language[WORDS]
				+ "2)";
		return false;
	}

	for (dci = r.begin(); dci != r.end(); dci++) {
		if (dci->length() == m_chainHelper[0].length()) {
			dl[*dci] = 0;
		}
	}

	j = 1; //j is step
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
				for (ic = vr.begin(); ic != vr.end(); ic++) {
					c = *ic;
					for (ia = getAlphabet().begin(); ia != getAlphabet().end();
							ia++) {
						if (*ia != c) {
							*ic = *ia;
							mi = dl.find(vr);
							if (mi != dl.end()) {
								if (mi->second == 0) {
									v.push_back(vr);
									mi->second = i == 0 ? j : -j;
								} else {
									if ((mi->second > 0 && i == 1)
											|| (mi->second < 0 && i == 0)) {
										s = vr;
										*ic = c;
										if (mi->second > 0) {
											//USE set not vector to avoid same words. Example "тара кофе"
											vs[0].insert(s);
											vs[1].insert(vr);
										} else {
											//USE set not vector to avoid same words. Example "тара кофе"
											vs[0].insert(vr);
											vs[1].insert(s);
										}
									}
								}
							}
						}
					}
					*ic = c;
				}
			}
			if (v.empty() || !vs[0].empty()) {
				goto l210;
				//break two cycles
			}
			w[i] = v;
		} //for(i)
	}

	l210: if (vs[0].empty()) {
		m_out = m_language[NO_CHAINS_FOUND];
		return false;
	}

	j = 0;
	for (i = 0; i < 2; i++) {
		mi = dl.find(*vs[i].begin());
		assert(mi!=dl.end());
		mx[i] = mi->second;
		j += i == 0 ? mx[i] : -mx[i];
	}
	//mi->second==1 is start word
	//mi->second==-1 is end word
	//so j-=2;
	j -= 2;

	vl = new VString[j];
	ch = new ChainNodeVector[j];
	ni = new int[j];
	nis = new int[j];

	if (j == 1) {//special proceeding for j==1 on change check check [тара->кора]
		for (dci = vs[0].begin(); dci != vs[0].end(); dci++) {
			ch[0].push_back(ChainNode(*dci));
		}
	} else {
		//fill vl
		for (mi = dl.begin(); mi != dl.end(); mi++) {
			if (mi->second > 1 && mi->second <= mx[0]) {
				vl[mi->second - 2].push_back(mi->first);
			} else if (mi->second < -1 && mi->second >= mx[1]) {//mi->second==1 is end word
				assert(j+mi->second+1>=0);
				vl[j + mi->second + 1].push_back(mi->first);
			}
		}

		//fill ch[] in middle
		mi = dl.find(*vs[0].begin());
		assert(mi!=dl.end());
		for (i = 0, l = mi->second - 2; i < 2; i++, l++) {
			for (dci = vs[i].begin(); dci != vs[i].end(); dci++) {
				ch[l].push_back(ChainNode(*dci));
			}
		}

		//make links for ch[] in middle
		l -= 2;
		for (cni = ch[l].begin(); cni != ch[l].end(); cni++) {
			for (cni1 = ch[l + 1].begin(), k = 0; cni1 != ch[l + 1].end();
					cni1++, k++) {
				if (differenceOnlyOneChar(cni->s, cni1->s)) {
					//find all links, so no break here
					cni->next.push_back(k);
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
					for (cni = ch[l].begin(); cni != ch[l].end(); cni++) {
						if (differenceOnlyOneChar(vci, cni->s)) {
							//find all links, so no break here
							if (!cp) {
								ch[i].push_back(ChainNode(vci));
								cp = &(ch[i].back());
							}
							if (k == -1) {
								cp->next.push_back(cni - ch[l].begin());
							} else {
								cni->next.push_back(ch[i].size() - 1);
							}
						}
					}
				}
			}
		}
	}	//else (j)

	//BEGIN OUTPUT ALL TREE PATH
	//init path
	for (i = 0; i < j; i++) {
		ni[i] = 0;
	}
	nis[0] = ch[0].size();
	for (i = 1; i < j; i++) {
		nis[i] = ch[i - 1][ni[i - 1]].next.size();
	}

	//iterate path
	v.clear();
	while (1) {
		//output tree path
		s = m_chainHelper[0];
		for (i = 0; i < j; i++) {
			if (i == 0) {
				k = ni[i];
			} else {
				k = ch[i - 1][ni[i - 1]].next[ni[i]];
			}
			const ChainNode &node = ch[i][k];
			//printf("%s %d %d,%d\n",node.s.c_str(),i,ni[i],nis[i]);
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
	delete[] ch;
	delete[] vl;
	delete[] ni;
	delete[] nis;

	for (auto &e : v) {
		m_result.push_back(SearchResult(e, m_chainHelper[0].length(), j + 2));
	}

	return false;
}

void WordsBase::fillResultFromMap(const MapStringTwoStringVectors &map,
		size_t len) {
	MapStringTwoStringVectorsCI mit;
	std::string svi;
	int j, k;
	std::string s;

	for (mit = map.begin(); mit != map.end(); mit++) {
		const TwoStringVectors &v = mit->second;
		VString const &v1 = v.v1;
		VString const &v2 = v.v2;
		j = v1.size();
		k = v2.size();

		if (j != 0 && k != 0) {

			//21nov2017 fixed only one same word found
			if (j == 1 && k == 1 && v1[0] == v2[0] && v1[0].length() == len) {
				continue;
			}

			s = "";
			for (auto svi : v1) {
				s += svi + " ";
			}
			s += "-";
			for (auto svi : v2) {
				s += " " + svi;
			}

			m_result.push_back(SearchResult(s, v1.begin()->length(), j + k));

		}
	}

}

bool WordsBase::twoDictionaries(bool translit) {
	/*
	 * Note time difference with same code java & c++ (c++ just took from java, only small  not significant optimizations
	 * so it's the same code. Time is written to store it before some accelerations)
	 *                 java   c++
	 * translit=false  1.03  0.71 (1.03-0.71)/1.03=31%
	 * translit=true   1.73  1.50 (1.73-1.50)/1.73=13.3%
	 */

	std::unique_ptr<VString[]> to;
	//VString* to=NULL;
	StringSetCI it;
	int i, j, l;
	int fromIndex = -1;
	const char BR[] = " ";
	char *p, *p1;
	FILE *f = fopen(
			getResourcePath(
					LNG[0] + LNG[1] + "_" + (translit ? "translit" : "simple")
							+ ".txt").c_str(), "r");
	assert(f);
	const int BUFF_LEN = 128;
	char buff[BUFF_LEN];
	char COMMENT[] = "//";
	std::string s, alphabetFrom;
	const int di = getDictionaryIndex();

	while (fgets(buff, BUFF_LEN, f) != NULL) {
		if (startsWith(buff, COMMENT)) {
			continue;
		}
		removeLastCRLF(buff);

		if (to) {
			for (i = 0, p = buff; (p1 = strstr(p, BR)) != NULL;
					i++, p = p1 + strlen(BR)) {
				if (i == 0) {
					assert(p1-p==1);	//the first string should have length=1
					j = indexOf(*p, alphabetFrom);
					assert(j>=0);
				} else {
					to[j].push_back(
							*p == '\'' ? "" : format("%.*s", p1 - p, p));//'\'' = empty string
				}
			}
			to[j].push_back(*p == '\'' ? "" : p);	//'\'' = empty string
		} else {
			for (fromIndex = 0;
					fromIndex < LANGUAGES
							&& !startsWith(buff,
									getShortLanguageString(fromIndex));
					fromIndex++)
				;
			assert(fromIndex<LANGUAGES);
			const std::string &af = m_settings[fromIndex][SETTINGS_ALPHABET];
			alphabetFrom = af;
			//to[.] is vector of transformation fromAlphabet->symbols n toAlphabet //fixed 4.3

			//to=new VString[af.length()];
			//to=std::make_unique<VString[]>(af.length());

			/* NOTE next two strings of code are the same
			 * to=std::make_unique<VString[]>(af.length()); since gcc 4.9
			 * to=std::unique_ptr<VString[]>(new VString[af.length()]);
			 * but the first one is not supported by old gcc on sourceforge
			 */
			//to=std::make_unique<VString[]>(af.length()); since gcc 4.9
			to = std::unique_ptr<VString[]>(new VString[af.length()]);
		}
	}
	fclose(f);

	StringSet const &df = m_dictionary[fromIndex];
	StringSet const &dt = m_dictionary[fromIndex == 1 ? 0 : 1];

	//4.31 use smart pointers to avoid make common user break mechanism
	i = m_longestWordLength[fromIndex];
	std::unique_ptr<StringVectorPtr[]> k(new StringVectorPtr[i]);
	std::unique_ptr<int[]> id(new int[i]);
	//StringVectorPtr* k=new StringVectorPtr[m_longestWordLength[fromIndex]];
	//int*id=new int[m_longestWordLength[fromIndex]];
	StringVectorPtr *pk;
	int *pid;
	int len, n;

	for (it = df.begin(); it != df.end(); it++) {

		len = it->length();
		for (pk = k.get(), n = 1, i = 0; i < len; i++, pk++) {
			j = indexOf((*it)[i], alphabetFrom);
			assert(j>=0);
			//Check whether char has no correspondence in config file.
			//For example symbol '-' in russian alphabet, has no correspondence in english alphabet
			*pk = to.get() + j;
			l = (*pk)->size();
			if (l == 0) {
				goto l1531;
				//faster then use break and check i<int(it->length()) after cycle
			}
			id[i] = l;
			n *= l;
		}

		for (j = 0; j < n; j++) {
			s.clear();
			l = j;
			for (pk = k.get(), pid = id.get(), i = 0; i < len;
					i++, pk++, pid++) {
				s += (**pk)[l % *pid];
				l /= *pid;
			}
			if (dt.find(s) != dt.end()) {
				/* fixed 4.3 first word should be in current dictionary language,
				 * for correct sorting vowels/consonant percent*/
				if (di == fromIndex) {
					s = *it + " " + s;
				} else {
					s = s + " " + *it;
				}
				assert(len==int(it->length()));
				m_result.push_back(SearchResult(s, len, 1));//mark as 1 word only to not show number of words
			}
		}
		l1531:
		RETURN_ON_USER_BREAK(true);
	}
	return false;
}

bool WordsBase::keyboardWords() {
	StringSet const &df = m_dictionary[0];
	StringSet const &dt = m_dictionary[1];
	StringSetCI it;
	int i;
	char b[128];
	char *p;
	std::string s;
	std::string::size_type j, len;
	char a[256];
	const int di = getDictionaryIndex();

	assert(SETTINGS_KEYBOARD_ROW2==SETTINGS_KEYBOARD_ROW1+1);
	assert(SETTINGS_KEYBOARD_ROW3==SETTINGS_KEYBOARD_ROW1+2);

	memset(a, 0, 256);
	for (i = SETTINGS_KEYBOARD_ROW1; i <= SETTINGS_KEYBOARD_ROW3; i++) {
		s = m_settings[0][i];
		assert(m_settings[1][i].length()>=s.length());
		for (j = 0; j < s.length(); j++) {
			//all english keys have russian char on the same keyboard key
			a[uchar(s[j])] = m_settings[1][i][j];
		}
	}

	for (it = df.begin(); it != df.end(); it++) {
		p = b;
		len = it->length();

		for (j = 0; j < len; j++) {
			*p++ = a[uchar((*it)[j])];
		}
		*p = 0;

		if (dt.find(b) != dt.end()) {
			/* fixed 4.3 first word should be in current dictionary language,
			 * for correct sorting vowels/consonant percent*/
			if (di == 0) {
				s = *it + " " + b;
			} else {
				s = b + std::string(" ") + *it;
			}
			m_result.push_back(SearchResult(s, len, 1));
		}

	}
	return false;
}

bool WordsBase::wordFrequency() {
	int i;
	const int MAX = getMaximumWordLength();
	int *m = new int[MAX];
	StringSet const &r = getDictionary();
	StringSetCI it;
	IntIntVector v;
	IntIntVectorCI itv;

	for (i = 0; i < MAX; ++i) {
		m[i] = 0;
	}

	for (it = r.begin(); it != r.end(); it++) {
		m[it->length() - 1]++;			//use length-1
	}

	for (i = 0; i < MAX; ++i) {
		if (m[i] > 0) {
			v.push_back(std::make_pair(m[i], i + 1));//store actual word length
		}
	}

	m_out = m_language[WORD_LENGTH_FREQUENCY];

	std::sort(v.begin(), v.end(), sortIntInt);

	for (itv = v.begin(); itv != v.end(); itv++) {
		//use separator for intToString for understandable view
		m_out += format("\n%2d %6.3lf%% %6s/%s", itv->second,
				100. * itv->first / r.size(), toString(itv->first, ',').c_str(),
				toString(r.size(), ',').c_str());
	}

	delete[] m;
	return false;
}

bool WordsBase::checkDictionary() {
	//NOTE!!! SHOULD CHECK DICTIONARY FILE NOT!!! DICTIONARY SET. Check for duplicates and valid line numbers
	int i, line;
	ENUM_STRING error;
	std::string s;
	FILE *f;
	const int BUFF_LEN = 1024;
	char buff[BUFF_LEN];

	//open in binary mode to check whether line ended by \r\n, should be always \n only
	f = open(getDictionaryIndex(), "words", 1);
	assert(f!=NULL);
	for (line = 1; fgets(buff, BUFF_LEN, f) != NULL; line++) {
		error = STRING_SIZE;

		i = strlen(buff);
		if (i < 2 || buff[i - 1] != '\n' || buff[i - 2] == '\r') {//buff should ended only with "\n", not "\r\n"
			error = INVALID_WORD_FOUND;
		} else {
			//after preliminary check
			removeLastCRLF(buff);

			if (spanIncluding(buff, getAlphabet())) {
				if (std::string(buff) <= s) {	//as well check duplicate words
					error = WORDS_NOT_IN_ALPHABET_ORDER;
				} else {
					s = buff;
				}
			} else {
				error = FOUND_SYMBOL_OUT_OF_ALPHABET;
			}
		}

		if (error != STRING_SIZE) {
			if (!m_out.empty()) {
				m_out += "\n";
			}
			m_out += m_language[STRING_ERROR] + ". " + m_language[error] + " "
					+ m_language[LINE] + " " + intToStringLocaled(line);
		}
	}
	fclose(f);

	if (m_out.empty()) {
		m_out = m_language[DICTIONARY_CHECK_FINISHED_SUCCESSFULLY];
	}
	return false;
}

bool WordsBase::twoCharactersDistribution() {
	StringSet const &r = getDictionary();
	StringSetCI it;
	int i, j;
	const int s = getAlphabetSize();
	int **a = create2dArray<int>(s, s);
	std::string ss;

	for (i = 0; i < s; i++) {
		for (j = 0; j < s; j++) {
			a[i][j] = 0;
		}
	}

	int total = 0;
	for (it = r.begin(); it != r.end(); it++) {
		if (it->length() < 2) {
			continue;
		}
		for (i = 0; i < int(it->length()) - 1; i++) {
			a[alphabetIndex((*it)[i])][alphabetIndex((*it)[i + 1])]++;
			total++;
		}
	}

	StringIntVector v;
	for (i = 0; i < s; i++) {
		for (j = 0; j < s; j++) {
			if (a[i][j] != 0) {
				ss = getAlphabetChar(i);
				ss += getAlphabetChar(j);
				v.push_back(std::make_pair(ss, a[i][j]));
			}
		}
	}
	std::sort(v.begin(), v.end(), sortStringInt);

	StringIntVectorCI p;
	for (p = v.begin(); p != v.end(); p++) {
		if (p != v.begin()) {
			m_out += "\n";
		}
		m_out += localeToUtf8(
				format("%s %.2f%% %6s/%s", p->first.c_str(),
						(p->second * 100.) / total,
						toString(p->second, ',').c_str(),
						toString(r.size(), ',').c_str()));
	}

	delete2dArray(a, s);
	return false;
}

bool WordsBase::dictionaryStatistics() {
	int i, j;
	unsigned k;
	double v;
	StringSetCI it;
	const int SZ_CAPTION = 3;
	std::string caption[SZ_CAPTION];
	std::string additionalCaption[2];
	std::string longestWord;
	const int a = getAlphabetSize();
	int **m = create2dArray<int>(SZ_CAPTION, a + 1);
	StringSet const &r = getDictionary();

	for (i = 0; i < SZ_CAPTION; i++) {
		caption[i] = m_language[
				i == 0 ?
						CHARACTER_FREQUENCY_PERCENTS :
						(i == 1 ?
								CHARACTER_FREQUENCY_AT_THE_BEGINNING_PERCENTS :
								CHARACTER_FREQUENCY_AT_THE_END_PERCENTS)];
	}

	for (i = 0; i < SIZEI(additionalCaption); i++) {
		additionalCaption[i] = " "
				+ m_language[i == 0 ? SORTED_BY_ALPHABET : SORTED_BY_FREQUENCY];
	}

	for (j = 0; j < SZ_CAPTION; j++) {
		for (i = 0; i < a + 1; ++i) {
			m[j][i] = 0;
		}
	}

	for (it = r.begin(); it != r.end(); it++) {
		m[0][a] += it->length();
		m[1][a]++;
		m[2][a]++;

		if (it->length() > longestWord.length()) {
			longestWord = *it;
		}

		for (i = 0; i < int(it->length()); ++i) {
			m[0][alphabetIndex((*it)[i])]++;
		}

		m[1][alphabetIndex((*it)[0])]++;
		m[2][alphabetIndex((*it)[it->length() - 1])]++;
	}

	std::string s, s2;
	m_out = m_language[PROCEED_SYMBOLS] + " " + intToStringLocaled(m[0][a])
			+ ", " + m_language[WORDS] + " " + intToStringLocaled(m[1][a]);

	m_out += "\n" + m_language[AVERAGE_WORD_LENGTH_EQUALS]
			+ format(" %.2lf", (double(m[0][a])) / m[1][a]);

	IntDouble ve[a];
	const int COLUMNS = a / 3;
	double *frequency = new double[getAlphabetSize()];
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

			ve[i] = std::make_pair(i, v);
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
				s2 =
						m_settings[getDictionaryIndex()][SETTINGS_KEYBOARD_ROW1
								+ i];
				for (k = 0; k < s2.length(); k++) {
					s += localeToUtf8(
							format(FORMAT, s2[k],
									frequency[alphabetIndex(s2[k])]));
				}
				m_out += "\n" + s;
			}
		}

	}

	m_out += "\n\n" + m_language[THE_LONGEST_WORD_IS]
			+ format(" - %s (%s %d).", localeToUtf8(longestWord).c_str(),
					m_language[LENGTH].c_str(), longestWord.length());

	delete2dArray(m, SZ_CAPTION);
	delete[] frequency;
	return false;
}

void WordsBase::sortFilterResults() {
	int i;
	//leave SearchResultVectorCI for old gcc compiler under sf.net
	SearchResultVectorCI it;
	std::string s;

#ifndef CGI
	bool find = m_comboValue[COMBOBOX_FILTER] == 0;
#endif

	m_out.clear();
	if (!m_outSplitted) {
		std::sort(m_result.begin(), m_result.end(),
				SORT_FUNCTION[m_comboValue[COMBOBOX_SORT] * 2
						+ m_comboValue[COMBOBOX_SORT_ORDER]]);
	}

	for (it = m_result.begin(); it != m_result.end(); it++) {
		s=localeToUtf8(it->s);
#ifndef CGI
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
			m_out += OPEN_S+m_language[CHARACTERS]+format(" %d",it->length);

			if (it->words > 1) {
				m_out += format(" %s %d", m_language[WORDS].c_str(), it->words);
			}

			i = m_comboValue[COMBOBOX_SORT] - (NUMBER_OF_SORTS - 3);
			if (i == 0 || i == 1) {			//sort by vowels, consonants
				m_out += " " + m_language[VOWELS + i]
						+ format(" %.1lf%%", it->percent[i]);//show percent of vowels or consonants
			} else if (i == 2) {
				m_out += " " + m_language[DIFFERENT_CHARACTERS]
						+ format(" %d", it->differentCharacters);
			}

			m_out += ")";
		}
		RETURN_ON_USER_BREAK()
	}
}

void WordsBase::loadLanguage() {
	FILE *f;
	int i;
	char buff[MAX_BUFF_LEN];
	std::string s;
	m_language.clear();			//Note first menu item -> m_language[SEARCH]

	f = open(m_languageIndex, "language");
	assert(f!=NULL);

#ifdef __unix__
		const char ML=2;
#else
	const char ML = 1;
#endif

	for (i = 0; fgets(buff, MAX_BUFF_LEN, f) != NULL && strlen(buff) > ML;) {
		if (startsWith(buff, SEPARATOR) || strchr(buff, '}') != NULL) {
			continue;
		}

		s = parseString(buff);//Note should use immediately std::string in gcc5.4.0
#ifndef CGI
		setMenuLabel(ENUM_MENU(i), s);
#endif
		if (i == 0) {
			m_language.push_back(utf8ToLowerCase(s));
		}
		i++;
	}

	while (fgets(buff, MAX_BUFF_LEN, f) != NULL) {
		m_language.push_back(parseString(buff));
	}
	fclose(f);
	assert(m_language.size()==STRING_SIZE);

#ifndef CGI
	//forma("4.4") -> "4.4", "4.41" -> "4.31" no zeros after
	m_programVersion = m_language[PROGRAM] + " " + m_language[VERSION] + " "
			+ forma(WORDS_VERSION);
#endif
	m_language[MODIFICATION_HELP] = format(
			m_language[MODIFICATION_HELP].c_str(),
			m_language[EVERY_MODIFICATION_CHANGES_WORD].c_str());
	/* use only first symbol. In Russian language separator is space, so ignore comment in language.txt file.
	 * Comment in ru/language.txt is important because otherwise it'll we empty string and looks like a bug
	 */
	m_language[SEPARATOR_SYMBOL] = m_language[SEPARATOR_SYMBOL].substr(0, 1);

#ifdef CGI
	f=open(m_languageIndex,"cgi_language");
	assert(f!=NULL);
	for( ; fgets(buff,MAX_BUFF_LEN,f)!=NULL ;){
		removeLastCRLF(buff);
		s=buff;
		m_cgiLanguage.push_back( localeToUtf8(s) );
	}
	assert(m_cgiLanguage.size()==CGI_STRING_SIZE);
	fclose(f);
#endif

}

const std::string WordsBase::parseString(const char *buff) {
	std::string s = localeToUtf8(buff);
	const char *b = s.c_str();
	const char *p = strchr(b, '{');
	if (p == NULL) {
		p = strchr(b, EL);
		if (p == NULL) {			//last string in file
			return b;
		}
	}
	return std::string(b, p - b);
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

	//MENU_REGULAR_EXPRESSIONS MENU_MODIFICATION MENU_CHAIN MENU_TEMPLATE MENU_CROSSWORD MENU_CHARACTER_SEQUENCE
	if (m_entryText.empty()) {
		return false;
	}

	if (m_menuClick == MENU_REGULAR_EXPRESSIONS) {//finish with MENU_REGULAR_EXPRESSIONS
		//russain char to lowercase, other ignorecase options in regcomp/g_regex_new functions
		m_entryText = localeToLowerCase(m_entryText, true);
#ifdef _WIN32
		//Note G_REGEX_RAW support 's' in locale, otherwise 's' should be a utf8 string
		m_regex = g_regex_new(m_entryText.c_str(),
				GRegexCompileFlags(G_REGEX_RAW | G_REGEX_CASELESS),
				GRegexMatchFlags(0), NULL);
		if (m_regex == NULL) {
			return false;
		}
#else
		try {
			m_regex=std::regex(m_entryText.c_str(),std::regex_constants::icase | std::regex_constants::extended );
		}
		catch (std::regex_error&) {
			return false;
		}
#endif

		return true;
	}

	//MENU_MODIFICATION MENU_CHAIN MENU_TEMPLATE MENU_CROSSWORD MENU_CHARACTER_SEQUENCE
	m_entryText = localeToLowerCase(m_entryText);

	if (m_menuClick == MENU_MODIFICATION) {		//finish with MENU_MODIFICATION
		if (!m_modifications.parse(m_entryText)) {
#ifdef CGI
			printf(m_cgiLanguage[CGI_STRING_ERROR_INVALID_MODIFICATION_STRING].c_str());
#endif
			return false;
		}
		return true;
	}

	//MENU_CHAIN MENU_TEMPLATE MENU_CROSSWORD MENU_CHARACTER_SEQUENCE
	s = getAlphabet();
	if (m_menuClick == MENU_CROSSWORD) {
		s += '*';
	} else if (m_menuClick == MENU_CHAIN) {
		s += ' ';
	}
	if (!spanIncluding(m_entryText.c_str(), s)) {
		return false;
	}

	if (m_menuClick == MENU_TEMPLATE) {
		memset(m_templateHelper, 0, 256);
		const char *p;
		for (p = m_entryText.c_str(); *p != 0; p++) {
			m_templateHelper[uchar(*p)]++;
		}

		size_t i, j;
		clear_m_template_a();
		m_template_a = new char[m_entryText.length() * 256];
		for (j = 0; j < m_entryText.length(); j++) {
			auto a = m_template_a + j * 256;
			memset(a, 0, 256);
			s = m_entryText.substr(j);
			for (i = 0; i < s.length(); i++) {
				uchar u = s[i];
				if (!a[u]) {
					a[u] = i + 1;
				}
			}
		}
	} else if (m_menuClick == MENU_CHAIN) {	//all symbols from alphabet or spaces
		//m_chainHelper is only from alphabet checked
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
				if (i == 1) {		//only two words allowed
					if (s.find_first_not_of(' ', pe + 1) != std::string::npos) {
						return false;
					}
				}
			}
		}

		if (m_chainHelper[0] == m_chainHelper[1]
				|| m_chainHelper[0].length() != m_chainHelper[1].length()) {
			return false;
		}

		return true;
	}

	return true;
}

std::string WordsBase::getStatusString() {
	std::string s = m_addstatus;
	if (!m_outSplitted) {
		s = m_language[NUMBER_OF_WORDS] + " "
				+ intToStringLocaled(m_result.size()) + ", ";
#ifndef CGI
		s += m_language[WITH_FILTER] + " "
				+ intToStringLocaled(m_filteredWordsCount) + ", ";
#endif
	}
	return s + m_language[TIME_OF_LAST_OPERATION] + " " + getTimeString();
}

std::string WordsBase::getTimeString() {
	return format("%.2lf", double(m_end - m_begin) / CLOCKS_PER_SEC);
}

void WordsBase::removeLastCRLF(char *p) {
	int i = strlen(p);
	if (i == 0) {
		return;
	}
	p += i - 1;
	if (*p == '\n') {
#ifdef __unix__
		if(i>1 && p[-1]=='\r'){
			p--;
		}
#endif
		*p = 0;
	}
}

bool WordsBase::run() {
	StringSet const &r = getDictionary();
	int i;

	if ((i = INDEX_OF(m_menuClick, BOOL_VOID_MENU)) != -1) {
		return (this->*BOOL_VOID_FUNCTION[i])();
	}

	i = INDEX_OF(m_menuClick, FUNCTION_MENU);
	assert(i!=-1);
	BOOL_STRING_WORDSBASE_FUNCTION searchFunction = FUNCTION_ID[i];

	for (auto &e : r) {
		if ((this->*searchFunction)(e)) {
			m_result.push_back(SearchResult(e, e.length(), 1));
		}

		RETURN_ON_USER_BREAK(true)
	}

	if (m_menuClick == MENU_REGULAR_EXPRESSIONS) {
#ifndef CGI
		g_regex_unref(m_regex);
#endif
	}

	return false;
}

bool WordsBase::differenceOnlyOneChar(const std::string &a,
		const std::string &b) {
	StringCI ia, ib;
	int i = 0;
	for (ia = a.begin(), ib = b.begin(); ia != a.end(); ia++, ib++) {
		if (*ia != *ib) {
			i++;
			if (i > 1) {
				return false;
			}
		}
	}
	return i == 1;

}

#ifndef CGI
bool WordsBase::setCheckFilterRegex() {
	if (m_filterText.empty()) {
		return true;
	}
	//need case insensitive filter („астота букв), work ok in russian only for utf8
	auto s = localeToUtf8(m_filterText);
	m_filterRegex = g_regex_new(s.c_str(), GRegexCompileFlags(G_REGEX_CASELESS),
			GRegexMatchFlags(0), NULL);
	return m_filterRegex != nullptr;
}

bool WordsBase::testFilterRegex(const std::string &s) {
	return m_filterRegex == nullptr || m_filterText.empty()
			|| g_regex_match(m_filterRegex, s.c_str(), GRegexMatchFlags(0),
			NULL);
}
#endif

std::string WordsBase::intToStringLocaled(int v) {
	return toString(v, m_language[SEPARATOR_SYMBOL][0]);
}

FILE* WordsBase::open(int i, std::string s, int mode/*=0*/) {
	std::string p = getResourcePath(
			getShortLanguageString(i) + "/" + s + ".txt");
	return ::open(p, mode == 2 ? "wb" : (mode ? "rb" : "r"));
}

#ifdef CGI
std::string WordsBase::getResourcePath(std::string name){
		return

#ifdef _WIN32
 "C:/Users/user/git/words"
#else
 "../htdocs"
#endif
+std::string("/words/words/")
				+name;
	}

void WordsBase::cgi(){
	int i,j;
	std::string s;
	m_begin = clock();
	Cgi c;
	for (auto a : c) {
		i = INDEX_OF(a.first,POST_NAME);
		//assert(i!=POST_SIZE); later in switch
		s=a.second;
		if(i!=POST_ENTRY){
			//std::stoi throws exception if not valid number
			j=std::stoi(s.c_str());
		}
		switch(i){
		case POST_SEARCHTYPE:
			m_menuClick=COMBO_MENU[j];
			break;

		case POST_ENTRY:
			m_entryText=utf8ToLocale(s);
			break;

		case POST_DICTIONARY:
			m_comboValue[COMBOBOX_DICTIONARY]=j;
			break;

		case POST_SORTTYPE:
			m_comboValue[COMBOBOX_SORT]=j;
			break;

		case POST_SORTORDER:
			m_comboValue[COMBOBOX_SORT_ORDER]=j;
			break;

		case POST_LANGUAGE:
			m_languageIndex=j;
			break;

		case POST_COMBO0:
		case POST_COMBO1:
		case POST_COMBO2:
			m_comboValue[COMBOBOX_HELPER0+i-POST_COMBO0]=j;
			break;

		case POST_CHECK:
			m_checkValue= j!=0;
			break;

		default:
			assert(0);
		}
	}

	loadLanguage();//prepare can output error message so load before

	if(!prepare()){
		printf(" %s",getTimeString().c_str());
		return;
	}

	run();
	sortFilterResults();
	m_end=clock();
	printf("%s\n%s",getStatusString().c_str(),m_out.c_str());
}

#endif

const std::string invalidDifference = "$";
std::vector<std::map<std::string, std::vector<std::string>>> eqmap;

std::string sub(std::string const &minuend, std::string const &subtrahend) {
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

std::string getOrderedString(std::string const &s) {
	auto o = s;
	std::sort(o.begin(), o.end());
	return o;
}

//get list of dictionary words from ordered string
std::string getUserString(std::string const &s) {
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

std::vector<std::pair<std::string, std::string>> getAllPairs(
		std::string const &s, std::string const &low = invalidDifference) {
	std::vector<std::pair<std::string, std::string>> v;
	for (size_t i = 1; i < s.size(); i++) {
		for (auto &e : eqmap[i]) {
			if (low == invalidDifference || low <= e.first) {
				auto a = sub(s, e.first);
				if (a != invalidDifference && e.first <= a) {
					auto &m = eqmap[a.length()];
					if (m.find(a) != m.end()) {
						v.push_back(
								{ getUserString(e.first), getUserString(a) });
					}
				}
			}
		}
	}
	return v;
}

//output all pairs to string
std::string pairsToString(
		std::vector<std::pair<std::string, std::string>> const &v, bool p = 0) {
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

bool WordsBase::findLetterGroupSplit() {
	std::string s, s1, t, lng;
	size_t i, j;
	StringSet const &r = getDictionary();
	auto charset = getOrderedString(m_entryText);

	const size_t size = m_entryText.length();
//#ifndef CGI
//	size_t sz[size];
//#endif
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
					m.insert( { s1, { s } });
				}
			} else {
				it->second.push_back(s);
			}
		}
	}

	auto v = getAllPairs(charset);
	size_t n[] = { v.size(), 0 };

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
					m_out += localeToUtf8(
							getUserString(e.first) + " "
									+ pairsToString(v, v.size() != 1));
				}
			}
		}
	}
	if (m_out.empty()) {
		m_out = m_language[SPLITS_NOT_FOUND];
	}
	else{
		m_addstatus = "";
		for (i = 0; i < 2; i++) {
			m_addstatus += m_language[i ? TRIPLETS : PAIRS] + " "
					+ intToStringLocaled(n[i]) + " ";
		}
	}

	return false;
}

void WordsBase::clear_m_template_a(){
	if (m_template_a) {
		delete[] m_template_a;
	}
}
