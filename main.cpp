#include <QApplication>
#include "LaundryTimer.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    LaundryTimer timer;
    timer.show();

    return app.exec();
}
