#ifndef STATUSITEMBRIDGE_H
#define STATUSITEMBRIDGE_H

#include <QString>
#include <QMenu>
#include <functional>

class StatusItemBridge {
public:
    StatusItemBridge();
    ~StatusItemBridge();

    void setText(const QString &text);
    void setMenu(QMenu *menu);
    void show();

private:
    void *statusItem; // NSStatusItem*
};

#endif // STATUSITEMBRIDGE_H
