/*
 * Modification.cpp
 *
 *  Created on: 17.11.2017
 *      Author: alexey slovesnov
 */

#include "Modification.h"

bool Modification::parse(const std::string &s) {
  ModificationItem mi;
  const char *p = s.c_str(); // string already in lowercase

  // clear previous parsing
  vr.clear();
  while (1) {
    // skip spaces before any search
    ModificationItem::skipSpaces(p);
    if (*p == 0) {
      return !vr.empty();
    }

    if (!mi.parse(p, m_parent)) {
#ifndef NDEBUG
      vr.clear(); // vr.size()==0 means error in info() function
                  // mi.info();
#endif
      return false;
    }

    vr.push_back(mi);
  }
}

std::string Modification::apply(const std::string &s,
                                bool everyOperationChangesWord) {
  std::string r = s, q;
  for (const auto &a : vr) {
    if (everyOperationChangesWord) {
      q = r;
    }
    r = a.apply(r);
    if (r.empty()) {
      return "";
    }
    if (everyOperationChangesWord && q == r) {
      return "";
    }
  }
  return r;
}

#ifndef NDEBUG
std::string Modification::applyStepByStep(const std::string &s) {
  std::string q = s;
  std::string r = s;
  for (const auto &a : vr) {
    r = a.apply(r);
    q += " " + r;
    if (r.empty()) {
      return "";
    }
  }
  return q;
}
#endif

#ifndef NDEBUG
void Modification::info() {
  if (vr.empty()) {
    printf("error");
    return;
  }
  printf("ok sz=%d{\n", int(vr.size())); // 32 & 64bit
  for (const auto &a : vr) {
    a.info();
  }
  printf("}\n");
}
#endif
