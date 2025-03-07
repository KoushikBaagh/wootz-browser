// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ash/growth/show_nudge_action_performer.h"

#include <memory>
#include <optional>

#include "base/json/json_reader.h"
#include "base/run_loop.h"
#include "base/scoped_observation.h"
#include "base/strings/stringprintf.h"
#include "chrome/browser/ash/growth/metrics.h"
#include "chrome/browser/ash/growth/mock_ui_performer_observer.h"
#include "content/public/test/browser_task_environment.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

constexpr char kNudgePayloadTemplate[] = R"(
    {
      "title": "title",
      "%s": "text"
    }
)";

}  // namespace

class ShowNudgeActionPerformerTest : public testing::Test {
 public:
  ShowNudgeActionPerformerTest() = default;
  ShowNudgeActionPerformerTest(const ShowNudgeActionPerformerTest&) = delete;
  ShowNudgeActionPerformerTest& operator=(const ShowNudgeActionPerformerTest&) =
      delete;
  ~ShowNudgeActionPerformerTest() override = default;

  void SetUp() override {
    action_ = std::make_unique<ShowNudgeActionPerformer>();
    scoped_observation_.Observe(action_.get());
  }

  void TearDown() override { scoped_observation_.Reset(); }

  ShowNudgeActionPerformer& action() { return *action_; }

  void RunActionPerformerCallback(
      growth::ActionResult result,
      std::optional<growth::ActionResultReason> reason) {
    if (result == growth::ActionResult::kSuccess) {
      std::move(action_success_closure_).Run();
    } else {
      std::move(action_failed_closure_).Run();
    }
  }

  bool VerifyActionResult(bool success) {
    if (success) {
      action_success_run_loop_.Run();
    } else {
      action_failed_run_loop_.Run();
    }
    return true;
  }

 protected:
  MockUiPerformerObserver mock_observer_;

 private:
  content::BrowserTaskEnvironment task_environment_;

  base::RunLoop action_success_run_loop_;
  base::RunLoop action_failed_run_loop_;

  base::OnceClosure action_success_closure_ =
      action_success_run_loop_.QuitClosure();
  base::OnceClosure action_failed_closure_ =
      action_failed_run_loop_.QuitClosure();

  std::unique_ptr<ShowNudgeActionPerformer> action_;

  base::ScopedObservation<UiActionPerformer, UiActionPerformer::Observer>
      scoped_observation_{&mock_observer_};
};

TEST_F(ShowNudgeActionPerformerTest, TestValidPayloadParams) {
  const auto validPayloadParam =
      base::StringPrintf(kNudgePayloadTemplate, "body");
  auto value = base::JSONReader::Read(validPayloadParam);
  ASSERT_TRUE(value.has_value());
  action().Run(
      /*campaign_id=*/1, &value->GetDict(),
      base::BindOnce(&ShowNudgeActionPerformerTest::RunActionPerformerCallback,
                     base::Unretained(this)));

  EXPECT_TRUE(VerifyActionResult(/*success=*/true));
}

TEST_F(ShowNudgeActionPerformerTest, TestInvalidPayloadParams) {
  auto* const inValidOpenUrlParam = "{}";
  auto value = base::JSONReader::Read(inValidOpenUrlParam);
  ASSERT_TRUE(value.has_value());
  action().Run(
      /*campaign_id=*/1, &value->GetDict(),
      base::BindOnce(&ShowNudgeActionPerformerTest::RunActionPerformerCallback,
                     base::Unretained(this)));

  EXPECT_TRUE(VerifyActionResult(/*success=*/false));
}

TEST_F(ShowNudgeActionPerformerTest, TestInvalidPayloadBody) {
  auto const inValidOpenUrlParam =
      base::StringPrintf(kNudgePayloadTemplate, "Body");
  auto value = base::JSONReader::Read(inValidOpenUrlParam);
  ASSERT_TRUE(value.has_value());
  action().Run(
      /*campaign_id=*/1, &value->GetDict(),
      base::BindOnce(&ShowNudgeActionPerformerTest::RunActionPerformerCallback,
                     base::Unretained(this)));

  EXPECT_TRUE(VerifyActionResult(/*success=*/false));
}

TEST_F(ShowNudgeActionPerformerTest, ShouldCallOnReadyToLogImpression) {
  const auto validPayloadParam =
      base::StringPrintf(kNudgePayloadTemplate, "body");
  auto value = base::JSONReader::Read(validPayloadParam);
  ASSERT_TRUE(value.has_value());

  int campaign_id = 100;
  EXPECT_CALL(mock_observer_, OnReadyToLogImpression(testing::Eq(campaign_id)))
      .Times(1);

  action().Run(
      campaign_id, &value->GetDict(),
      base::BindOnce(&ShowNudgeActionPerformerTest::RunActionPerformerCallback,
                     base::Unretained(this)));

  EXPECT_TRUE(VerifyActionResult(/*success=*/true));
}
