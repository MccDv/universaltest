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
    //QCoreApplication a(argc, argv);
    a.setApplicationName("version");
    a.setApplicationVersion(VERSION_STRING);
    QCommandLineParser parser;
    parser.addVersionOption();
    parser.process(a);
    QPixmap pixmap(":/images/UtLnxSplash.png");
    QSplashScreen *splash = new QSplashScreen;
    splash->setPixmap(pixmap);
    splash->show();
    MainWindow w;
    //w.setWindowTitle("Universal Test for Linux");
    qApp->processEvents();
    splash->activateWindow();
    QColor textColor;
    QString appVer;
    appVer = "Version " + QApplication::applicationVersion() + "            ";
    textColor = "royalblue";
    splash->showMessage(appVer, Qt::AlignVCenter | Qt::AlignRight, textColor);
    I::sleep(3); // show splash for 3 seconds

    w.show();
    splash->finish(&w);

    return a.exec();
}
