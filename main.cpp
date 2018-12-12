#include "mainwindow.h"
#include "qthread.h"
#include <QApplication>
#include <QSplashScreen>

class I : public QThread
{
public:
    static void sleep(unsigned long secs) { QThread::sleep(secs); }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QPixmap pixmap(":/images/UtLnxSplash.png");
    QSplashScreen *splash = new QSplashScreen;
    splash->setPixmap(pixmap);
    splash->show();
    splash->activateWindow();

    qApp->processEvents();
    I::sleep(3); // show splash for 3 seconds
    MainWindow w;
    w.show();
    splash->finish(&w);

    return a.exec();
}
