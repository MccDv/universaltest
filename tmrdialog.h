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

    bool stopOnStart() { return mStopOnStart; }
    bool enabled() { return mEnabled; }
    int interval() { return mInterval; }

private slots:
    void setParams();
    void updateInterval();
    void updateEnabling();
    void updateStopOnStart();

private:
    Ui::TmrDialog *ui;

    bool mEnabled;
    bool mStopOnStart;
    int mInterval;

signals:
    void intervalChanged(int);
    void enabledChanged(bool);
    void stopOnStartChanged(bool);
};

#endif // TMRDIALOG_H
