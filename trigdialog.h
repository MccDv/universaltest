#ifndef TRIGDIALOG_H
#define TRIGDIALOG_H

#include <QDialog>
#include "uldaq.h"

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
    Q_PROPERTY(int trigChanType READ trigChanType WRITE setTrigChanType NOTIFY trigChanTypeChanged)
    Q_PROPERTY(Range trigRange READ trigRange WRITE setTrigRange NOTIFY trigRangeChanged)

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

    void setTrigRange(Range trigRange)
    {
        mTrigRange = trigRange;
        emit trigRangeChanged(trigRange);
    }

    void setTrigChanType(int trigChanType)
    {
        mTrigChannel = trigChanType;
        emit trigChanTypeChanged(trigChanType);
    }

    int trigChannel() { return mTrigChannel; }
    double trigLevel() { return mTrigLevel; }
    double trigVariance() { return mTrigVariance; }
    unsigned int retrigCount() { return mRetrigCount; }
    Range trigRange() { return mTrigRange; }
    int trigChanType() { return mTrigChanType; }

private slots:
    void setParams();
    void updateTrigChan();
    void updateTrigLevel();
    void updateTrigVariance();
    void updateRetrigCount();
    void updateTrigChanType();
    void updateTrigRange();

private:
    Ui::TrigDialog *ui;

    int mTrigChannel;
    int mTrigChanType;
    Range mTrigRange;
    double mTrigVariance;
    double mTrigLevel;
    unsigned int mRetrigCount;

signals:
    void trigChannelChanged(int);
    void trigLevelChanged(double);
    void trigVarianceChanged(double);
    void retrigCountChanged(unsigned int);
    void trigChanTypeChanged(int);
    void trigRangeChanged(Range);
};

#endif // TRIGDIALOG_H
