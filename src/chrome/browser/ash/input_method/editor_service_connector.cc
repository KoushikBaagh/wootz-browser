// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/input_method/editor_service_connector.h"

#include <vector>

#include "ash/constants/ash_features.h"
#include "base/feature_list.h"
#include "chrome/browser/ash/input_method/editor_helpers.h"
#include "chrome/browser/browser_process.h"
#include "chromeos/ash/services/orca/public/mojom/orca_service.mojom.h"
#include "content/public/browser/service_process_host.h"

namespace ash::input_method {
namespace {

constexpr char kDefaultLanguageCode[] = "en";

orca::mojom::EditorConfigPtr GenerateConfig() {
  std::vector<orca::mojom::PresetTextQueryType> allowed;
  if (base::FeatureList::IsEnabled(features::kOrcaElaborate)) {
    allowed.push_back(orca::mojom::PresetTextQueryType::kElaborate);
  }
  if (base::FeatureList::IsEnabled(features::kOrcaEmojify)) {
    allowed.push_back(orca::mojom::PresetTextQueryType::kEmojify);
  }
  if (base::FeatureList::IsEnabled(features::kOrcaFormalize)) {
    allowed.push_back(orca::mojom::PresetTextQueryType::kFormalize);
  }
  if (base::FeatureList::IsEnabled(features::kOrcaProofread)) {
    allowed.push_back(orca::mojom::PresetTextQueryType::kProofread);
  }
  if (base::FeatureList::IsEnabled(features::kOrcaRephrase)) {
    allowed.push_back(orca::mojom::PresetTextQueryType::kRephrase);
  }
  if (base::FeatureList::IsEnabled(features::kOrcaShorten)) {
    allowed.push_back(orca::mojom::PresetTextQueryType::kShorten);
  }
  return orca::mojom::EditorConfig::New(
      /*allowed_types=*/std::move(allowed),
      /*language_code=*/ShouldUseL10nStrings()
          ? GetSystemLocale()
          : std::string{kDefaultLanguageCode});
}

}  // namespace

EditorServiceConnector::EditorServiceConnector() {}

EditorServiceConnector::~EditorServiceConnector() = default;

bool EditorServiceConnector::SetUpNewEditorService() {
  if (IsBound()) {
    return false;
  }

  content::ServiceProcessHost::Launch(
      remote_orca_service_connector_.BindNewPipeAndPassReceiver(),
      content::ServiceProcessHost::Options()
          // replace with IDS strings
          .WithDisplayName("EditorService")
          .Pass());

  remote_orca_service_connector_.reset_on_disconnect();

  return true;
}

void EditorServiceConnector::BindEditor(
    mojo::PendingAssociatedReceiver<orca::mojom::EditorClientConnector>
        editor_client_connector,
    mojo::PendingAssociatedReceiver<orca::mojom::EditorEventSink>
        editor_event_sink,
    mojo::PendingAssociatedRemote<orca::mojom::SystemActuator> system_actuator,
    mojo::PendingAssociatedRemote<orca::mojom::TextQueryProvider>
        text_query_provider) {
  remote_orca_service_connector_->BindEditor(
      std::move(system_actuator), std::move(text_query_provider),
      std::move(editor_client_connector), std::move(editor_event_sink),
      GenerateConfig());
}

bool EditorServiceConnector::IsBound() {
  return remote_orca_service_connector_ &&
         remote_orca_service_connector_.is_bound();
}

}  // namespace ash::input_method
