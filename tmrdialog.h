#ifndef TMRDIALOG_H
#define TMRDIALOG_H

#include <QDialog>

namespace Ui {
class TmrDialog;
}

class TmrDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(int interval READ interval WRITE setInterval NOTIFY intervalChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool stopOnStart READ stopOnStart WRITE setStopOnStart NOTIFY stopOnStartChanged)
    Q_PROPERTY(bool onePerInterval READ onePerInterval WRITE setOnePerInterval NOTIFY onePerIntervalChanged)

public:
    explicit TmrDialog(QWidget *parent = 0);
    ~TmrDialog();

    void setEnabled(bool enabled)
    {
        mEnabled = enabled;
        emit enabledChanged(enabled);
    }

    void setInterval(int interval)
    {
        mInterval = interval;
        emit intervalChanged(interval);
    }

    void setStopOnStart(bool stopOnStart)
    {
        mStopOnStart = stopOnStart;
        emit stopOnStartChanged(stopOnStart);
    }

    void setOnePerInterval(bool oneSamplePer)
    {
        mOneSamplePer = oneSamplePer;
        emit onePerIntervalChanged(oneSamplePer);
    }

    bool stopOnStart() { return mStopOnStart; }
    bool enabled() { return mEnabled; }
    int interval() { return mInterval; }
    bool onePerInterval() { return mOneSamplePer; }

private slots:
    void setParams();
    void updateInterval();
    void updateEnabling();
    void updateOnePer();
    void updateStopOnStart();

private:
    Ui::TmrDialog *ui;

    bool mEnabled = false;
    bool mStopOnStart = false;
    bool mOneSamplePer = false;
    int mInterval = 1000;

signals:
    void intervalChanged(int);
    void enabledChanged(bool);
    void stopOnStartChanged(bool);
    void onePerIntervalChanged(bool);
};

#endif // TMRDIALOG_H
