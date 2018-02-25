#include <QtGui>
#include <QWidget>
#include <QMdiSubWindow>
#include "analogwindow.h"
#include "analogwidget.h"

AnalogWindow::AnalogWindow(QWidget *parent) : QMdiSubWindow(parent)
{
    analogwidget = new analogWidget(this);
    this->setWidget(analogwidget);
}

AnalogWindow::~AnalogWindow()
{
    analogwidget->~analogWidget();
}
