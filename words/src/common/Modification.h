/*
 * Modification.h
 *
 *  Created on: 17.11.2017
 *      Author: alexey slovesnov
 */

#pragma once

#include "ModificationItem.h"
#include <vector>

using VectorModificationItem = std::vector<ModificationItem>;

class Modification {
  Parameters *m_parent;
  VectorModificationItem vr;

public:
  Modification(Parameters *parent) : m_parent(parent) {}
  bool parse(const std::string &s);
  std::string apply(const std::string &s,
                    bool everyOperationChangesWord = false);

#ifndef NDEBUG
  std::string applyStepByStep(const std::string &s);
  void info();
#endif
};
