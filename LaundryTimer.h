#ifndef LAUNDRYTIMER_H
#define LAUNDRYTIMER_H

#include <QObject>
#include <QMenu>
#include <QTimer>
#include <QSettings>
#include "StatusItemBridge.h"
#include "NotificationManager.h"

class LaundryTimer : public QObject {
    Q_OBJECT

public:
    explicit LaundryTimer(QObject *parent = nullptr);
    ~LaundryTimer() override;

    void show();

private slots:
    void updateTimers();
    void setWasherTime(int minutes);
    void setDryerTime(int minutes);
    void stopWasher();
    void stopDryer();
    void pauseWasher();
    void pauseDryer();
    void setCustomTime(bool isWasher);
    void toggleLaunchAtLogin();

private:
    void createMenu();
    void updateDisplay();
    void showNotification(const QString &title, const QString &message, bool isWasher);
    void handleNotificationAction(const QString &actionId, const QString &notificationId);
    QMenu* createTimeSubmenu(const QString &title, void (LaundryTimer::*slot)(int), bool isWasher);
    QString formatTime(int seconds) const;

    void saveState();
    void restoreState();

    StatusItemBridge *statusItem;
    NotificationManager *notificationManager;
    QMenu *contextMenu;

    QTimer *updateTimer;
    int washerSecondsRemaining;
    int dryerSecondsRemaining;
    bool washerPaused;
    bool dryerPaused;

    // Menu actions that need updating
    QAction *pauseWasherAction;
    QAction *pauseDryerAction;
    QAction *launchAtLoginAction;

    // Settings keys
    static const QString WASHER_TIME_KEY;
    static const QString DRYER_TIME_KEY;
    static const QString WASHER_PAUSED_KEY;
    static const QString DRYER_PAUSED_KEY;
};

#endif // LAUNDRYTIMER_H
