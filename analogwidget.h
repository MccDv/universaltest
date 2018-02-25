#ifndef ANALOGWIDGET_H
#define ANALOGWIDGET_H

#include <QWidget>

namespace Ui {
class analogWidget;
}

class analogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit analogWidget(QWidget *parent = 0);
    ~analogWidget();

private:
    Ui::analogWidget *ui;
};

#endif // ANALOGWIDGET_H
