/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2010 Rob Buis <buis@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "third_party/blink/renderer/core/svg/svg_tspan_element.h"

#include "third_party/blink/renderer/core/layout/svg/layout_svg_tspan.h"
#include "third_party/blink/renderer/core/svg/svg_a_element.h"
#include "third_party/blink/renderer/core/svg/svg_text_element.h"
#include "third_party/blink/renderer/core/svg/svg_text_path_element.h"
#include "third_party/blink/renderer/core/svg_names.h"

namespace blink {

SVGTSpanElement::SVGTSpanElement(Document& document)
    : SVGTextPositioningElement(svg_names::kTSpanTag, document) {}

LayoutObject* SVGTSpanElement::CreateLayoutObject(const ComputedStyle&) {
  return MakeGarbageCollected<LayoutSVGTSpan>(this);
}

bool SVGTSpanElement::LayoutObjectIsNeeded(const DisplayStyle& style) const {
  if (parentNode() &&
      (IsA<SVGAElement>(*parentNode()) || IsA<SVGTextElement>(*parentNode()) ||
       IsA<SVGTextPathElement>(*parentNode()) ||
       IsA<SVGTSpanElement>(*parentNode())))
    return SVGElement::LayoutObjectIsNeeded(style);

  return false;
}

}  // namespace blink
