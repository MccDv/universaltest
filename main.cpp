#include "mainwindow.h"
#include "qthread.h"
#include <QApplication>
#include <QSplashScreen>

class I : public QThread
{
public:
    static void sleep(unsigned long secs) {
        QThread::sleep(secs);
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QPixmap pixmap(":/images/UtLnxSplash.png");
    QSplashScreen *splash = new QSplashScreen;
    splash->setPixmap(pixmap);
    splash->show();
    MainWindow w;
    //w.setWindowTitle("Universal Test for Linux");
    qApp->processEvents();
    splash->activateWindow();
    QColor textColor;
    textColor = "royalblue";
    splash->showMessage("       Version 1.2.0", Qt::AlignCenter, textColor);
    I::sleep(3); // show splash for 3 seconds

    w.show();
    splash->finish(&w);

    return a.exec();
}
