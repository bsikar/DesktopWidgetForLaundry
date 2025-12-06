#ifndef LAUNDRYTIMER_H
#define LAUNDRYTIMER_H

#include <QObject>
#include <QMenu>
#include <QTimer>
#include "StatusItemBridge.h"

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

private:
    void createMenu();
    void updateDisplay();
    void showNotification(const QString &title, const QString &message);
    QMenu* createTimeSubmenu(const QString &title, void (LaundryTimer::*slot)(int));
    QString formatTime(int seconds) const;

    StatusItemBridge *statusItem;
    QMenu *contextMenu;

    QTimer *updateTimer;
    int washerSecondsRemaining;
    int dryerSecondsRemaining;
};

#endif // LAUNDRYTIMER_H
