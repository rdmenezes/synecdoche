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

//
//  Mac_Saver_ModuleView.m
//  BOINC_Saver_Module
//

#import "Mac_Saver_ModuleView.h"
#include <Carbon/Carbon.h>
#include <AppKit/AppKit.h>

void print_to_log_file(const char *format, ...);
void strip_cr(char *buf);

int gGoToBlank;      // True if we are to blank the screen
int gBlankingTime;   // Delay in minutes before blanking the screen
NSString *gPathToBundleResources;
NSImage *gBOINC_Logo;

NSRect gMovingRect;
float gImageXIndent;
float gTextBoxHeight;
NSPoint gCurrentPosition;
NSPoint gCurrentDelta;

ATSUStyle  theStyle;
ATSUFontID theFontID;
Fixed   atsuSize;
char myFontName[] = "Helvetica";
//char myFontName[] = "Lucida Blackletter";
    
ATSUAttributeTag  theTags[] =  { kATSUFontTag, kATSUSizeTag };
ByteCount        theSizes[] = { sizeof (ATSUFontID), sizeof(Fixed) };
ATSUAttributeValuePtr theValues[] = { &theFontID, &atsuSize };

CGContextRef myContext;
bool isErased;

#define TEXTBOXMINWIDTH 400.0
#define MINTEXTBOXHEIGHT 40.0
#define MAXTEXTBOXHEIGHT 300.0
#define TEXTBOXTOPBORDER 15
#define SAFETYBORDER 20.0
#define MINDELTA 8
#define MAXDELTA 16

int signof(float x) {
    return (x > 0.0 ? 1 : -1);
}

@implementation BOINC_Saver_ModuleView

- (id)initWithFrame:(NSRect)frame isPreview:(BOOL)isPreview {
    NSBundle * myBundle;
    OSStatus err;

    self = [ super initWithFrame:frame isPreview:isPreview ];
    if (gBOINC_Logo == NULL) {
        if (self) {
            myBundle = [ NSBundle bundleForClass:[self class]];
            // grab the screensaver defaults
            mBundleID = [ myBundle bundleIdentifier ];

            // Path to our copy of switcher utility application in this screensaver bundle
            gPathToBundleResources = [ myBundle resourcePath ];
            
            ScreenSaverDefaults *defaults = [ ScreenSaverDefaults defaultsForModuleWithName:mBundleID ];
            
            // try to load the version key, used to see if we have any saved settings
            mVersion = [defaults floatForKey:@"version"];
            if (!mVersion) {
                // no previous settings so define our defaults
                mVersion = 1;
                gGoToBlank = NO;
                gBlankingTime = 1;
                
                // write out the defaults
                [ defaults setInteger:mVersion forKey:@"version" ];
                [ defaults setInteger:gGoToBlank forKey:@"GoToBlank" ];
                [ defaults setInteger:gBlankingTime forKey:@"BlankingTime" ];
                
                // synchronize
                [defaults synchronize];
            }

            // set defaults...
            gGoToBlank = [ defaults integerForKey:@"GoToBlank" ];
            gBlankingTime = [ defaults integerForKey:@"BlankingTime" ];

           [ self setAutoresizesSubviews:YES ];	// make sure the subview resizes.

            NSString *fileName = [[ NSBundle bundleForClass:[ self class ]] pathForImageResource:@"screensaver" ];
            if (! fileName) {
                // What should we do in this case?
                return self;
            }
            
            gBOINC_Logo = [[ NSImage alloc ] initWithContentsOfFile:fileName ];
            gMovingRect.origin.x = 0.0;
            gMovingRect.origin.y = 0.0;
            gMovingRect.size = [gBOINC_Logo size];
            
            if (gMovingRect.size.width < TEXTBOXMINWIDTH) {
                gImageXIndent = (TEXTBOXMINWIDTH - gMovingRect.size.width) / 2;
                gMovingRect.size.width = TEXTBOXMINWIDTH;
            } else {
                gImageXIndent = 0.0;
            }
            gTextBoxHeight = MINTEXTBOXHEIGHT;
            gMovingRect.size.height += gTextBoxHeight;
            gCurrentPosition.x = SAFETYBORDER + 1;
            gCurrentPosition.y = SAFETYBORDER + 1 + gTextBoxHeight;
            gCurrentDelta.x = 1.0;
            gCurrentDelta.y = 1.0;
            
            [ self setAnimationTimeInterval:1/8.0 ];

            ATSUFindFontFromName(myFontName, strlen(myFontName), kFontFamilyName, kFontMacintoshPlatform, 
                                    kFontNoScriptCode, kFontNoLanguageCode, &theFontID);
                                
            err = ATSUCreateStyle(&theStyle);
            atsuSize = Long2Fix (20);
            err = ATSUSetAttributes(theStyle, 2, theTags, theSizes, theValues);
        }
    }
    
    return self;
}

- (void)startAnimation {
    int newFrequency;

    [ super startAnimation ];

    if ( [ self isPreview ] )
        return;
        
    newFrequency = initBOINCSaver([ self isPreview ]);        
    if (newFrequency)
        [ self setAnimationTimeInterval:1.0/newFrequency ];
}

- (void)stopAnimation {
    [ super stopAnimation ];

    if ( [ self isPreview ] )
        return;
    closeBOINCSaver();
}

- (void)drawRect:(NSRect)rect {
    [ super drawRect:rect ];

//  optionally draw here
}

- (void)animateOneFrame {
    int newFrequency = 0;
    int coveredFreq = 0;
    NSRect theFrame = [ self frame ];
    int myWindowNumber;
    int windowList[1];
    NSRect currentDrawingRect, eraseRect;
    NSPoint imagePosition;
    Rect r;
    char *msg;
    CFStringRef cf_msg;
    AbsoluteTime timeToUnblock, frameStartTime = UpTime();
    OSStatus err;

   if ([ self isPreview ]) {
#if 1   // Currently drawRect just draws our logo in the preview window
        NSString *fileName = [[ NSBundle bundleForClass:[ self class ]] pathForImageResource:@"synecdoche" ];
        if (fileName) {
            NSImage *myImage = [[ NSImage alloc ] initWithContentsOfFile:fileName ];
            [ myImage setScalesWhenResized:YES ];
            [ myImage setSize:theFrame.size ];
            [ myImage compositeToPoint:NSZeroPoint operation:NSCompositeSourceOver ];
            [ myImage release ];
        }
        [ self setAnimationTimeInterval:1/1.0 ];
#else   // Code for possible future use if we want to draw more in preview
        myContext = [[NSGraphicsContext currentContext] graphicsPort];
        drawPreview(myContext);        
        [ self setAnimationTimeInterval:1/30.0 ];
#endif
        return;
    }

   myContext = [[NSGraphicsContext currentContext] graphicsPort];
//    [myContext retain];

    NSWindow *myWindow = [ self window ];
    NSRect windowFrame = [ myWindow frame ];
    if ( (windowFrame.origin.x != 0) || (windowFrame.origin.y != 0) ) {
        // Hide window on second display to aid in debugging
//      [[[ NSView focusView] window ] setLevel:kCGMinimumWindowLevel ];
        return;         // We draw only to main screen
    }

    NSRect viewBounds = [self bounds];

    newFrequency = getSSMessage(&msg, &coveredFreq);

    // NOTE: My tests seem to confirm that the first window returned by NSWindowList
    // is always the top window under OS 10.5, but not under earlier systems.  However, 
    // Apple's documentation is unclear whether we can depend on this.  So I have added 
    // some safety by doing two things:
    // [1] Only use the NSWindowList test when we have started project graphics.
    // [2] Assume that our window is covered 45 seconds after starting project 
    //     graphics even if the NSWindowList test did not indicate that is so.
    //
    // getSSMessage() returns a non-zero value for coveredFreq only if we have started 
    // project graphics.
    //
    // If we should use a different frequency when our window is covered by another 
    // window, then check whether there is a window at a higher z-level than ours.
    if (coveredFreq) {
        myWindowNumber = [ myWindow windowNumber ];

        windowList[0] = 0;
        NSWindowList(1, windowList);
        if (windowList[0] != myWindowNumber) {
            // Project graphics application has a window open above ours
            // Don't waste CPU cycles since our window is obscured by application graphics
            newFrequency = coveredFreq;
            msg = NULL;
        }
    }
    
    // Clear the previous drawing area
    currentDrawingRect = gMovingRect;
    currentDrawingRect.origin.x = (float) ((int)gCurrentPosition.x);
    currentDrawingRect.origin.y += (float) ((int)gCurrentPosition.y - gTextBoxHeight);

    if ( (msg != NULL) && (msg[0] != '\0') ) {

        // Set direction of motion to "bounce" off edges of screen
       if (currentDrawingRect.origin.x <= SAFETYBORDER) {
            gCurrentDelta.x = (float)SSRandomIntBetween(MINDELTA, MAXDELTA) / 16.;
            gCurrentDelta.y = (float)(SSRandomIntBetween(MINDELTA, MAXDELTA) * signof(gCurrentDelta.y)) / 16.;
        }
        if ( (currentDrawingRect.origin.x + currentDrawingRect.size.width) >= 
                    (viewBounds.origin.x + viewBounds.size.width - SAFETYBORDER) ) {
            gCurrentDelta.x = -(float)SSRandomIntBetween(MINDELTA, MAXDELTA) / 16.;
            gCurrentDelta.y = (float)(SSRandomIntBetween(MINDELTA, MAXDELTA) * signof(gCurrentDelta.y)) / 16.;
        }
        if (currentDrawingRect.origin.y <= SAFETYBORDER) {
            gCurrentDelta.y = (float)SSRandomIntBetween(MINDELTA, MAXDELTA) / 16.;
            gCurrentDelta.x = (float)(SSRandomIntBetween(MINDELTA, MAXDELTA) * signof(gCurrentDelta.x)) / 16.;
        }
        if ( (currentDrawingRect.origin.y + currentDrawingRect.size.height) >= 
                   (viewBounds.origin.y + viewBounds.size.height - SAFETYBORDER) ) {
            gCurrentDelta.y = -(float)SSRandomIntBetween(MINDELTA, MAXDELTA) / 16.;
            gCurrentDelta.x = (float)(SSRandomIntBetween(MINDELTA, MAXDELTA) * signof(gCurrentDelta.x)) / 16.;
        }
#if 0
        // For testing
        gCurrentDelta.x = 0;
        gCurrentDelta.y = 0;
#endif

    if (!isErased) {
        [[NSColor blackColor] set];
        
        // Erasing only 2 small rectangles reduces screensaver's CPU usage by about 25%
        imagePosition.x = (float) ((int)gCurrentPosition.x + gImageXIndent);
        imagePosition.y = (float) (int)gCurrentPosition.y;
        eraseRect.origin.y = imagePosition.y;
        eraseRect.size.height = currentDrawingRect.size.height - gTextBoxHeight;
        
        if (gCurrentDelta.x > 0) {
            eraseRect.origin.x = imagePosition.x - 1;
            eraseRect.size.width = gCurrentDelta.x + 1;
        } else {
            eraseRect.origin.x = currentDrawingRect.origin.x + currentDrawingRect.size.width - gImageXIndent + gCurrentDelta.x - 1;
            eraseRect.size.width = -gCurrentDelta.x + 1;
        }
        
        eraseRect = NSInsetRect(eraseRect, -1, -1);
        NSRectFill(eraseRect);
        
        eraseRect.origin.x = imagePosition.x;
        eraseRect.size.width = currentDrawingRect.size.width - gImageXIndent - gImageXIndent;

        if (gCurrentDelta.y > 0) {
            eraseRect.origin.y = imagePosition.y;
            eraseRect.size.height = gCurrentDelta.y + 1;
        } else {
            eraseRect.origin.y = imagePosition.y + currentDrawingRect.size.height - gTextBoxHeight - 1;
            eraseRect.size.height = -gCurrentDelta.y + 1;
        }
        eraseRect = NSInsetRect(eraseRect, -1, -1);
        NSRectFill(eraseRect);
        
        eraseRect = currentDrawingRect;
        eraseRect.size.height = gTextBoxHeight;
        eraseRect = NSInsetRect(eraseRect, -1, -1);
        NSRectFill(eraseRect);

        isErased  = true;
    }

        // Get the new drawing area
        gCurrentPosition.x += gCurrentDelta.x;
        gCurrentPosition.y += gCurrentDelta.y;
        
        imagePosition.x = (float) ((int)gCurrentPosition.x + gImageXIndent);
        imagePosition.y = (float) (int)gCurrentPosition.y;
    
        // Calculate QuickDraw Rect for current text box
        r.left = (float) ((int)gCurrentPosition.x);
        r.right = r.left + gMovingRect.size.width;
        r.top = viewBounds.size.height - imagePosition.y;
        r.bottom = r.top + (int)MAXTEXTBOXHEIGHT;
        r.top += TEXTBOXTOPBORDER;        // Add a few pixels space below image
        
        TXNTextBoxOptionsData theOptions = {kTXNUseCGContextRefMask | kTXNSetFlushnessMask, 
                                            kATSUCenterAlignment, kATSUNoJustification, 0, myContext };

        cf_msg = CFStringCreateWithCString(NULL, msg, kCFStringEncodingMacRoman);

        [[NSColor whiteColor] set];

        [ gBOINC_Logo compositeToPoint:imagePosition operation:NSCompositeCopy ];

        err = TXNDrawCFStringTextBox ( cf_msg, &r, theStyle, &theOptions);
        gTextBoxHeight = r.bottom - r.top + TEXTBOXTOPBORDER;
        gMovingRect.size.height = [gBOINC_Logo size].height + gTextBoxHeight;
        
        CFRelease(cf_msg);
        isErased  = false;
        
    } else {        // Empty or NULL message
        if (!isErased) {
            eraseRect = NSInsetRect(currentDrawingRect, -1, -1);
            [[NSColor blackColor] set];
            isErased  = true;
            NSRectFill(eraseRect);
            gTextBoxHeight = MAXTEXTBOXHEIGHT;
            gMovingRect.size.height = [gBOINC_Logo size].height + gTextBoxHeight;
        }
    }
    
    if (newFrequency)
        [ self setAnimationTimeInterval:(1.0/newFrequency) ];
    // setAnimationTimeInterval does not seem to be working, so we 
    // throttle the screensaver directly here.
    timeToUnblock = AddDurationToAbsolute(durationSecond/newFrequency, frameStartTime);
    MPDelayUntil(&timeToUnblock);
}

- (BOOL)hasConfigureSheet {
    return YES;
}

// Display the configuration sheet for the user to choose their settings
- (NSWindow*)configureSheet
{
	// if we haven't loaded our configure sheet, load the nib named MyScreenSaver.nib
	if (!mConfigureSheet)
            [ NSBundle loadNibNamed:@"BOINCSaver" owner:self ];
	// set the UI state
	[ mGoToBlankCheckbox setState:gGoToBlank ];
        mBlankingTimeString = [[ NSString alloc ] initWithFormat:@"%d", gBlankingTime ];
	[ mBlankingTimeTextField setStringValue:mBlankingTimeString ];
    
	return mConfigureSheet;
}

// Called when the user clicked the SAVE button
- (IBAction) closeSheetSave:(id) sender
{
    // get the defaults
	ScreenSaverDefaults *defaults = [ ScreenSaverDefaults defaultsForModuleWithName:mBundleID ];

	// save the UI state
	gGoToBlank = [ mGoToBlankCheckbox state ];
	mBlankingTimeString = [ mBlankingTimeTextField stringValue ];
        gBlankingTime = [ mBlankingTimeString intValue ];
	
	// write the defaults
	[ defaults setInteger:gGoToBlank forKey:@"GoToBlank" ];
	[ defaults setInteger:gBlankingTime forKey:@"BlankingTime" ];
	
	// synchronize
    [ defaults synchronize ];

	// end the sheet
    [ NSApp endSheet:mConfigureSheet ];
}

// Called when the user clicked the CANCEL button
- (IBAction) closeSheetCancel:(id) sender
{
	// nothing to configure
    [ NSApp endSheet:mConfigureSheet ];
}

@end
