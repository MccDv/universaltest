#ifndef ANALOGWINDOW_H
#define ANALOGWINDOW_H

#include <QWidget>
#include <QtGui>
#include <QMdiSubWindow>
#include <analogwidget.h>

namespace Ui {
    class AnalogWindow;
}

class AnalogWindow : public QMdiSubWindow
{
    Q_OBJECT

public:
    explicit AnalogWindow(QWidget *parent = 0);
    ~AnalogWindow();

private:
    analogWidget *analogwidget;

};

#endif // ANALOGWINDOW_H
