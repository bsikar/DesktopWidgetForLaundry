#include "NotificationManager.h"
#import <UserNotifications/UserNotifications.h>
#import <Foundation/Foundation.h>

@interface NotificationDelegate : NSObject <UNUserNotificationCenterDelegate>
@property (nonatomic, copy) void (^actionCallback)(NSString*, NSString*);
@end

@implementation NotificationDelegate

- (void)userNotificationCenter:(UNUserNotificationCenter *)center
       willPresentNotification:(UNNotification *)notification
         withCompletionHandler:(void (^)(UNNotificationPresentationOptions))completionHandler {
    (void)center;
    (void)notification;

    UNNotificationPresentationOptions options = UNNotificationPresentationOptionSound;
    if (@available(macOS 11.0, *)) {
        options |= UNNotificationPresentationOptionBanner | UNNotificationPresentationOptionList;
    }
    completionHandler(options);
}

- (void)userNotificationCenter:(UNUserNotificationCenter *)center
didReceiveNotificationResponse:(UNNotificationResponse *)response
         withCompletionHandler:(void (^)(void))completionHandler {
    (void)center;

    NSString *actionIdentifier = response.actionIdentifier;
    NSString *notificationIdentifier = response.notification.request.identifier;

    if (self.actionCallback) {
        self.actionCallback(actionIdentifier, notificationIdentifier);
    }

    completionHandler();
}

@end

NotificationManager::NotificationManager() : delegate(nullptr) {
    @autoreleasepool {
        NotificationDelegate *del = [[NotificationDelegate alloc] init];
        delegate = del;

        UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
        center.delegate = del;
    }
}

NotificationManager::~NotificationManager() {
    if (delegate) {
        NotificationDelegate *del = (NotificationDelegate *)delegate;
        [del release];
        delegate = nullptr;
    }
}

void NotificationManager::requestAuthorization(std::function<void(bool)> callback) {
    @autoreleasepool {
        UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
        UNAuthorizationOptions options = UNAuthorizationOptionAlert |
                                        UNAuthorizationOptionSound |
                                        UNAuthorizationOptionBadge;

        [center requestAuthorizationWithOptions:options
                              completionHandler:^(BOOL granted, NSError * _Nullable error) {
            (void)error;
            if (callback) {
                dispatch_async(dispatch_get_main_queue(), ^{
                    callback(granted);
                });
            }
        }];
    }
}

void NotificationManager::sendNotification(const QString &title,
                                          const QString &body,
                                          const QString &identifier,
                                          const QString &categoryId) {
    @autoreleasepool {
        // Convert QString to NSString immediately and retain them for the block
        NSString *nsTitle = [title.toNSString() copy];
        NSString *nsBody = [body.toNSString() copy];
        NSString *nsIdentifier = [identifier.toNSString() copy];
        NSString *nsCategoryId = categoryId.isEmpty() ? nil : [categoryId.toNSString() copy];

        UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];

        [center getNotificationSettingsWithCompletionHandler:^(UNNotificationSettings *settings) {
            if (settings.authorizationStatus != UNAuthorizationStatusAuthorized) {
                NSLog(@"Notifications not authorized");
                dispatch_async(dispatch_get_main_queue(), ^{
                    [nsTitle release];
                    [nsBody release];
                    [nsIdentifier release];
                    if (nsCategoryId) [nsCategoryId release];
                });
                return;
            }

            dispatch_async(dispatch_get_main_queue(), ^{
                UNMutableNotificationContent *content = [[UNMutableNotificationContent alloc] init];
                content.title = nsTitle;
                content.body = nsBody;
                content.sound = [UNNotificationSound defaultSound];

                if (nsCategoryId) {
                    content.categoryIdentifier = nsCategoryId;
                }

                UNTimeIntervalNotificationTrigger *trigger =
                    [UNTimeIntervalNotificationTrigger triggerWithTimeInterval:0.1 repeats:NO];

                UNNotificationRequest *request =
                    [UNNotificationRequest requestWithIdentifier:nsIdentifier
                                                         content:content
                                                         trigger:trigger];

                [center addNotificationRequest:request withCompletionHandler:^(NSError * _Nullable error) {
                    if (error) {
                        NSLog(@"Notification error: %@", error.localizedDescription);
                    }
                }];

                [nsTitle release];
                [nsBody release];
                [nsIdentifier release];
                if (nsCategoryId) [nsCategoryId release];
            });
        }];
    }
}

void NotificationManager::setupActionCategories() {
    @autoreleasepool {
        UNNotificationAction *restartWasherAction =
            [UNNotificationAction actionWithIdentifier:@"RESTART_WASHER"
                                               title:@"Restart (30 min)"
                                             options:UNNotificationActionOptionNone];

        UNNotificationAction *snoozeWasherAction =
            [UNNotificationAction actionWithIdentifier:@"SNOOZE_WASHER"
                                               title:@"Snooze (5 min)"
                                             options:UNNotificationActionOptionNone];

        UNNotificationCategory *washerCategory =
            [UNNotificationCategory categoryWithIdentifier:@"WASHER_DONE"
                                                   actions:@[restartWasherAction, snoozeWasherAction]
                                         intentIdentifiers:@[]
                                                   options:UNNotificationCategoryOptionNone];

        UNNotificationAction *restartDryerAction =
            [UNNotificationAction actionWithIdentifier:@"RESTART_DRYER"
                                               title:@"Restart (45 min)"
                                             options:UNNotificationActionOptionNone];

        UNNotificationAction *snoozeDryerAction =
            [UNNotificationAction actionWithIdentifier:@"SNOOZE_DRYER"
                                               title:@"Snooze (5 min)"
                                             options:UNNotificationActionOptionNone];

        UNNotificationCategory *dryerCategory =
            [UNNotificationCategory categoryWithIdentifier:@"DRYER_DONE"
                                                   actions:@[restartDryerAction, snoozeDryerAction]
                                         intentIdentifiers:@[]
                                                   options:UNNotificationCategoryOptionNone];

        UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
        [center setNotificationCategories:[NSSet setWithObjects:washerCategory, dryerCategory, nil]];
    }
}

void NotificationManager::setActionHandler(std::function<void(QString, QString)> handler) {
    actionHandler = handler;

    if (delegate) {
        NotificationDelegate *del = (NotificationDelegate *)delegate;
        del.actionCallback = ^(NSString *actionId, NSString *notificationId) {
            if (handler) {
                handler(QString::fromNSString(actionId),
                       QString::fromNSString(notificationId));
            }
        };
    }
}
