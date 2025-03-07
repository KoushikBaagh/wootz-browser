// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/style/position_try_options.h"

namespace blink {

bool PositionTryOption::operator==(const PositionTryOption& other) const {
  return tactic_list_ == other.tactic_list_ &&
         base::ValuesEquivalent(position_try_name_, other.position_try_name_) &&
         inset_area_ == other.inset_area_;
}

void PositionTryOption::Trace(Visitor* visitor) const {
  visitor->Trace(position_try_name_);
}

bool PositionTryOptions::operator==(const PositionTryOptions& other) const {
  return options_ == other.options_;
}

bool PositionTryOptions::HasPositionTryName(
    const HashSet<AtomicString>& names) const {
  for (const auto& option : options_) {
    if (const ScopedCSSName* scoped_name = option.GetPositionTryName()) {
      if (names.Contains(scoped_name->GetName())) {
        return true;
      }
    }
  }
  return false;
}

void PositionTryOptions::Trace(Visitor* visitor) const {
  visitor->Trace(options_);
}

}  // namespace blink
