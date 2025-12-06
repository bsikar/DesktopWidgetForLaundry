#include <QApplication>
#include "LaundryTimer.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Set up application info for QSettings
    QCoreApplication::setOrganizationDomain("com.laundry.timer");
    QCoreApplication::setOrganizationName("LaundryTimer");
    QCoreApplication::setApplicationName("Laundry Timer");
    QCoreApplication::setApplicationVersion("1.1.0");

    app.setQuitOnLastWindowClosed(false);

    LaundryTimer timer;
    timer.show();

    return app.exec();
}
