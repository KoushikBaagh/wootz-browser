// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/android/compositor/scene_layer/tab_strip_scene_layer.h"

#include "base/android/jni_android.h"
#include "base/feature_list.h"
#include "cc/resources/scoped_ui_resource.h"
#include "cc/slim/layer.h"
#include "cc/slim/solid_color_layer.h"
#include "cc/slim/ui_resource_layer.h"
#include "chrome/android/chrome_jni_headers/TabStripSceneLayer_jni.h"
#include "chrome/browser/android/compositor/decoration_title.h"
#include "chrome/browser/android/compositor/layer/tab_handle_layer.h"
#include "chrome/browser/android/compositor/layer_title_cache.h"
#include "chrome/browser/flags/android/chrome_feature_list.h"
#include "ui/android/resources/nine_patch_resource.h"
#include "ui/android/resources/resource_manager_impl.h"
#include "ui/base/l10n/l10n_util_android.h"
#include "ui/gfx/geometry/point_f.h"
#include "ui/gfx/geometry/rounded_corners_f.h"
#include "ui/gfx/geometry/transform.h"

using base::android::JavaParamRef;
using base::android::JavaRef;

namespace android {

TabStripSceneLayer::TabStripSceneLayer(JNIEnv* env,
                                       const JavaRef<jobject>& jobj)
    : SceneLayer(env, jobj),
      tab_strip_layer_(cc::slim::SolidColorLayer::Create()),
      scrollable_strip_layer_(cc::slim::Layer::Create()),
      group_indicator_layer_(cc::slim::Layer::Create()),
      new_tab_button_(cc::slim::UIResourceLayer::Create()),
      new_tab_button_background_(cc::slim::UIResourceLayer::Create()),
      left_fade_(cc::slim::UIResourceLayer::Create()),
      right_fade_(cc::slim::UIResourceLayer::Create()),
      left_padding_layer_(cc::slim::SolidColorLayer::Create()),
      right_padding_layer_(cc::slim::SolidColorLayer::Create()),
      model_selector_button_(cc::slim::UIResourceLayer::Create()),
      model_selector_button_background_(cc::slim::UIResourceLayer::Create()),
      scrim_layer_(cc::slim::SolidColorLayer::Create()),
      content_tree_(nullptr) {
  new_tab_button_->SetIsDrawable(true);
  new_tab_button_background_->SetIsDrawable(true);
  model_selector_button_->SetIsDrawable(true);
  model_selector_button_background_->SetIsDrawable(true);

  left_fade_->SetIsDrawable(true);
  right_fade_->SetIsDrawable(true);
  scrim_layer_->SetIsDrawable(true);
  left_padding_layer_->SetIsDrawable(true);
  right_padding_layer_->SetIsDrawable(true);

  // When the ScrollingStripStacker is used, the new tab button and tabs scroll,
  // while the incognito button and left/right fade stay fixed. Put the new tab
  // button and tabs in a separate layer placed visually below the others. Put
  // tab group indicators in a separate layer placed visually below the tabs.
  group_indicator_layer_->SetIsDrawable(true);
  scrollable_strip_layer_->SetIsDrawable(true);
  tab_strip_layer_->SetIsDrawable(true);
  tab_strip_layer_->AddChild(group_indicator_layer_);
  tab_strip_layer_->AddChild(scrollable_strip_layer_);

  tab_strip_layer_->AddChild(left_fade_);
  tab_strip_layer_->AddChild(right_fade_);
  tab_strip_layer_->AddChild(left_padding_layer_);
  tab_strip_layer_->AddChild(right_padding_layer_);
  tab_strip_layer_->AddChild(model_selector_button_);
  tab_strip_layer_->AddChild(model_selector_button_background_);
  model_selector_button_background_->AddChild(model_selector_button_);
  tab_strip_layer_->AddChild(new_tab_button_background_);
  tab_strip_layer_->AddChild(scrim_layer_);

  layer()->AddChild(tab_strip_layer_);
}

TabStripSceneLayer::~TabStripSceneLayer() = default;

void TabStripSceneLayer::SetContentTree(
    JNIEnv* env,
    const JavaParamRef<jobject>& jobj,
    const JavaParamRef<jobject>& jcontent_tree) {
  SceneLayer* content_tree = FromJavaObject(env, jcontent_tree);
  if (content_tree_ &&
      (!content_tree_->layer()->parent() ||
       content_tree_->layer()->parent()->id() != layer()->id()))
    content_tree_ = nullptr;

  if (content_tree != content_tree_) {
    if (content_tree_)
      content_tree_->layer()->RemoveFromParent();
    content_tree_ = content_tree;
    if (content_tree) {
      layer()->InsertChild(content_tree->layer(), 0);
      content_tree->layer()->SetPosition(
          gfx::PointF(0, -layer()->position().y()));
    }
  }
}

void TabStripSceneLayer::BeginBuildingFrame(JNIEnv* env,
                                            const JavaParamRef<jobject>& jobj,
                                            jboolean visible) {
  write_index_ = 0;
  group_write_index_ = 0;
  tab_strip_layer_->SetHideLayerAndSubtree(!visible);
}

void TabStripSceneLayer::FinishBuildingFrame(
    JNIEnv* env,
    const JavaParamRef<jobject>& jobj) {
  if (tab_strip_layer_->hide_layer_and_subtree())
    return;

  for (unsigned i = write_index_; i < tab_handle_layers_.size(); ++i) {
    tab_handle_layers_[i]->layer()->RemoveFromParent();
  }
  for (unsigned i = group_write_index_; i < group_title_layers_.size(); ++i) {
    group_title_layers_[i]->RemoveFromParent();
  }
  for (unsigned i = group_write_index_; i < group_bottom_layers_.size(); ++i) {
    group_bottom_layers_[i]->RemoveFromParent();
  }

  tab_handle_layers_.erase(tab_handle_layers_.begin() + write_index_,
                           tab_handle_layers_.end());
  group_title_layers_.erase(group_title_layers_.begin() + group_write_index_,
                            group_title_layers_.end());
  group_bottom_layers_.erase(group_bottom_layers_.begin() + group_write_index_,
                             group_bottom_layers_.end());
}

void TabStripSceneLayer::UpdateTabStripLayer(JNIEnv* env,
                                             const JavaParamRef<jobject>& jobj,
                                             jint width,
                                             jint height,
                                             jfloat y_offset,
                                             jint background_color,
                                             jint scrim_color,
                                             jfloat scrim_opacity,
                                             jfloat left_padding,
                                             jfloat right_padding,
                                             jfloat top_padding) {
  gfx::RectF content(0, y_offset, width, height);
  layer()->SetPosition(gfx::PointF(0, y_offset));
  tab_strip_layer_->SetBounds(gfx::Size(width, height));
  tab_strip_layer_->SetBackgroundColor(SkColor4f::FromColor(background_color));

  float scrollable_strip_height = height - top_padding;
  scrollable_strip_layer_->SetBounds(gfx::Size(width, scrollable_strip_height));
  scrollable_strip_layer_->SetPosition(gfx::PointF(0, top_padding));

  group_indicator_layer_->SetBounds(gfx::Size(width, scrollable_strip_height));
  group_indicator_layer_->SetPosition(gfx::PointF(0, top_padding));

  // Content tree should not be affected by tab strip scene layer visibility.
  if (content_tree_)
    content_tree_->layer()->SetPosition(gfx::PointF(0, -y_offset));

  // Update left and right padding layers as required.
  if (left_padding == 0) {
    left_padding_layer_->SetHideLayerAndSubtree(true);
  } else {
    left_padding_layer_->SetHideLayerAndSubtree(false);
    left_padding_layer_->SetBounds(gfx::Size(left_padding, height));
    left_padding_layer_->SetBackgroundColor(
        SkColor4f::FromColor(background_color));
  }

  if (right_padding == 0) {
    right_padding_layer_->SetHideLayerAndSubtree(true);
  } else {
    right_padding_layer_->SetHideLayerAndSubtree(false);
    right_padding_layer_->SetBounds(gfx::Size(right_padding, height));
    right_padding_layer_->SetPosition(gfx::PointF(width - right_padding, 0));
    right_padding_layer_->SetBackgroundColor(
        SkColor4f::FromColor(background_color));
  }

  // Hide scrim layer if it's not visible.
  if (scrim_opacity == 0.f) {
    scrim_layer_->SetHideLayerAndSubtree(true);
    return;
  }

  // Set opacity and color
  scrim_layer_->SetOpacity(scrim_opacity);
  scrim_layer_->SetBounds(tab_strip_layer_->bounds());
  scrim_layer_->SetBackgroundColor(SkColor4f::FromColor(scrim_color));

  // Ensure layer is visible.
  scrim_layer_->SetHideLayerAndSubtree(false);
}

void TabStripSceneLayer::UpdateNewTabButton(
    JNIEnv* env,
    const JavaParamRef<jobject>& jobj,
    jint resource_id,
    jint bg_resource_id,
    jboolean should_apply_hover_highlight,
    jfloat x,
    jfloat y,
    jfloat top_padding,
    jfloat touch_target_offset,
    jboolean visible,
    jint tint,
    jint background_tint,
    jfloat button_alpha,
    const JavaParamRef<jobject>& jresource_manager) {
  ui::ResourceManager* resource_manager =
      ui::ResourceManagerImpl::FromJavaObject(jresource_manager);
  ui::Resource* button_resource =
      resource_manager->GetStaticResourceWithTint(resource_id, tint);

  new_tab_button_->SetUIResourceId(button_resource->ui_resource()->id());

  new_tab_button_->SetBounds(button_resource->size());
  new_tab_button_->SetHideLayerAndSubtree(!visible);
  new_tab_button_->SetOpacity(button_alpha);

  float left_offset = touch_target_offset;

  // Set Tab Strip Redesign new tab button background
  ui::Resource* button_background_resource =
      resource_manager->GetStaticResourceWithTint(bg_resource_id,
                                                  background_tint, true);

  float background_left_offset = (button_background_resource->size().width() -
                                  button_resource->size().width()) /
                                 2;
  float background_top_offset = (button_background_resource->size().height() -
                                 button_resource->size().height()) /
                                2;

  // Move new tab button visually towards tabs when tab strip is not full.
  x += left_offset;
  // Move new tab button down with respect to top padding.
  // TODO(crbug/331678185): Use a container layer to position the NTB.
  y += top_padding;

  // Only show button bg if btn is being hovered on.
  if (!should_apply_hover_highlight) {
    new_tab_button_background_->RemoveFromParent();
    tab_strip_layer_->AddChild(new_tab_button_);
    new_tab_button_->SetPosition(
        gfx::PointF(x + background_left_offset, y + background_top_offset));
  } else {
    tab_strip_layer_->AddChild(new_tab_button_background_);
    new_tab_button_background_->SetUIResourceId(
        button_background_resource->ui_resource()->id());
    new_tab_button_background_->SetPosition(gfx::PointF(x, y));
    new_tab_button_background_->SetBounds(button_background_resource->size());
    new_tab_button_background_->SetHideLayerAndSubtree(!visible);
    new_tab_button_background_->SetOpacity(button_alpha);
    new_tab_button_->SetPosition(
        gfx::PointF(background_left_offset, background_top_offset));
    new_tab_button_background_->AddChild(new_tab_button_);
  }
}

void TabStripSceneLayer::UpdateModelSelectorButton(
    JNIEnv* env,
    const JavaParamRef<jobject>& jobj,
    jint resource_id,
    jfloat x,
    jfloat y,
    jfloat width,
    jfloat height,
    jboolean incognito,
    jboolean visible,
    jfloat button_alpha,
    const JavaParamRef<jobject>& jresource_manager) {
  ui::ResourceManager* resource_manager =
      ui::ResourceManagerImpl::FromJavaObject(jresource_manager);
  ui::Resource* button_resource = resource_manager->GetResource(
      ui::ANDROID_RESOURCE_TYPE_STATIC, resource_id);

  model_selector_button_->SetUIResourceId(button_resource->ui_resource()->id());
  float left_offset = (width - button_resource->size().width()) / 2;
  float top_offset = (height - button_resource->size().height()) / 2;
  model_selector_button_->SetPosition(
      gfx::PointF(x + left_offset, y + top_offset));
  model_selector_button_->SetBounds(button_resource->size());
  model_selector_button_->SetHideLayerAndSubtree(!visible);
  model_selector_button_->SetOpacity(button_alpha);
}

void TabStripSceneLayer::UpdateModelSelectorButtonBackground(
    JNIEnv* env,
    const JavaParamRef<jobject>& jobj,
    jint resource_id,
    jint bg_resource_id,
    jfloat x,
    jfloat y,
    jfloat width,
    jfloat height,
    jboolean incognito,
    jboolean visible,
    jint tint,
    jint background_tint,
    jboolean should_apply_hover_highlight,
    jfloat button_alpha,
    const JavaParamRef<jobject>& jresource_manager) {
  ui::ResourceManager* resource_manager =
      ui::ResourceManagerImpl::FromJavaObject(jresource_manager);
  ui::Resource* button_resource;

  // Set Tab Strip Redesign model selector button background
  button_resource =
      resource_manager->GetStaticResourceWithTint(resource_id, tint);

  ui::Resource* button_background_resource =
      resource_manager->GetStaticResourceWithTint(bg_resource_id,
                                                  background_tint, true);

  model_selector_button_->SetUIResourceId(button_resource->ui_resource()->id());
  model_selector_button_background_->SetUIResourceId(
      button_background_resource->ui_resource()->id());

  float background_left_offset = (button_background_resource->size().width() -
                                  button_resource->size().width()) /
                                 2;
  float background_top_offset = (button_background_resource->size().height() -
                                 button_resource->size().height()) /
                                2;

  // Only show button bg if btn style enabled or when the btn is being hovered
  // on.
  if (!should_apply_hover_highlight) {
    model_selector_button_background_->RemoveFromParent();
    model_selector_button_->SetPosition(
        gfx::PointF(x + background_left_offset, y + background_top_offset));
    tab_strip_layer_->AddChild(model_selector_button_);
  } else {
    tab_strip_layer_->AddChild(model_selector_button_background_);
    model_selector_button_background_->SetPosition(gfx::PointF(x, y));

    model_selector_button_background_->SetBounds(
        button_background_resource->size());
    model_selector_button_background_->SetHideLayerAndSubtree(!visible);
    model_selector_button_background_->SetOpacity(button_alpha);
    model_selector_button_->SetPosition(
        gfx::PointF(background_left_offset, background_top_offset));
    model_selector_button_background_->AddChild(model_selector_button_);
  }
  model_selector_button_->SetBounds(button_resource->size());
  model_selector_button_->SetHideLayerAndSubtree(!visible);
  model_selector_button_->SetOpacity(button_alpha);
}

void TabStripSceneLayer::UpdateTabStripLeftFade(
    JNIEnv* env,
    const JavaParamRef<jobject>& jobj,
    jint resource_id,
    jfloat opacity,
    const JavaParamRef<jobject>& jresource_manager,
    jint left_fade_color,
    jfloat left_padding) {
  // Hide layer if it's not visible.
  if (opacity == 0.f) {
    left_fade_->SetHideLayerAndSubtree(true);
    return;
  }

  // Set UI resource.
  ui::ResourceManager* resource_manager =
      ui::ResourceManagerImpl::FromJavaObject(jresource_manager);
  ui::Resource* fade_resource = resource_manager->GetStaticResourceWithTint(
        resource_id, left_fade_color);
  left_fade_->SetUIResourceId(fade_resource->ui_resource()->id());

  // The same resource is used for both left and right fade, so the
  // resource must be mirrored for the left fade.
  gfx::Transform fade_transform = gfx::Transform::MakeScale(-1.0f, 1.0f);
  left_fade_->SetTransform(fade_transform);

  // Set opacity.
  left_fade_->SetOpacity(opacity);

  // Set bounds. Use the parent layer height so the 1px fade resource is
  // stretched vertically.
  float height = tab_strip_layer_->bounds().height();
  left_fade_->SetBounds(gfx::Size(fade_resource->size().width(), height));

  // Set position. The rotation set above requires the layer to be offset
  // by its width in order to display on the left edge.
  left_fade_->SetPosition(
      gfx::PointF(fade_resource->size().width() + left_padding, 0));

  // Ensure layer is visible.
  left_fade_->SetHideLayerAndSubtree(false);
}

void TabStripSceneLayer::UpdateTabStripRightFade(
    JNIEnv* env,
    const JavaParamRef<jobject>& jobj,
    jint resource_id,
    jfloat opacity,
    const JavaParamRef<jobject>& jresource_manager,
    jint right_fade_color,
    jfloat right_padding) {
  // Hide layer if it's not visible.
  if (opacity == 0.f) {
    right_fade_->SetHideLayerAndSubtree(true);
    return;
  }

  // Set UI resource.
  ui::ResourceManager* resource_manager =
      ui::ResourceManagerImpl::FromJavaObject(jresource_manager);
  ui::Resource* fade_resource = resource_manager->GetStaticResourceWithTint(
        resource_id, right_fade_color);
  right_fade_->SetUIResourceId(fade_resource->ui_resource()->id());

  // Set opacity.
  right_fade_->SetOpacity(opacity);

  // Set bounds. Use the parent layer height so the 1px fade resource is
  // stretched vertically.
  float height = tab_strip_layer_->bounds().height();
  right_fade_->SetBounds(gfx::Size(fade_resource->size().width(), height));

  // Set position. The right fade is positioned at the end of the tab strip.
  float x = scrollable_strip_layer_->bounds().width() -
            fade_resource->size().width() - right_padding;
  right_fade_->SetPosition(gfx::PointF(x, 0));

  // Ensure layer is visible.
  right_fade_->SetHideLayerAndSubtree(false);
}

void TabStripSceneLayer::PutStripTabLayer(
    JNIEnv* env,
    const JavaParamRef<jobject>& jobj,
    jint id,
    jint close_resource_id,
    jint close_hover_bg_resource_id,
    jint divider_resource_id,
    jint handle_resource_id,
    jint handle_outline_resource_id,
    jint close_tint,
    jint close_hover_bg_tint,
    jint divider_tint,
    jint handle_tint,
    jint handle_outline_tint,
    jboolean foreground,
    jboolean shouldShowTabOutline,
    jboolean close_pressed,
    jfloat toolbar_width,
    jfloat x,
    jfloat y,
    jfloat width,
    jfloat height,
    jfloat content_offset_y,
    jfloat divider_offset_x,
    jfloat bottom_margin,
    jfloat top_margin,
    jfloat close_button_padding,
    jfloat close_button_alpha,
    jboolean is_start_divider_visible,
    jboolean is_end_divider_visible,
    jboolean is_loading,
    jfloat spinner_rotation,
    jfloat brightness,
    jfloat opacity,
    const JavaParamRef<jobject>& jlayer_title_cache,
    const JavaParamRef<jobject>& jresource_manager) {
  LayerTitleCache* layer_title_cache =
      LayerTitleCache::FromJavaObject(jlayer_title_cache);
  ui::ResourceManager* resource_manager =
      ui::ResourceManagerImpl::FromJavaObject(jresource_manager);
  scoped_refptr<TabHandleLayer> layer = GetNextLayer(layer_title_cache);
  ui::NinePatchResource* tab_handle_resource =
      ui::NinePatchResource::From(resource_manager->GetStaticResourceWithTint(
          handle_resource_id, handle_tint, true));
  ui::NinePatchResource* tab_handle_outline_resource =
      ui::NinePatchResource::From(resource_manager->GetStaticResourceWithTint(
          handle_outline_resource_id, handle_outline_tint));
  ui::Resource* close_button_resource =
      resource_manager->GetStaticResourceWithTint(close_resource_id,
                                                  close_tint);

  ui::Resource* close_button_hover_resource =
      resource_manager->GetStaticResourceWithTint(close_hover_bg_resource_id,
                                                  close_hover_bg_tint, true);

  ui::Resource* divider_resource = resource_manager->GetStaticResourceWithTint(
      divider_resource_id, divider_tint, true);
  layer->SetProperties(
      id, close_button_resource, close_button_hover_resource, divider_resource,
      tab_handle_resource, tab_handle_outline_resource, foreground,
      shouldShowTabOutline, close_pressed, toolbar_width, x, y, width, height,
      content_offset_y, divider_offset_x, bottom_margin, top_margin,
      close_button_padding, close_button_alpha, is_start_divider_visible,
      is_end_divider_visible, is_loading, spinner_rotation, brightness,
      opacity);
}

void TabStripSceneLayer::PutGroupIndicatorLayer(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& jobj,
    jboolean incognito,
    jint id,
    jint tint,
    jfloat x,
    jfloat y,
    jfloat width,
    jfloat height,
    jfloat title_text_padding,
    jfloat corner_radius,
    jfloat bottom_indicator_width,
    jfloat bottom_indicator_height,
    const JavaParamRef<jobject>& jlayer_title_cache) {
  LayerTitleCache* layer_title_cache =
      LayerTitleCache::FromJavaObject(jlayer_title_cache);

  // Reuse existing layer if it exists.
  scoped_refptr<cc::slim::SolidColorLayer> title_indicator_layer =
      GetNextGroupTitleLayer();
  scoped_refptr<cc::slim::SolidColorLayer> bottom_indicator_layer =
      GetNextGroupBottomLayer();
  group_write_index_++;

  // Set title indicator container properties.
  title_indicator_layer->SetPosition(gfx::PointF(x, y));
  title_indicator_layer->SetBounds(gfx::Size(width, height));
  title_indicator_layer->SetRoundedCorner(gfx::RoundedCornersF(
      corner_radius, corner_radius, corner_radius, corner_radius));
  title_indicator_layer->SetBackgroundColor(SkColor4f::FromColor(tint));

  // Set title.
  DecorationTitle* title_layer =
      layer_title_cache->GetGroupTitleLayer(id, incognito);
  if (title_layer) {
    float title_y = (height - title_layer->size().height()) / 2.f;
    title_layer->SetUIResourceIds();
    title_layer->setOpacity(1.0f);
    title_layer->setBounds(gfx::Size(width - (title_text_padding * 2), height));
    title_layer->layer()->SetPosition(gfx::PointF(title_text_padding, title_y));
    if (title_indicator_layer->children().size() == 0) {
      title_indicator_layer->AddChild(title_layer->layer());
    } else {
      title_indicator_layer->ReplaceChild(
          title_indicator_layer->children()[0].get(), title_layer->layer());
    }
  } else {
    title_indicator_layer->RemoveAllChildren();
  }

  // Set bottom indicator properties.
  float bottom_indicator_x = x;
  float bottom_indicator_y =
      group_indicator_layer_->bounds().height() - bottom_indicator_height;
  if (l10n_util::IsLayoutRtl()) {
    bottom_indicator_x -= (bottom_indicator_width - width);
  }

  // Use ceiling value to prevent height float from getting truncated, otherwise
  // it could result in bottom indicator looks thinner than intended in certain
  // screen densities.
  bottom_indicator_layer->SetBounds(
      gfx::Size(bottom_indicator_width, ceil(bottom_indicator_height)));

  // Use the floor value to position vertically to prevent bottom indicator from
  // getting cut off in certain screen densities.
  bottom_indicator_layer->SetPosition(
      gfx::PointF(bottom_indicator_x, floor(bottom_indicator_y)));
  bottom_indicator_layer->SetBackgroundColor(SkColor4f::FromColor(tint));
}

scoped_refptr<TabHandleLayer> TabStripSceneLayer::GetNextLayer(
    LayerTitleCache* layer_title_cache) {
  if (write_index_ < tab_handle_layers_.size())
    return tab_handle_layers_[write_index_++];

  scoped_refptr<TabHandleLayer> layer_tree =
      TabHandleLayer::Create(layer_title_cache);
  tab_handle_layers_.push_back(layer_tree);
  scrollable_strip_layer_->AddChild(layer_tree->layer());
  write_index_++;
  return layer_tree;
}

scoped_refptr<cc::slim::SolidColorLayer>
TabStripSceneLayer::GetNextGroupTitleLayer() {
  if (group_write_index_ < group_title_layers_.size()) {
    return group_title_layers_[group_write_index_];
  }

  scoped_refptr<cc::slim::SolidColorLayer> layer =
      cc::slim::SolidColorLayer::Create();
  layer->SetIsDrawable(true);
  group_title_layers_.push_back(layer);
  group_indicator_layer_->AddChild(layer);
  return layer;
}

scoped_refptr<cc::slim::SolidColorLayer>
TabStripSceneLayer::GetNextGroupBottomLayer() {
  if (group_write_index_ < group_bottom_layers_.size()) {
    return group_bottom_layers_[group_write_index_];
  }

  scoped_refptr<cc::slim::SolidColorLayer> layer =
      cc::slim::SolidColorLayer::Create();
  layer->SetIsDrawable(true);
  group_bottom_layers_.push_back(layer);
  group_indicator_layer_->AddChild(layer);
  return layer;
}

bool TabStripSceneLayer::ShouldShowBackground() {
  if (content_tree_)
    return content_tree_->ShouldShowBackground();
  return SceneLayer::ShouldShowBackground();
}

SkColor TabStripSceneLayer::GetBackgroundColor() {
  if (content_tree_)
    return content_tree_->GetBackgroundColor();
  return SceneLayer::GetBackgroundColor();
}

static jlong JNI_TabStripSceneLayer_Init(JNIEnv* env,
                                         const JavaParamRef<jobject>& jobj) {
  // This will automatically bind to the Java object and pass ownership there.
  TabStripSceneLayer* scene_layer = new TabStripSceneLayer(env, jobj);
  return reinterpret_cast<intptr_t>(scene_layer);
}

}  // namespace android
