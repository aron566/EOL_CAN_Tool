#include "mainwindow.h"

#include <QApplication>
#include <QTextCodec>

#define PC_SOFTWARE_VERSION       "1.0.3"

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QApplication a(argc, argv);
    a.setApplicationName("EOL CAN Tool");
    a.setApplicationVersion(PC_SOFTWARE_VERSION);
    MainWindow w;
    w.show();
    return a.exec();
}
