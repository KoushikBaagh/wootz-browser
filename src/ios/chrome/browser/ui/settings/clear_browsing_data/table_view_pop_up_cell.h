// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_SETTINGS_CLEAR_BROWSING_DATA_TABLE_VIEW_POP_UP_CELL_H_
#define IOS_CHROME_BROWSER_UI_SETTINGS_CLEAR_BROWSING_DATA_TABLE_VIEW_POP_UP_CELL_H_

#import <UIKit/UIKit.h>

// TableViewCell that displays a pop-up menu bottom on the right side and a one
// line title on the left side.
@interface TableViewPopUpCell : UITableViewCell

// Menu to display when the cell is touched. The UIAction selected is displayed
// by a UIButton on the right side. By default, if no UIAction is selected, then
// UIButton displays the first UIAction.
- (void)setMenu:(UIMenu*)menu;

// Title to display. Optional. Max of one line. Displayed on the left side of
// the cell.
- (void)setTitle:(NSString*)title;

@end

#endif  // IOS_CHROME_BROWSER_UI_SETTINGS_CLEAR_BROWSING_DATA_TABLE_VIEW_POP_UP_CELL_H_
