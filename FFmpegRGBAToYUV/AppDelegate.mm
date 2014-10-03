//
//  AppDelegate.m
//  FFmpegRGBAToYUV
//
//  Created by Philip Schneider on 10/3/14.
//  Copyright (c) 2014 Philip Schneider. All rights reserved.
//

#import "AppDelegate.h"
#include "ConvertRGBA.h"

@interface AppDelegate ()

@property (weak) IBOutlet NSWindow *window;
@property (weak) IBOutlet NSButton *createMovieButton;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication {
    return YES;
}


- (IBAction)createMovie:(id)sender {
    convertRGBA();
}
@end
