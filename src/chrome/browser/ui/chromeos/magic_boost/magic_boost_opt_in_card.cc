// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/chromeos/magic_boost/magic_boost_opt_in_card.h"

#include <string>

#include "chrome/browser/ui/views/editor_menu/utils/utils.h"
#include "components/vector_icons/vector_icons.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/compositor/layer.h"
#include "ui/gfx/font.h"
#include "ui/gfx/font_list.h"
#include "ui/gfx/geometry/insets.h"
#include "ui/views/background.h"
#include "ui/views/controls/button/md_text_button.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/controls/label.h"
#include "ui/views/layout/box_layout_view.h"
#include "ui/views/layout/flex_layout.h"
#include "ui/views/layout/flex_layout_view.h"
#include "ui/views/view.h"
#include "ui/views/view_class_properties.h"
#include "ui/views/widget/unique_widget_ptr.h"

namespace chromeos {

namespace {

// Widget constants
constexpr char kWidgetName[] = "MagicBoostOptInWidget";

// Card constants
constexpr gfx::Insets kInteriorMargin = gfx::Insets(16);
constexpr float kCornerRadius = 8.0f;

// Label constants
constexpr int kTitleLabelMaxLines = 2;
constexpr int kBodyLabelMaxLines = 3;

// Image constants
constexpr int kImageViewSize = 36;
constexpr int kImageViewCornerRadius = 12;
constexpr int kImageViewIconSize = 20;

// Button constants
constexpr int kButtonsContainerHeight = 32;

// Spacing constants
constexpr int kBetweenButtonsSpacing = 8;
constexpr int kBetweenImageAndTextSpacing = 16;
constexpr int kBetweenContentsAndButtonsSpacing = 16;
constexpr int kBetweenLabelsSpacing = 4;

// Placeholder values
// TODO(b/339528642): Resolve with real strings.
const std::u16string title_text = u"Title text";
const std::u16string body_text =
    u"Body text that is multi-line which means it can span from one line to up "
    u"to three lines for this case";
const std::u16string secondary_button_text = u"No thanks";
const std::u16string primary_button_text = u"Try it";

// Placeholder callbacks
// TODO(b/340940507): Implement button callbacks.
base::RepeatingClosure primary_callback =
    base::BindRepeating([]() { LOG(ERROR) << "Primary button pressed"; });
base::RepeatingClosure secondary_callback =
    base::BindRepeating([]() { LOG(ERROR) << "Secondary button pressed"; });

// Font lists
const gfx::FontList body_text_font_list =
    gfx::FontList({"Google Sans", "Roboto"},
                  gfx::Font::NORMAL,
                  /*font_size=*/12,
                  gfx::Font::Weight::NORMAL);
const gfx::FontList title_text_font_list =
    gfx::FontList({"Google Sans", "Roboto"},
                  gfx::Font::NORMAL,
                  /*font_size=*/14,
                  gfx::Font::Weight::MEDIUM);

}  // namespace

// MagicBoostOptInCard --------------------------------------------------------

MagicBoostOptInCard::MagicBoostOptInCard() {
  SetLayoutManager(std::make_unique<views::FlexLayout>())
      ->SetOrientation(views::LayoutOrientation::kVertical)
      .SetInteriorMargin(kInteriorMargin)
      .SetDefault(views::kMarginsKey,
                  gfx::Insets::VH(kBetweenContentsAndButtonsSpacing, 0))
      .SetCollapseMargins(true)
      .SetIgnoreDefaultMainAxisMargins(true);
  SetBackground(
      views::CreateThemedSolidBackground(ui::kColorPrimaryBackground));

  // Painted to layer so view can be semi-transparent and set rounded corners.
  SetPaintToLayer();
  layer()->SetFillsBoundsOpaquely(false);
  layer()->SetRoundedCornerRadius(gfx::RoundedCornersF(kCornerRadius));

  // Create image and text container and add an image view.
  auto* image_and_text_container = AddChildView(
      views::Builder<views::FlexLayoutView>()
          .SetOrientation(views::LayoutOrientation::kHorizontal)
          .SetCrossAxisAlignment(views::LayoutAlignment::kStart)
          .CustomConfigure(base::BindOnce([](views::FlexLayoutView* layout) {
            layout->SetDefault(views::kMarginsKey,
                               gfx::Insets::VH(0, kBetweenImageAndTextSpacing));
          }))
          .SetCollapseMargins(true)
          .SetIgnoreDefaultMainAxisMargins(true)
          // Set FlexSpecification to `kUnbounded` so the body text can take up
          // more height when it's multi-line.
          .SetProperty(
              views::kFlexBehaviorKey,
              views::FlexSpecification(views::MinimumFlexSizeRule::kPreferred,
                                       views::MaximumFlexSizeRule::kUnbounded,
                                       /*adjust_height_for_width=*/true))
          .AddChildren(
              views::Builder<views::ImageView>()
                  .SetPreferredSize(gfx::Size(kImageViewSize, kImageViewSize))
                  .SetImage(ui::ImageModel::FromVectorIcon(
                      vector_icons::kUsbIcon, ui::kColorSysOnPrimaryContainer,
                      kImageViewIconSize))
                  .SetBackground(views::CreateThemedSolidBackground(
                      ui::kColorSysPrimaryContainer))
                  // Painted to layer to set rounded corners.
                  .SetPaintToLayer()
                  .CustomConfigure(
                      base::BindOnce([](views::ImageView* image_view) {
                        image_view->layer()->SetFillsBoundsOpaquely(false);
                        image_view->layer()->SetRoundedCornerRadius(
                            gfx::RoundedCornersF(kImageViewCornerRadius));
                      })))
          .Build());

  // Create text container that holds title and body text.
  image_and_text_container->AddChildView(
      views::Builder<views::FlexLayoutView>()
          .SetOrientation(views::LayoutOrientation::kVertical)
          .CustomConfigure(base::BindOnce([](views::FlexLayoutView* layout) {
            layout->SetDefault(views::kMarginsKey,
                               gfx::Insets::VH(kBetweenLabelsSpacing, 0));
          }))
          .SetCollapseMargins(true)
          .SetIgnoreDefaultMainAxisMargins(true)
          // Set FlexSpecification to `kUnbounded` so the body text can
          // take up more height when it's multi-line.
          .SetProperty(
              views::kFlexBehaviorKey,
              views::FlexSpecification(views::MinimumFlexSizeRule::kScaleToZero,
                                       views::MaximumFlexSizeRule::kUnbounded,
                                       /*adjust_height_for_width=*/true))
          .AddChildren(views::Builder<views::Label>()
                           .SetText(title_text)
                           .SetHorizontalAlignment(gfx::ALIGN_LEFT)
                           .SetEnabledColorId(ui::kColorSysOnSurface)
                           .SetAutoColorReadabilityEnabled(false)
                           .SetSubpixelRenderingEnabled(false)
                           .SetFontList(title_text_font_list)
                           .SetMultiLine(true)
                           .SetMaxLines(kTitleLabelMaxLines),
                       views::Builder<views::Label>()
                           .SetText(body_text)
                           .SetHorizontalAlignment(gfx::ALIGN_LEFT)
                           .SetEnabledColorId(ui::kColorSysOnSurface)
                           .SetAutoColorReadabilityEnabled(false)
                           .SetSubpixelRenderingEnabled(false)
                           .SetFontList(body_text_font_list)
                           .SetMultiLine(true)
                           .SetMaxLines(kBodyLabelMaxLines))
          .Build());

  // Create buttons container that holds two buttons.
  AddChildView(
      views::Builder<views::BoxLayoutView>()
          .SetMainAxisAlignment(views::LayoutAlignment::kEnd)
          .SetBetweenChildSpacing(kBetweenButtonsSpacing)
          // Set a preferred size so buttons can adjust to the desired height,
          // instead of the default height set by the `MdTextButton` class.
          .SetPreferredSize(gfx::Size(image_and_text_container->width(),
                                      kButtonsContainerHeight))
          .AddChildren(views::Builder<views::MdTextButton>()
                           .CopyAddressTo(&secondary_button_)
                           .SetText(secondary_button_text)
                           .SetAccessibleName(secondary_button_text)
                           .SetStyle(ui::ButtonStyle::kText)
                           .SetCallback(std::move(secondary_callback)),
                       views::Builder<views::MdTextButton>()
                           .SetText(primary_button_text)
                           .SetAccessibleName(primary_button_text)
                           .SetStyle(ui::ButtonStyle::kProminent)
                           .SetCallback(std::move(primary_callback)))
          .Build());
}

MagicBoostOptInCard::~MagicBoostOptInCard() = default;

// static
views::UniqueWidgetPtr MagicBoostOptInCard::CreateWidget(
    const gfx::Rect& anchor_view_bounds) {
  views::Widget::InitParams params;
  params.opacity = views::Widget::InitParams::WindowOpacity::kTranslucent;
  params.activatable = views::Widget::InitParams::Activatable::kYes;
  params.shadow_elevation = 2;
  params.shadow_type = views::Widget::InitParams::ShadowType::kDrop;
  params.type = views::Widget::InitParams::TYPE_POPUP;
  params.z_order = ui::ZOrderLevel::kFloatingUIElement;
  params.name = GetWidgetName();

  views::UniqueWidgetPtr widget =
      std::make_unique<views::Widget>(std::move(params));
  MagicBoostOptInCard* magic_boost_opt_in_card =
      widget->SetContentsView(std::make_unique<MagicBoostOptInCard>());
  magic_boost_opt_in_card->UpdateWidgetBounds(anchor_view_bounds);

  return widget;
}

// static
const char* MagicBoostOptInCard::GetWidgetName() {
  return kWidgetName;
}

void MagicBoostOptInCard::UpdateWidgetBounds(
    const gfx::Rect& anchor_view_bounds) {
  // TODO(b/318733414): Move `GetEditorMenuBounds` to a common place to use.
  GetWidget()->SetBounds(
      editor_menu::GetEditorMenuBounds(anchor_view_bounds, this));
}

void MagicBoostOptInCard::RequestFocus() {
  views::View::RequestFocus();
  secondary_button_->RequestFocus();
}

BEGIN_METADATA(MagicBoostOptInCard)
END_METADATA

}  // namespace chromeos
