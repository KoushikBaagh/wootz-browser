// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/snapshot/snapshot.h"

#import <Cocoa/Cocoa.h>

#include <memory>

#include "base/mac/mac_util.h"
#include "base/test/task_environment.h"
#include "base/test/test_future.h"
#include "testing/platform_test.h"
#import "ui/base/test/cocoa_helper.h"
#import "ui/base/test/windowed_nsnotification_observer.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/image/image.h"
#include "ui/snapshot/snapshot_mac.h"

namespace ui {
namespace {

class GrabWindowSnapshotTest : public CocoaTest,
                               public testing::WithParamInterface<SnapshotAPI> {
 private:
  base::test::TaskEnvironment task_environment_{
      base::test::TaskEnvironment::MainThreadType::UI};
};

INSTANTIATE_TEST_SUITE_P(Snapshot,
                         GrabWindowSnapshotTest,
                         ::testing::Values(SnapshotAPI::kOldAPI,
                                           SnapshotAPI::kNewAPI));

TEST_P(GrabWindowSnapshotTest, TestGrabWindowSnapshot) {
  SnapshotAPI api = GetParam();
  if (api == SnapshotAPI::kNewAPI && base::mac::MacOSVersion() < 14'04'00) {
    GTEST_SKIP() << "Cannot test macOS 14.4 API on pre-14.4 macOS";
  }
  if (api == SnapshotAPI::kNewAPI) {
    GTEST_SKIP() << "https://crbug.com/335449467";
  }

  ForceAPIUsageForTesting(api);

  // The window snapshot code uses `CGWindowListCreateImage` which requires
  // going to the windowserver. By default, unittests are run with the
  // `NSApplicationActivationPolicyProhibited` policy which prohibits
  // windowserver connections, which would cause this test to fail for reasons
  // other than the code not actually working.
  NSApp.activationPolicy = NSApplicationActivationPolicyAccessory;

  // Launch a test window so we can take a snapshot.
  const NSUInteger window_size = 400;
  NSRect frame = NSMakeRect(0, 0, window_size, window_size);
  NSWindow* window = test_window();
  WindowedNSNotificationObserver* waiter =
      [[WindowedNSNotificationObserver alloc]
          initForNotification:NSWindowDidUpdateNotification
                       object:window];
  [window setFrame:frame display:false];
  window.backgroundColor = NSColor.blueColor;
  [window makeKeyAndOrderFront:nil];
  [window display];
  EXPECT_TRUE([waiter wait]);

  // Take the snapshot.
  base::test::TestFuture<gfx::Image> future;
  gfx::Rect bounds = gfx::Rect(0, 0, window_size, window_size);
  ui::GrabWindowSnapshot(window, bounds, future.GetCallback());

  gfx::Image image = future.Take();
  ASSERT_TRUE(!image.IsEmpty());

  // The call to `CGWindowListCreateImage` returned a `CGImageRef` that is
  // wrapped in an `NSImage` (inside the returned `gfx::Image`). The image rep
  // that results (e.g. an `NSCGImageSnapshotRep` in macOS 12) isn't anything
  // that pixel values can be retrieved from, so do a quick-and-dirty conversion
  // to an `NSBitmapImageRep`.
  NSBitmapImageRep* image_rep =
      [NSBitmapImageRep imageRepWithData:image.ToNSImage().TIFFRepresentation];

  // Test the size.
  EXPECT_EQ(window_size * window.backingScaleFactor, image_rep.pixelsWide);
  EXPECT_EQ(window_size * window.backingScaleFactor, image_rep.pixelsHigh);

  // Pick a pixel in the middle of the screenshot and expect it to be some
  // version of blue.
  NSColor* color = [image_rep colorAtX:image_rep.pixelsWide / 2
                                     y:image_rep.pixelsHigh / 2];
  CGFloat red = 0, green = 0, blue = 0, alpha = 0;
  [color getRed:&red green:&green blue:&blue alpha:&alpha];
  EXPECT_LE(red, 0.2);
  EXPECT_LE(green, 0.2);
  EXPECT_GE(blue, 0.9);
  EXPECT_EQ(alpha, 1);

  ForceAPIUsageForTesting(SnapshotAPI::kUnspecified);
}

}  // namespace
}  // namespace ui
