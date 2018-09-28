#include <QtGui>
#include <QWidget>
#include <QMdiSubWindow>
#include "childwindow.h"
#include "subwidget.h"
#include "aisubwidget.h"
#include "aosubwidget.h"
#include "diosubwidget.h"
#include "ctrsubwidget.h"
#include "discoversubwidget.h"

ChildWindow::ChildWindow(QWidget *parent, UtFunctionGroup funcGroup) : QMdiSubWindow(parent)
{
    switch (funcGroup) {
    case FUNC_GROUP_AIN:
        subwidget = new AiSubWidget(this);
        windowName = "ainwindow";
        break;
    case FUNC_GROUP_AOUT:
        subwidget = new AoSubWidget(this);
        windowName = "aoutwindow";
        break;
    case FUNC_GROUP_DIN:
        subwidget = new DioSubWidget(this);
        windowName = "dinwindow";
        break;
    case FUNC_GROUP_DOUT:
        subwidget = new DioSubWidget(this);
        windowName = "doutwindow";
        break;
    case FUNC_GROUP_CTR:
        subwidget = new CtrSubWidget(this);
        windowName = "ctrwindow";
        break;
    case FUNC_GROUP_MISC:
        subwidget = new subWidget(this);
        windowName = "miscwindow";
        break;
    case FUNC_GROUP_CONFIG:
        subwidget = new subWidget(this);
        windowName = "cfgwindow";
        break;
    default:
        subwidget = new DiscoverSubWidget(this);
        windowName = "dscvrwindow";
        break;
    }

    MainWindow *mainParent = qobject_cast<MainWindow *>(parent);
    this->setWidget(subwidget);

    mStatusEnabled = true;
    mWaitEnabled = false;
    mWaitTime = 5.5;
    tmrRunFunc = new QTimer(this);
    mTmrEnabled = false;
    mTmrInterval = 0;
    mOneSamplePer = false;
    mRange = BIP10VOLTS;
    mInputMode = AI_DIFFERENTIAL;
    mAiFlags = (AInFlag)0;
    mAoFlags = (AOutFlag)0;
    mDaqiFlags = (DaqInScanFlag)0;
    mCiFlags = CINSCAN_FF_DEFAULT;
    mTriggerType = TRIG_NONE;
    mRetrigCount = 0;
    mTrigChannel = 0;
    mTrigChanType = 0;
    mTrigRange = BIP10VOLTS;
    mTrigLevel = 1.00;
    mTrigVariance = 0.002;
    mScanOptions = SO_DEFAULTIO;

    connect(this, SIGNAL(refreshData()), subwidget, SLOT(runSelectedFunc()));
    connect(this, SIGNAL(refreshBoads()), subwidget, SLOT(initDeviceParams()));

    //connect(this, SIGNAL(eventParamsChanged(unsigned long long)), subwidget, SLOT(updateParamters()));
    //connect(this, SIGNAL(eventTypesChanged(DaqEventType)), subwidget, SLOT(updateParamters()));
    connect(this, SIGNAL(statusEnabledChanged(bool)), subwidget, SLOT(updateParameters()));
    connect(this, SIGNAL(waitEnabledChanged(bool)), subwidget, SLOT(updateParameters()));
    connect(this, SIGNAL(waitTimeChanged(double)), subwidget, SLOT(updateParameters()));
    connect(this, SIGNAL(eventsReadyForUpdate()), subwidget, SLOT(runEventSetup()));
    //connect(this, SIGNAL(eventsReadyForDisable()), subwidget, SLOT(runEventDisable()));

    connect(tmrRunFunc, SIGNAL(timeout()), subwidget, SLOT(runSelectedFunc()));
    connect(this, SIGNAL(tmrRunningChanged(bool)), this, SLOT(goTimerRun(bool)));
    connect(this, SIGNAL(tmrEnabledChanged(bool)), subwidget, SLOT(updateParameters()));
    connect(this, SIGNAL(tmrSampPerIntervalChanged(bool)), subwidget, SLOT(updateParameters()));
    connect(this, SIGNAL(tmrIntervalChanged(int)), subwidget, SLOT(updateParameters()));
    connect(this, SIGNAL(devHandleChanged(DaqDeviceHandle)), subwidget, SLOT(updateParameters()));
    connect(this, SIGNAL(devNameChanged(QString)), subwidget, SLOT(updateParameters()));
    connect(this, SIGNAL(devUIDChanged(QString)), subwidget, SLOT(updateParameters()));
    connect(this, SIGNAL(showPlotChanged(bool)), subwidget, SLOT(showPlotWindow(bool)));
    //connect(this, SIGNAL(plotWindowChanged(PlotWindow*)), subwidget, SLOT(updatePlotWindow(PlotWindow*)));

    connect(this, SIGNAL(inputModeChanged(AiInputMode)), subwidget, SLOT(updateParameters()));
    connect(this, SIGNAL(aiFlagsChanged(long long)), subwidget, SLOT(updateParameters()));
    connect(this, SIGNAL(aoFlagsChanged(long long)), subwidget, SLOT(updateParameters()));
    connect(this, SIGNAL(daqiFlagsChanged(DaqInScanFlag)), subwidget, SLOT(updateParameters()));
    connect(this, SIGNAL(daqOutFlagChanged(DaqOutScanFlag)), subwidget, SLOT(updateParameters()));
    connect(this, SIGNAL(ciFlagsChanged(CInScanFlag)), subwidget, SLOT(updateParameters()));
    connect(this, SIGNAL(scanOptionsChanged(ScanOption)), subwidget, SLOT(updateParameters()));
    connect(this, SIGNAL(triggerTypeChanged(QString)), subwidget, SLOT(updateText(QString)));
    /*connect(this, SIGNAL(trigChannelChanged(int)), subwidget, SLOT(updateParameters()));
    connect(this, SIGNAL(trigLevelChanged(double)), subwidget, SLOT(updateParameters()));
    connect(this, SIGNAL(trigVarianceChanged(double)), subwidget, SLOT(updateParameters()));
    connect(this, SIGNAL(retrigCountChanged(uint)), subwidget, SLOT(updateParameters()));*/
    connect(this, SIGNAL(configData()), subwidget, SLOT(showDataGen()));
    connect(this, SIGNAL(configQueue()), subwidget, SLOT(showQueueConfig()));

    connect(this, SIGNAL(groupChanged(int)), subwidget, SLOT(groupChanged(int)));
    connect(this, SIGNAL(funcChanged(int)), subwidget, SLOT(functionChanged(int)));
    connect(this, SIGNAL(stopScanRequest()), subwidget, SLOT(swStopScan()));
    connect(this, SIGNAL(rangeChanged(Range)), mainParent, SLOT(syncRange(Range)));
    connect(this, SIGNAL(scaleChanged(TempScale)), mainParent, SLOT(syncScale(TempScale)));
    curFunctionGroup = funcGroup;
    emit groupChanged(funcGroup);

    readWindowPosition();
}

ChildWindow::~ChildWindow()
{
    subwidget->~QWidget();
}

void ChildWindow::closeEvent(QCloseEvent *event)
{
    writeWindowPosition();
    event->accept();
    this->deleteLater();
}

void ChildWindow::goTimerRun(bool enable)
{
    if (enable) {
        tmrRunFunc->start(mTmrInterval);
    } else {
        tmrRunFunc->stop();
    }
    emit tmrEnabledChanged(true);
}

void ChildWindow::setCurFunction(int utFunction)
{
    curFunction = utFunction;
    emit funcChanged(utFunction);
}

void ChildWindow::setCurRange(Range curRange)
{
    mRange = curRange;
    emit rangeChanged(curRange);
}

void ChildWindow::setCurScale(TempScale curScale)
{
    mScale = curScale;
    emit scaleChanged(curScale);
}

void ChildWindow::updateEventSetup()
{
    emit eventsReadyForUpdate();
}

void ChildWindow::disableEvents()
{
    emit eventsReadyForDisable();
}

void ChildWindow::setUpTimer()
{
    tmrDialog = new TmrDialog(this);
    connect(tmrDialog, SIGNAL(accepted()), this, SLOT(tmrDialogResponse()));
    if (mTmrInterval == 0)
        mTmrInterval = 1000;
    tmrDialog->setEnabled(mTmrEnabled);
    tmrDialog->setInterval(mTmrInterval);
    tmrDialog->setOnePerInterval(mOneSamplePer);
    tmrDialog->setStopOnStart(mStopOnStart);
    tmrDialog->exec();
}

void ChildWindow::tmrDialogResponse()
{
    mTmrEnabled = tmrDialog->enabled();
    mTmrInterval = tmrDialog->interval();
    mOneSamplePer = tmrDialog->onePerInterval();
    mStopOnStart = tmrDialog->stopOnStart();
    disconnect(tmrDialog);
    delete tmrDialog;
    emit tmrIntervalChanged(mTmrInterval);
}

void ChildWindow::stopScan()
{
    emit stopScanRequest();
}

void ChildWindow::readWindowPosition()
{
    QSettings windowSettings("Measurement Computing", "Universal Test Linux");

    windowSettings.beginGroup(windowName);

    restoreGeometry(windowSettings.value("geometry", saveGeometry()).toByteArray());
    //restoreState(windowSettings.value("savestate", saveState()).toByteArray());
    move(windowSettings.value("pos", pos()).toPoint());
    resize(windowSettings.value("size", size()).toSize());
    if (windowSettings.value("maximized", isMaximized()).toBool())
        showMaximized();

    windowSettings.endGroup();
}

void ChildWindow::writeWindowPosition()
{
    QSettings windowSettings("Measurement Computing", "Universal Test Linux");

    windowSettings.beginGroup(windowName);

    windowSettings.setValue("geometry", saveGeometry());
    //windowSettings.setValue("savestate", saveState());
    windowSettings.setValue("maximized", isMaximized());
    if (!isMaximized()) {
        windowSettings.setValue("pos", pos());
        windowSettings.setValue("size", size());
    }
    windowSettings.endGroup();
}
