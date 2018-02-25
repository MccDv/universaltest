#ifndef TRIGDIALOG_H
#define TRIGDIALOG_H

#include <QDialog>

namespace Ui {
class TrigDialog;
}

class TrigDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(int trigChannel READ trigChannel WRITE setTrigChannel NOTIFY trigChannelChanged)
    Q_PROPERTY(double trigLevel READ trigLevel WRITE setTrigLevel NOTIFY trigLevelChanged)
    Q_PROPERTY(double trigVariance READ trigVariance WRITE setTrigVariance NOTIFY trigVarianceChanged)
    Q_PROPERTY(unsigned int retrigCount READ retrigCount WRITE setRetrigCount NOTIFY retrigCountChanged)

public:
    explicit TrigDialog(QWidget *parent = 0);
    ~TrigDialog();

    void setTrigChannel(int trigChannel)
    {
        mTrigChannel = trigChannel;
        emit trigChannelChanged(trigChannel);
    }

    void setTrigLevel(double trigLevel)
    {
        mTrigLevel = trigLevel;
        emit trigLevelChanged(trigLevel);
    }

    void setTrigVariance(double trigVariance)
    {
        mTrigVariance = trigVariance;
        emit trigVarianceChanged(trigVariance);
    }

    void setRetrigCount(unsigned int retrigCount)
    {
        mRetrigCount = retrigCount;
        emit retrigCountChanged(retrigCount);
    }

    int trigChannel() { return mTrigChannel; }
    double trigLevel() { return mTrigLevel; }
    double trigVariance() { return mTrigVariance; }
    unsigned int retrigCount() { return mRetrigCount; }

private slots:
    void setParams();
    void updateTrigChan();
    void updateTrigLevel();
    void updateTrigVariance();
    void updateRetrigCount();

private:
    Ui::TrigDialog *ui;

    int mTrigChannel;
    double mTrigVariance;
    double mTrigLevel;
    unsigned int mRetrigCount;

signals:
    void trigChannelChanged(int);
    void trigLevelChanged(double);
    void trigVarianceChanged(double);
    void retrigCountChanged(unsigned int);
};

#endif // TRIGDIALOG_H
