/*
 * HelperStructs.cpp
 *
 *  Created on: 21.12.2016
 *      Author: alexey slovesnov
 */

#include "HelperStructs.h"

void ThreadResult::clear(int n, int n1) {
  s = "";
  total = 0;
  a.resize(n);
  for (int i = 0; i < n; ++i) {
    a[i].assign(n1 == -1 ? n : n1, 0);
  }
}

void ThreadResult::add(std::vector<IntVector> &e) const {
  size_t i, j;
  for (i = 0; i < e.size(); i++) {
    for (j = 0; j < e[0].size(); j++) {
      e[i][j] += a[i][j];
    }
  }
}

bool sortIntDouble(const IntDouble &r1, const IntDouble &r2) {
  return r1.second > r2.second;
}

bool sortStringInt(const StringInt &r1, const StringInt &r2) {
  return r1.second == r2.second ? (r1.first < r2.first) : r1.second > r2.second;
}

std::string fastLocaleToUtf8(const std::string &src) {
  std::string result;
  result.reserve(src.length() * 2);
  for (uchar c : src) {
    if (c >= 0xF0) {
      result.push_back(0xD1);
      result.push_back(c - 0x70);
    } else if (c >= 0xE0) {
      result.push_back(0xD0);
      result.push_back(c - 0x30);
    } else {
      result.push_back(c);
    }
  }
  return result;
}

std::string fastUtf8ToLocale(const std::string &src) {
  std::string result;
  result.reserve(src.length());
  for (size_t i = 0; i < src.length(); ++i) {
    uchar c = src[i];
    if (c == 0xD0 && (i + 1) < src.length()) {
      uchar next = src[++i];
      result.push_back(next + 0x30);
    } else if (c == 0xD1 && (i + 1) < src.length()) {
      uchar next = src[++i];
      result.push_back(next + 0x70);
    } else {
      result.push_back(c);
    }
  }
  return result;
}

std::string capitalizeFirstUtf8(const std::string& src) {
    if (src.empty()) return src;
    gunichar first_char = g_utf8_get_char(src.c_str());
    gunichar upper_char = g_unichar_toupper(first_char);
    char utf8_buf[6] = {0};
    int len = g_unichar_to_utf8(upper_char, utf8_buf);
    const char* rest_of_string = g_utf8_next_char(src.c_str());
    return std::string(utf8_buf, len) + rest_of_string;
}
