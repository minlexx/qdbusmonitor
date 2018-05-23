#include <QCoreApplication>
#include "monitorapp.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    MonitorApp app(argc, argv);
    if (!app.init()) {
        return -1;
    }

    int mainret = app.exec();
    app.stopMonitor(); // should stop BG thread before exiting app

    return mainret;
}
