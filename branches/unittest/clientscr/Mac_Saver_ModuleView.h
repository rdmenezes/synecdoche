// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2005 University of California
//
// Synecdoche is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Synecdoche is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License with Synecdoche.  If not, see <http://www.gnu.org/licenses/>.

//  Mac_Saver_ModuleView.h
//  BOINC_Saver_Module
//

#import <ScreenSaver/ScreenSaver.h>


@interface BOINC_Saver_ModuleView : ScreenSaverView 
{
    NSString *mBundleID;                        // our bundle ID
    
    IBOutlet id mConfigureSheet;		// our configuration sheet
    IBOutlet NSButton *mGoToBlankCheckbox;
    IBOutlet NSTextField *mBlankingTimeTextField;
    
    int mVersion;               // the version of our prefs
    NSString *mBlankingTimeString;
}

- (IBAction)closeSheetSave:(id) sender;
- (IBAction)closeSheetCancel:(id) sender;

@end

int initBOINCSaver(Boolean ispreview);
int getSSMessage(char **theMessage, int* coveredFreq);
void drawPreview(CGContextRef myContext);
void closeBOINCSaver(void);
extern void print_to_log_file(const char *format, ...);
