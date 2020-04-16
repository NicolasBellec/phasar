#pragma once

#include <initializer_list>
#include <unordered_set>

#include "phasar/PhasarLLVM/DataFlowSolver/IfdsIde/Problems/ConstantPropagation/LCUtils/EdgeValue.h"

namespace psr::LCUtils {
class EdgeValueSet {
  std::unordered_set<EdgeValue> underlying;

public:
  EdgeValueSet();
  template <typename Iter>
  EdgeValueSet(Iter beg, Iter ed) : underlying(beg, ed) {}
  EdgeValueSet(std::initializer_list<EdgeValue> ilist);
  auto begin() -> decltype(underlying.begin());
  auto end() -> decltype(underlying.end());
  auto begin() const -> decltype(underlying.begin());
  auto end() const -> decltype(underlying.end());
  int count(const EdgeValue &ev) const;
  auto find(const EdgeValue &ev) -> decltype(underlying.find(ev));
  auto find(const EdgeValue &ev) const -> decltype(underlying.find(ev));

  size_t size() const;
  auto insert(const EdgeValue &ev) -> decltype(underlying.insert(ev));
  auto insert(EdgeValue &&ev) -> decltype(underlying.insert(ev));
  bool empty() const;
  bool operator==(const EdgeValueSet &other) const;
  bool operator!=(const EdgeValueSet &other) const;
};
} // namespace psr::LCUtils