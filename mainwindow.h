#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSplashScreen>
#include <QTimer>
#include "childwindow.h"
#include "unitest.h"
#include <QHash>
#include "uldaq.h"
#include "errordialog.h"
//#include "plotwindow.h"
#include "trigdialog.h"
#include "eventsdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void keyPressEvent(QKeyEvent *event);
    void closeEvent(QCloseEvent *event);

    void addFunction(QString funcString);
    void setError(UlError curError, QString funcText);
    QHash<QString, DaqDeviceHandle> getListedDevices();
    void addDeviceToMenu(QString devName, QString devUiD, DaqDeviceHandle devHandle);
    void removeDeviceFromMenu(QString devUiD);

    ErrorDialog errDlg;

private slots:
    void discoverDevices();
    void setTimer();
    void readTmrParams();
    void disableTimer();
    void createDiscChild();
    void createCfgChild();
    void createMiscChild();
    void createAiChild();
    void createAoChild();
    void createDinChild();
    void createDOutChild();
    void createCtrChild();
    void setSelectedDevice();
    void setBoardMenuSelect(QMdiSubWindow*);
    void curFunctionChanged();
    void curRangeChanged();
    void curScaleChanged();
    void curInputModeChanged();
    void curIoOptionChanged();
    void curOptionChanged();
    void changeTrigType();
    void setAiFlags();
    void showPlot(bool showIt);
    void setTriggerParameters();
    void trigDialogResponse();
    void configureData();
    void configureQueue();
    void syncRange(Range childRange);
    void syncScale(TempScale childScale);
    void showEventSetup();
    void eventDialogResponse();
    void showHistory();
    void showAbout();
    void delayTimer();

private:
    Ui::MainWindow *ui;
    ChildWindow *activeMdiChild() const;
    QSplashScreen *mSplash;
    QTimer *tmrDelay;
    void updateInventory();
    void createChild(UtFunctionGroup, int);
    void createMenus();
    void createFuncMenus();
    ScanOption getSoMask(ScanOption optSelected);

    QStringList mFuncHistoryList;
    QAction *bdAction;
    QActionGroup *optionGroup;
    QActionGroup *rangeGroup;
    QActionGroup *scaleGroup;
    QActionGroup *functionGroup;
    QActionGroup *inputModeGroup;
    QActionGroup *trigTypeGroup;

    QString curFunctionGroupName;
    QString curBoardName;
    QString curUniqueID;

    ScanOption mScanOptions;

    //PlotWindow *plotWindow;
    TrigDialog *trigDialog;
    EventsDialog *eventSetup;
    int mHistListSize;
    int mTrigChannel;
    int mTrigChanType;
    Range mTrigRange;
    double mTrigLevel;
    double mTrigVariance;
    unsigned int mRetrigCount;
    void scanStopRequested(ChildWindow *curChild);
    void readWindowPosition();
    void writeWindowPosition();
    bool mAutoConnect;
    bool mTimesUp;

signals:
    void devListCountChanged(int);
    void trigTypeChanged();

};

#endif // MAINWINDOW_H
