#include "LaundryTimer.h"
#include <QApplication>
#include <QInputDialog>
#include <QDateTime>
#include <QDebug>

// Settings keys
const QString LaundryTimer::WASHER_TIME_KEY = "washer/timeRemaining";
const QString LaundryTimer::DRYER_TIME_KEY = "dryer/timeRemaining";
const QString LaundryTimer::WASHER_PAUSED_KEY = "washer/paused";
const QString LaundryTimer::DRYER_PAUSED_KEY = "dryer/paused";

static const int UPDATE_INTERVAL_MS = 1000;
static const int SNOOZE_MINUTES = 5;
static const int DEFAULT_WASHER_MINUTES = 30;
static const int DEFAULT_DRYER_MINUTES = 45;

LaundryTimer::LaundryTimer(QObject *parent)
    : QObject(parent)
    , statusItem(new StatusItemBridge())
    , notificationManager(new NotificationManager())
    , contextMenu(new QMenu())
    , updateTimer(new QTimer(this))
    , washerSecondsRemaining(0)
    , dryerSecondsRemaining(0)
    , washerPaused(false)
    , dryerPaused(false)
    , pauseWasherAction(nullptr)
    , pauseDryerAction(nullptr)
    , launchAtLoginAction(nullptr)
{
    // Request notification permissions
    notificationManager->requestAuthorization([](bool granted) {
        if (!granted) {
            qWarning() << "Notification authorization denied";
        }
    });

    // Set up notification action categories
    notificationManager->setupActionCategories();

    // Set up action handler for notification buttons
    notificationManager->setActionHandler([this](QString actionId, QString notificationId) {
        handleNotificationAction(actionId, notificationId);
    });

    createMenu();
    statusItem->setMenu(contextMenu);

    connect(updateTimer, &QTimer::timeout, this, &LaundryTimer::updateTimers);
    updateTimer->start(UPDATE_INTERVAL_MS);

    // Restore previous state
    restoreState();
    updateDisplay();
}

LaundryTimer::~LaundryTimer() {
    saveState();
    delete statusItem;
    delete notificationManager;
    delete contextMenu;
}

void LaundryTimer::show() {
    statusItem->show();
}

void LaundryTimer::createMenu() {
    QMenu *washerMenu = createTimeSubmenu("Set Washer", &LaundryTimer::setWasherTime, true);
    QMenu *dryerMenu = createTimeSubmenu("Set Dryer", &LaundryTimer::setDryerTime, false);

    contextMenu->addMenu(washerMenu);
    contextMenu->addMenu(dryerMenu);
    contextMenu->addSeparator();

    pauseWasherAction = contextMenu->addAction("Pause Washer");
    pauseWasherAction->setEnabled(false);
    connect(pauseWasherAction, &QAction::triggered, this, &LaundryTimer::pauseWasher);

    pauseDryerAction = contextMenu->addAction("Pause Dryer");
    pauseDryerAction->setEnabled(false);
    connect(pauseDryerAction, &QAction::triggered, this, &LaundryTimer::pauseDryer);

    contextMenu->addSeparator();

    QAction *stopWasherAction = contextMenu->addAction("Stop Washer");
    connect(stopWasherAction, &QAction::triggered, this, &LaundryTimer::stopWasher);

    QAction *stopDryerAction = contextMenu->addAction("Stop Dryer");
    connect(stopDryerAction, &QAction::triggered, this, &LaundryTimer::stopDryer);

    contextMenu->addSeparator();

    launchAtLoginAction = contextMenu->addAction("Launch at Login");
    launchAtLoginAction->setCheckable(true);
    launchAtLoginAction->setChecked(StatusItemBridge::isLoginItemEnabled());
    connect(launchAtLoginAction, &QAction::triggered, this, &LaundryTimer::toggleLaunchAtLogin);

    contextMenu->addSeparator();

    QAction *quitAction = contextMenu->addAction("Quit");
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
}

QMenu* LaundryTimer::createTimeSubmenu(const QString &title, void (LaundryTimer::*slot)(int), bool isWasher) {
    QMenu *menu = new QMenu(title, contextMenu);

    // Test option: 3 seconds
    QAction *testAction = menu->addAction("3 sec (test)");
    connect(testAction, &QAction::triggered, this, [this, isWasher]() {
        if (isWasher) {
            washerSecondsRemaining = 3;
            washerPaused = false;
        } else {
            dryerSecondsRemaining = 3;
            dryerPaused = false;
        }
        updateDisplay();
    });
    menu->addSeparator();

    // Custom time option
    QAction *customAction = menu->addAction("Custom...");
    connect(customAction, &QAction::triggered, this, [this, isWasher]() {
        setCustomTime(isWasher);
    });
    menu->addSeparator();

    // Preset times
    QList<int> presetTimes = {15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 75, 90};

    for (int minutes : presetTimes) {
        QAction *action = menu->addAction(QString("%1 min").arg(minutes));
        connect(action, &QAction::triggered, this, [this, slot, minutes]() {
            (this->*slot)(minutes);
        });
    }

    return menu;
}

void LaundryTimer::setCustomTime(bool isWasher) {
    bool ok;
    int minutes = QInputDialog::getInt(
        nullptr,
        isWasher ? "Set Washer Timer" : "Set Dryer Timer",
        "Enter time in minutes:",
        30, 1, 300, 1, &ok
    );

    if (ok) {
        if (isWasher) {
            setWasherTime(minutes);
        } else {
            setDryerTime(minutes);
        }
    }
}

void LaundryTimer::setWasherTime(int minutes) {
    washerSecondsRemaining = minutes * 60;
    washerPaused = false;
    updateDisplay();
}

void LaundryTimer::setDryerTime(int minutes) {
    dryerSecondsRemaining = minutes * 60;
    dryerPaused = false;
    updateDisplay();
}

void LaundryTimer::stopWasher() {
    washerSecondsRemaining = 0;
    washerPaused = false;
    updateDisplay();
}

void LaundryTimer::stopDryer() {
    dryerSecondsRemaining = 0;
    dryerPaused = false;
    updateDisplay();
}

void LaundryTimer::pauseWasher() {
    if (washerSecondsRemaining > 0) {
        washerPaused = !washerPaused;
        updateDisplay();
    }
}

void LaundryTimer::pauseDryer() {
    if (dryerSecondsRemaining > 0) {
        dryerPaused = !dryerPaused;
        updateDisplay();
    }
}

void LaundryTimer::toggleLaunchAtLogin() {
    bool currentState = StatusItemBridge::isLoginItemEnabled();
    bool success = StatusItemBridge::setLoginItemEnabled(!currentState);

    if (success) {
        launchAtLoginAction->setChecked(!currentState);
    } else {
        // Revert checkbox if operation failed
        launchAtLoginAction->setChecked(currentState);
    }
}

void LaundryTimer::updateTimers() {
    bool updated = false;

    if (washerSecondsRemaining > 0 && !washerPaused) {
        washerSecondsRemaining--;
        updated = true;
        if (washerSecondsRemaining == 0) {
            showNotification("Laundry Timer", "Washer is done!", true);
        }
    }

    if (dryerSecondsRemaining > 0 && !dryerPaused) {
        dryerSecondsRemaining--;
        updated = true;
        if (dryerSecondsRemaining == 0) {
            showNotification("Laundry Timer", "Dryer is done!", false);
        }
    }

    if (updated) {
        updateDisplay();
    }

    // Save state periodically (every 30 seconds when timers are active)
    static int saveCounter = 0;
    if ((washerSecondsRemaining > 0 || dryerSecondsRemaining > 0) && ++saveCounter >= 30) {
        saveState();
        saveCounter = 0;
    }
}

void LaundryTimer::showNotification(const QString &title, const QString &message, bool isWasher) {
    QString identifier = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString categoryId = isWasher ? "WASHER_DONE" : "DRYER_DONE";
    notificationManager->sendNotification(title, message, identifier, categoryId);
}

void LaundryTimer::handleNotificationAction(const QString &actionId, const QString &notificationId) {
    Q_UNUSED(notificationId);

    if (actionId == "RESTART_WASHER") {
        setWasherTime(DEFAULT_WASHER_MINUTES);
    } else if (actionId == "SNOOZE_WASHER") {
        washerSecondsRemaining = SNOOZE_MINUTES * 60;
        washerPaused = false;
        updateDisplay();
    } else if (actionId == "RESTART_DRYER") {
        setDryerTime(DEFAULT_DRYER_MINUTES);
    } else if (actionId == "SNOOZE_DRYER") {
        dryerSecondsRemaining = SNOOZE_MINUTES * 60;
        dryerPaused = false;
        updateDisplay();
    }
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

    // Add pause indicator
    if (washerPaused && washerSecondsRemaining > 0) {
        washerStr = "[" + washerStr + "]";
    }
    if (dryerPaused && dryerSecondsRemaining > 0) {
        dryerStr = "[" + dryerStr + "]";
    }

    QString displayText;

    // Show compact display when idle, full display when active
    if (washerSecondsRemaining == 0 && dryerSecondsRemaining == 0) {
        displayText = "W:-- D:--";
    } else {
        displayText = QString("W:%1 D:%2").arg(washerStr, dryerStr);
    }

    statusItem->setText(displayText);

    // Update pause action states and text
    pauseWasherAction->setEnabled(washerSecondsRemaining > 0);
    pauseWasherAction->setText(washerPaused ? "Resume Washer" : "Pause Washer");

    pauseDryerAction->setEnabled(dryerSecondsRemaining > 0);
    pauseDryerAction->setText(dryerPaused ? "Resume Dryer" : "Pause Dryer");
}

void LaundryTimer::saveState() {
    QSettings settings;
    settings.setValue(WASHER_TIME_KEY, washerSecondsRemaining);
    settings.setValue(DRYER_TIME_KEY, dryerSecondsRemaining);
    settings.setValue(WASHER_PAUSED_KEY, washerPaused);
    settings.setValue(DRYER_PAUSED_KEY, dryerPaused);
}

void LaundryTimer::restoreState() {
    QSettings settings;
    washerSecondsRemaining = settings.value(WASHER_TIME_KEY, 0).toInt();
    dryerSecondsRemaining = settings.value(DRYER_TIME_KEY, 0).toInt();
    washerPaused = settings.value(WASHER_PAUSED_KEY, false).toBool();
    dryerPaused = settings.value(DRYER_PAUSED_KEY, false).toBool();
}
