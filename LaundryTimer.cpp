#include "LaundryTimer.h"
#include <QApplication>
#include <QProcess>

LaundryTimer::LaundryTimer(QObject *parent)
    : QObject(parent)
    , statusItem(new StatusItemBridge())
    , contextMenu(new QMenu())
    , updateTimer(new QTimer(this))
    , washerSecondsRemaining(0)
    , dryerSecondsRemaining(0)
{
    createMenu();
    statusItem->setMenu(contextMenu);

    connect(updateTimer, &QTimer::timeout, this, &LaundryTimer::updateTimers);
    updateTimer->start(1000);

    updateDisplay();
}

LaundryTimer::~LaundryTimer() {
    delete statusItem;
    delete contextMenu;
}

void LaundryTimer::show() {
    statusItem->show();
}

void LaundryTimer::createMenu() {
    QMenu *washerMenu = createTimeSubmenu("Set Washer", &LaundryTimer::setWasherTime);
    QMenu *dryerMenu = createTimeSubmenu("Set Dryer", &LaundryTimer::setDryerTime);

    contextMenu->addMenu(washerMenu);
    contextMenu->addMenu(dryerMenu);
    contextMenu->addSeparator();

    QAction *stopWasherAction = contextMenu->addAction("Stop Washer");
    connect(stopWasherAction, &QAction::triggered, this, &LaundryTimer::stopWasher);

    QAction *stopDryerAction = contextMenu->addAction("Stop Dryer");
    connect(stopDryerAction, &QAction::triggered, this, &LaundryTimer::stopDryer);

    contextMenu->addSeparator();

    QAction *quitAction = contextMenu->addAction("Quit");
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
}

QMenu* LaundryTimer::createTimeSubmenu(const QString &title, void (LaundryTimer::*slot)(int)) {
    QMenu *menu = new QMenu(title, contextMenu);

    QList<int> presetTimes = {15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 75, 90};

    for (int minutes : presetTimes) {
        QAction *action = menu->addAction(QString("%1 min").arg(minutes));
        connect(action, &QAction::triggered, this, [this, slot, minutes]() {
            (this->*slot)(minutes);
        });
    }

    return menu;
}

void LaundryTimer::setWasherTime(int minutes) {
    washerSecondsRemaining = minutes * 60;
    updateDisplay();
}

void LaundryTimer::setDryerTime(int minutes) {
    dryerSecondsRemaining = minutes * 60;
    updateDisplay();
}

void LaundryTimer::stopWasher() {
    washerSecondsRemaining = 0;
    updateDisplay();
}

void LaundryTimer::stopDryer() {
    dryerSecondsRemaining = 0;
    updateDisplay();
}

void LaundryTimer::updateTimers() {
    bool updated = false;

    if (washerSecondsRemaining > 0) {
        washerSecondsRemaining--;
        updated = true;
        if (washerSecondsRemaining == 0) {
            showNotification("Laundry Timer", "Washer is done!");
        }
    }

    if (dryerSecondsRemaining > 0) {
        dryerSecondsRemaining--;
        updated = true;
        if (dryerSecondsRemaining == 0) {
            showNotification("Laundry Timer", "Dryer is done!");
        }
    }

    if (updated) {
        updateDisplay();
    }
}

void LaundryTimer::showNotification(const QString &title, const QString &message) {
    // Use macOS native notifications via NSUserNotification or system call
    QString script = QString("display notification \"%1\" with title \"%2\" sound name \"Glass\"")
                         .arg(message, title);
    QProcess::startDetached("osascript", {"-e", script});
}

QString LaundryTimer::formatTime(int seconds) const {
    if (seconds <= 0) {
        return "--";
    }

    int minutes = seconds / 60;
    int secs = seconds % 60;

    if (minutes >= 60) {
        int hours = minutes / 60;
        minutes = minutes % 60;
        return QString("%1:%2:%3")
            .arg(hours)
            .arg(minutes, 2, 10, QChar('0'))
            .arg(secs, 2, 10, QChar('0'));
    }

    return QString("%1:%2").arg(minutes).arg(secs, 2, 10, QChar('0'));
}

void LaundryTimer::updateDisplay() {
    QString washerStr = formatTime(washerSecondsRemaining);
    QString dryerStr = formatTime(dryerSecondsRemaining);
    QString displayText = QString("W:%1 D:%2").arg(washerStr, dryerStr);

    statusItem->setText(displayText);
}
