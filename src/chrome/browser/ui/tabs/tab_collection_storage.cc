// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "base/notimplemented.h"
#include "chrome/browser/ui/tabs/tab_collection.h"
#include "chrome/browser/ui/tabs/tab_collection_storage.h"
#include "chrome/browser/ui/tabs/tab_model.h"

namespace tabs {

TabCollectionStorage::TabCollectionStorage(TabCollection& owner)
    : owning_collection_(owner) {}

TabCollectionStorage::~TabCollectionStorage() = default;

bool TabCollectionStorage::ContainsTab(TabModel* tab_model) const {
  return GetIndexOfTab(tab_model).has_value();
}

TabModel* TabCollectionStorage::GetTabAtIndex(size_t index) const {
  const std::unique_ptr<tabs::TabModel>* tab =
      std::get_if<std::unique_ptr<tabs::TabModel>>(&children_[index]);
  return tab ? tab->get() : nullptr;
}

bool TabCollectionStorage::ContainsCollection(
    TabCollection* tab_collection) const {
  return GetIndexOfCollection(tab_collection).has_value();
}

TabModel* TabCollectionStorage::AddTab(std::unique_ptr<TabModel> tab_model,
                                       size_t index) {
  CHECK(index <= GetChildrenCount());
  CHECK(tab_model);

  TabModel* tab_model_ptr = tab_model.get();
  children_.insert(children_.begin() + index, std::move(tab_model));
  return tab_model_ptr;
}

void TabCollectionStorage::MoveTab(TabModel* tab_model, size_t dst_index) {
  CHECK(tab_model);
  std::unique_ptr<TabModel> tab_model_to_move = RemoveTab(tab_model);
  CHECK(tab_model_to_move);
  AddTab(std::move(tab_model_to_move), dst_index);
}

std::unique_ptr<TabModel> TabCollectionStorage::RemoveTab(TabModel* tab_model) {
  CHECK(tab_model);
  for (size_t i = 0; i < children_.size(); ++i) {
    if (std::holds_alternative<std::unique_ptr<TabModel>>(children_[i])) {
      std::unique_ptr<TabModel>& stored_tab_model =
          std::get<std::unique_ptr<TabModel>>(children_[i]);
      if (stored_tab_model.get() == tab_model) {
        auto removed_tab_model = std::move(stored_tab_model);
        children_.erase(children_.begin() + i);
        return removed_tab_model;
      }
    }
  }
  NOTREACHED_NORETURN();
}

void TabCollectionStorage::CloseTab(TabModel* tab) {
  std::unique_ptr<TabModel> removed_tab_model = RemoveTab(tab);
  removed_tab_model.reset();
}

TabCollection* TabCollectionStorage::AddCollection(
    std::unique_ptr<TabCollection> collection,
    size_t index) {
  CHECK(index <= GetChildrenCount());
  CHECK(collection);

  TabCollection* collection_ptr = collection.get();
  children_.insert(children_.begin() + index, std::move(collection));
  return collection_ptr;
}

void TabCollectionStorage::MoveCollection(TabCollection* collection,
                                          size_t dst_index) {
  CHECK(collection);
  std::unique_ptr<TabCollection> tab_collection_to_move =
      RemoveCollection(collection);
  CHECK(tab_collection_to_move);
  AddCollection(std::move(tab_collection_to_move), dst_index);
}

std::unique_ptr<TabCollection> TabCollectionStorage::RemoveCollection(
    TabCollection* collection) {
  CHECK(collection);
  for (size_t i = 0; i < children_.size(); ++i) {
    if (std::holds_alternative<std::unique_ptr<TabCollection>>(children_[i])) {
      std::unique_ptr<TabCollection>& stored_tab_collection =
          std::get<std::unique_ptr<TabCollection>>(children_[i]);
      if (stored_tab_collection.get() == collection) {
        auto removed_tab_collection = std::move(stored_tab_collection);
        children_.erase(children_.begin() + i);
        return removed_tab_collection;
      }
    }
  }
  NOTREACHED_NORETURN();
}

void TabCollectionStorage::CloseCollection(TabCollection* collection) {
  // This should free all the children as well.
  std::unique_ptr<TabCollection> removed_tab_collection =
      RemoveCollection(collection);
  removed_tab_collection.reset();
}

std::optional<size_t> TabCollectionStorage::GetIndexOfTab(
    const TabModel* const tab_model) const {
  const auto it = std::find_if(
      children_.begin(), children_.end(), [tab_model](const auto& child) {
        return std::holds_alternative<std::unique_ptr<TabModel>>(child) &&
               std::get<std::unique_ptr<TabModel>>(child).get() == tab_model;
      });
  return it == children_.end() ? std::nullopt
                               : std::optional<size_t>(it - children_.begin());
}

std::optional<size_t> TabCollectionStorage::GetIndexOfCollection(
    TabCollection* tab_collection) const {
  const auto it = std::find_if(
      children_.begin(), children_.end(), [tab_collection](const auto& child) {
        return std::holds_alternative<std::unique_ptr<TabCollection>>(child) &&
               std::get<std::unique_ptr<TabCollection>>(child).get() ==
                   tab_collection;
      });
  return it == children_.end() ? std::nullopt
                               : std::optional<size_t>(it - children_.begin());
}

size_t TabCollectionStorage::GetChildrenCount() const {
  return children_.size();
}
}  // namespace tabs
