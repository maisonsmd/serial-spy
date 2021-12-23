#include <QApplication>

#include "controllers/mainwindow.h"
#include "utils/loghandler.h"

int main(int argc, char *argv[])
{
    initLog();

    QApplication a(argc, argv);
    MainWindow w;

    w.show();
    return a.exec();
}
