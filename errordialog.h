#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QDialog>
#include "uldaq.h"

namespace Ui {
class ErrorDialog;
}

class ErrorDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(bool historyCleared READ historyCleared WRITE setHistoryCleared NOTIFY historyClearedChanged)

public:
    explicit ErrorDialog(QWidget *parent = 0);
    ~ErrorDialog();

    void setHistoryCleared(bool historyCleared)
    {
        mHistoryCleared = historyCleared;
        emit historyClearedChanged(historyCleared);
    }

    bool historyCleared() { return mHistoryCleared; }

    void setError(UlError curError, QString funcText);
    void addFunction(QString funcString);
    void showHistory(QStringList historyList);
    void setHistSize(int histSize);
    int getHistSize();

private slots:
    void on_cmdOK_clicked();
    void clearHistory();

private:
    Ui::ErrorDialog *ui;
    QStringList mFuncHistoryList;
    QString getErrorText(int errorNumber);
    int mHistorySize;
    bool mHistoryCleared;

signals:
    void historyClearedChanged(bool);
};

#endif // ERRORDIALOG_H
