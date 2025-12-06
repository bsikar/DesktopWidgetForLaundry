#include "StatusItemBridge.h"

#import <Cocoa/Cocoa.h>
#include <QMacNativeWidget>

StatusItemBridge::StatusItemBridge() {
    NSStatusBar *statusBar = [NSStatusBar systemStatusBar];
    NSStatusItem *item = [statusBar statusItemWithLength:NSVariableStatusItemLength];
    [item retain];
    statusItem = item;

    NSStatusBarButton *button = [item button];
    [button setFont:[NSFont monospacedDigitSystemFontOfSize:13.0 weight:NSFontWeightMedium]];
}

StatusItemBridge::~StatusItemBridge() {
    NSStatusItem *item = (NSStatusItem *)statusItem;
    [[NSStatusBar systemStatusBar] removeStatusItem:item];
    [item release];
}

void StatusItemBridge::setText(const QString &text) {
    NSStatusItem *item = (NSStatusItem *)statusItem;
    NSString *nsText = text.toNSString();
    [[item button] setTitle:nsText];
}

void StatusItemBridge::setMenu(QMenu *menu) {
    NSStatusItem *item = (NSStatusItem *)statusItem;
    [item setMenu:menu->toNSMenu()];
}

void StatusItemBridge::show() {
    // Status item is visible once created
}
