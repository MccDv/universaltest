#include "mainwindow.h"
#include <QApplication>
#include <QSplashScreen>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QPixmap pixmap(":/images/UtLnxSplash.png");
    QSplashScreen *splash = new QSplashScreen;
    splash->setPixmap(pixmap);
    splash->show();

    Qt::Alignment topRight = Qt::AlignRight | Qt::AlignTop;
    splash->showMessage(QObject::tr("Setting up the main window..."),
                           topRight, Qt::white);
    qApp->processEvents();
    MainWindow w;
    w.show();
    splash->finish(&w);

    return a.exec();
}
