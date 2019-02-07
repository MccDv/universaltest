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
    QCommandLineParser parser;
    QColor textColor;
    QString appVer;

    textColor = "royalblue";
    a.setApplicationName("version");
    a.setApplicationVersion(VERSION_STRING);
    parser.addVersionOption();
    parser.process(a);
    QPixmap pixmap(":/images/UtLnxSplash.png");
    QSplashScreen *splash = new QSplashScreen;
    splash->setPixmap(pixmap);
    splash->showMessage("Initializing...            ", Qt::AlignVCenter | Qt::AlignRight, textColor);
    splash->show();
    MainWindow w;
    qApp->processEvents();

    splash->activateWindow();
    qApp->processEvents();
    appVer = "Version " + QApplication::applicationVersion() + "            ";
    splash->showMessage(appVer, Qt::AlignVCenter | Qt::AlignRight, textColor);
    I::sleep(3); // show splash for 3 seconds

    w.show();
    splash->finish(&w);

    return a.exec();
}
