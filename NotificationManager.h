#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QString>
#include <functional>

class NotificationManager {
public:
    NotificationManager();
    ~NotificationManager();

    void requestAuthorization(std::function<void(bool granted)> callback);
    void sendNotification(const QString &title,
                         const QString &body,
                         const QString &identifier,
                         const QString &categoryId = QString());
    void setupActionCategories();
    void setActionHandler(std::function<void(QString actionId, QString notificationId)> handler);

private:
    void *delegate;
    std::function<void(QString, QString)> actionHandler;
};

#endif // NOTIFICATIONMANAGER_H
