#include "StatusItemBridge.h"

#import <Cocoa/Cocoa.h>
#import <ServiceManagement/ServiceManagement.h>
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

bool StatusItemBridge::isLoginItemEnabled() {
    if (@available(macOS 13.0, *)) {
        SMAppService *service = [SMAppService mainAppService];
        return service.status == SMAppServiceStatusEnabled;
    }
    return false;
}

bool StatusItemBridge::setLoginItemEnabled(bool enabled) {
    if (@available(macOS 13.0, *)) {
        SMAppService *service = [SMAppService mainAppService];
        NSError *error = nil;

        if (enabled) {
            BOOL success = [service registerAndReturnError:&error];
            if (!success) {
                NSLog(@"Failed to register login item: %@", error.localizedDescription);
                return false;
            }
        } else {
            BOOL success = [service unregisterAndReturnError:&error];
            if (!success) {
                NSLog(@"Failed to unregister login item: %@", error.localizedDescription);
                return false;
            }
        }
        return true;
    }
    return false;
}
