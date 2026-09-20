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
