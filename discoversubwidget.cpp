#include "discoversubwidget.h"
#include "ui_discoversubwidget.h"
#include "errordialog.h"
#include <QMouseEvent>

DiscoverSubWidget::DiscoverSubWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DiscoverSubWidget)
{
    ui->setupUi(this);
    QString str;

    ui->leNumDevs->setText(str.setNum(MAX_DEV_COUNT));
    ui->lblInfo->setStyleSheet("QLabel { color : blue; }" );
    ui->textEdit->setStyleSheet("QTextEdit { background-color : white; color : blue; }");
    ui->lblDevicesFound->setStyleSheet("QLabel { color : blue; }" );
    connect(ui->listWidget, SIGNAL(itemSelectionChanged()),
                     this, SLOT(on_listWidget_itemSelectionChanged()));
    //connect(ui->cmdCreate, SIGNAL(clicked()), MainWindow, SLOT(());
    connect(ui->cmdDiscover, SIGNAL(clicked(bool)), this, SLOT(on_actionRefresh_Devices_triggered()));
    connect(ui->cmdDescriptor, SIGNAL(clicked(bool)), this, SLOT(getDescriptor()));
    connect(ui->cmdIsConnected, SIGNAL(clicked(bool)), this, SLOT(checkConnection()));
    ui->textEdit->setTabStopWidth(50);
    mMainWindow = getMainWindow();

}

DiscoverSubWidget::~DiscoverSubWidget()
{
    delete ui;
}

MainWindow* DiscoverSubWidget::getMainWindow()
{
    foreach (QWidget *w, QApplication::topLevelWidgets())
        if (QMainWindow* mainWin = qobject_cast<QMainWindow*>(w))
            return qobject_cast<MainWindow *>(mainWin);
    return nullptr;
}

void DiscoverSubWidget::updateParameters()
{

}

void DiscoverSubWidget::functionChanged(int utFunction)
{
    if (utFunction == UL_DISC) {
        //mDevListCount = MainWindow.devListCount();
        on_actionRefresh_Devices_triggered();
    }
}

void DiscoverSubWidget::groupChanged(int newGroup)
{
    mCurGroup = newGroup;
}

void DiscoverSubWidget::on_listWidget_itemSelectionChanged()
{
    updateList();
}

void DiscoverSubWidget::runSelectedFunc()
{

}

void DiscoverSubWidget::setUiForGroup()
{

}

void DiscoverSubWidget::setUiForFunction()
{

}

void DiscoverSubWidget::updateList()
{
    QString ifc;
    QString dStr = "Device string: ";
    QString uidStr = "Unique ID: ";
    QString pidStr = "Product ID: 0x";

    QListWidgetItem *curItem = ui->listWidget->currentItem();
    //int devListed = ui->listWidget->currentRow();
    int descriptorIndex = curItem->data(Qt::UserRole).toInt();
    DaqDeviceInterface devInterface = devDescriptors[descriptorIndex].devInterface;
    ui->cmdCreate->setEnabled(true);

    switch (devInterface) {
    case USB_IFC:
        ifc = "USB";
        break;
    case BLUETOOTH_IFC:
        ifc = "Bluetooth";
        break;
    case ETHERNET_IFC:
        ifc = "Ethernet";
        break;
    case ANY_IFC:
        ifc = "All";
        break;
    }

    uint pId = devDescriptors[descriptorIndex].productId;
    pidStr.append(QString::number(pId, 16));
    mUidString = devDescriptors[descriptorIndex].uniqueId;

    ui->textEdit->setText(devDescriptors[descriptorIndex].productName);
    ui->textEdit->append(dStr.append(devDescriptors[descriptorIndex].devString));
    ui->textEdit->append(uidStr.append(mUidString));
    ui->textEdit->append(pidStr);
    ui->textEdit->append(QString("Interface: %1").arg(ifc));
    DaqDeviceHandle devHandle = devList.value(mUidString);
    ui->textEdit->append(QString("Device handle: %1").arg(devHandle));
    ui->cmdCreate->setEnabled(devHandle < 1);
    ui->cmdRelease->setEnabled(devHandle > 0);
    //ui->cmdConnect->setEnabled(devHandle);
    //ui->cmdDisconnect->setEnabled(devHandle);
    ui->cmdDescriptor->setEnabled(devHandle);
    mDaqDeviceHandle = devHandle;
    if (mDaqDeviceHandle == 0) {
        ui->cmdCreate->setEnabled(true);
        ui->cmdRelease->setEnabled(false);
        ui->cmdDisconnect->setEnabled(false);
        ui->cmdConnect->setEnabled(false);
        ui->cmdDescriptor->setEnabled(false);
    } else
        checkConnection();
}

void DiscoverSubWidget::on_cmdCreate_clicked()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;

    int itemsListed = ui->listWidget->count();

    if (itemsListed > 0) {
        QListWidgetItem *curItem = ui->listWidget->currentItem();
        int descriptorIndex = curItem->data(Qt::UserRole).toInt();
        QString uidKey = devDescriptors[descriptorIndex].uniqueId;

        nameOfFunc = "ulCreateDaqDevice";
        funcArgs = "({productName, productId, devInterface, devString, uniqueId})\n";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        mDaqDeviceHandle = ulCreateDaqDevice(devDescriptors[descriptorIndex]);
        argVals = QStringLiteral("(%1, {%2, %3, %4, %5})")
                .arg(devDescriptors[descriptorIndex].productName)
                .arg(devDescriptors[descriptorIndex].productId)
                .arg(devDescriptors[descriptorIndex].devInterface)
                .arg(devDescriptors[descriptorIndex].devString)
                .arg(devDescriptors[descriptorIndex].uniqueId);
        ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

        funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
        if (mDaqDeviceHandle != 0) {
            devList.remove(uidKey);
            devList.insert(uidKey, mDaqDeviceHandle);
            QString devName = devDescriptors[descriptorIndex].productName;
            mMainWindow->addDeviceToMenu(devName, uidKey, mDaqDeviceHandle);
            updateList();
            checkConnection();
        }
        //updateConnectionStatus();
    }
}

void DiscoverSubWidget::on_cmdRelease_clicked()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;

    nameOfFunc = "ulReleaseDaqDevice";
    funcArgs = "(mDaqDeviceHandle)\n";
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulReleaseDaqDevice(mDaqDeviceHandle);
    argVals = QStringLiteral("(%1)")
            .arg(mDaqDeviceHandle);
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err != ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->removeDeviceFromMenu(mUidString);
        devList.remove(mUidString);
        mMainWindow->addFunction(sStartTime + funcStr);
        updateList();
    }
}

void DiscoverSubWidget::on_cmdDisconnect_clicked()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;

    nameOfFunc = "ulDisconnectDaqDevice";
    funcArgs = "(mDaqDeviceHandle)\n";
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulDisconnectDaqDevice(mDaqDeviceHandle);
    argVals = QStringLiteral("(%1)")
            .arg(mDaqDeviceHandle);
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err != ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
        //updateList();
        checkConnection();
    }
}

void DiscoverSubWidget::on_cmdConnect_clicked()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;

    nameOfFunc = "ulConnectDaqDevice";
    funcArgs = "(mDaqDeviceHandle)\n";
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulConnectDaqDevice(mDaqDeviceHandle);
    argVals = QStringLiteral("(%1)")
            .arg(mDaqDeviceHandle);
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err != ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
        //updateList();
        checkConnection();
    }
}

void DiscoverSubWidget::updateConnectionStatus()
{
    int connected;
    bool isConnected;
    QString nameOfFunc, funcArgs, argVals;
    QString funcStr, sStartTime;
    QTime t;

    nameOfFunc = "ulIsDaqDeviceConnected";
    funcArgs = "(mDaqDeviceHandle, &connected)\n";

    if (mDaqDeviceHandle != 0) {
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulIsDaqDeviceConnected(mDaqDeviceHandle, &connected);
        argVals = QStringLiteral("(%1, %2)")
                .arg(mDaqDeviceHandle)
                .arg(connected);

        funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
        if (err != ERR_NO_ERROR) {
            ErrorDialog errDlg;
            errDlg.setModal(true);
            errDlg.setError(err, funcStr);
            errDlg.exec();
        } else {
            mMainWindow->addFunction(sStartTime + funcStr);
            isConnected = connected;
            ui->cmdConnect->setEnabled(!isConnected);
            ui->cmdDisconnect->setEnabled(isConnected);
        }
    } else {
        ui->cmdDisconnect->setEnabled(false);
        ui->cmdConnect->setEnabled(false);
    }
}

void DiscoverSubWidget::on_actionRefresh_Devices_triggered()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;
    unsigned int numDevs;   // = MAX_DEV_COUNT
    QString uidKey;
    DaqDeviceHandle existingDevHandle;

    numDevs = ui->leNumDevs->text().toUInt();
    QHash<QString, DaqDeviceHandle> existingList = mMainWindow->getListedDevices();
    DaqDeviceInterface interfaceType = ANY_IFC;

    nameOfFunc = "ulGetDaqDeviceInventory";
    funcArgs = "(interfaceType, devDescriptors, &numDevs)\n";
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulGetDaqDeviceInventory(interfaceType, devDescriptors, &numDevs);
    argVals = QStringLiteral("(%1, %2, %3)")
            .arg(interfaceType)
            .arg("devDescriptors")
            .arg(numDevs);
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));
    ui->lblDevicesFound->setText(QString("Found: %1").arg(numDevs));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err != ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
        ui->listWidget->clear();
        ui->textEdit->clear();
        ui->cmdCreate->setEnabled(false);
        ui->cmdRelease->setEnabled(false);
        ui->cmdConnect->setEnabled(false);
        ui->cmdDisconnect->setEnabled(false);
        if (numDevs) {
            for (uint i=0;i<numDevs;i++) {
                existingDevHandle = 0;
                QListWidgetItem *newItem = new QListWidgetItem;
                newItem->setData(Qt::UserRole, QVariant(i));
                newItem->setText(devDescriptors[i].productName);
                ui->listWidget->addItem(newItem);
                ui->listWidget->setCurrentItem(newItem);
                uidKey = devDescriptors[i].uniqueId;
                if (existingList.contains(uidKey)) {
                    existingDevHandle = existingList.value(uidKey);
                    existingList.remove(uidKey);
                }
                mDaqDeviceHandle = existingDevHandle;
                devList.insert(uidKey, mDaqDeviceHandle);
                updateList();
            }
        } else
            ui->listWidget->addItem("No device detected");

        //any items left in existingList weren't detected, so
        //must be removed from the board menu and mainwindow list
        if (existingList.count()) {
            foreach (QString key, existingList.keys()) {
                mMainWindow->removeDeviceFromMenu(key);
            }
        }
    }
}

void DiscoverSubWidget::refreshSelDevice()
{
    QString uIdKey;
    int itemsListed = ui->listWidget->count();

    for (int i = 0; i < itemsListed; i++) {
        QListWidgetItem *curItem = ui->listWidget->item(i);
        int descriptorIndex = curItem->data(Qt::UserRole).toInt();
        uIdKey = devDescriptors[descriptorIndex].uniqueId;
        if (devList.contains(uIdKey)) {
            if (mDaqDeviceHandle == devList.value(uIdKey)) {
                ui->listWidget->setCurrentItem(ui->listWidget->item(i));
                ui->cmdRelease->setEnabled(true);
                ui->cmdConnect->setEnabled(true);
                ui->cmdDisconnect->setEnabled(true);
                updateList();
                break;
            }

        }
    }
}

void DiscoverSubWidget::getDescriptor()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;
    DaqDeviceDescriptor descriptor;

    nameOfFunc = "ulGetDaqDeviceDescriptor";
    funcArgs = "(mDaqDeviceHandle, {productName, productId, devInterface, devString, uniqueId})\n";
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulGetDaqDeviceDescriptor(mDaqDeviceHandle, &descriptor);
    argVals = QStringLiteral("(%1, {%2, %3, %4, %5, %6})")
            .arg(mDaqDeviceHandle)
            .arg(descriptor.productName)
            .arg(descriptor.productId)
            .arg(descriptor.devInterface)
            .arg(descriptor.devString)
            .arg(descriptor.uniqueId);
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err != ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
    }
}

void DiscoverSubWidget::checkConnection()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;
    int connected;

    nameOfFunc = "ulIsDaqDeviceConnected";
    funcArgs = "(mDaqDeviceHandle, connected)\n";
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulIsDaqDeviceConnected(mDaqDeviceHandle, &connected);
    argVals = QStringLiteral("(%1, %2)")
            .arg(mDaqDeviceHandle)
            .arg(connected);
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err != ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
        ui->cmdDisconnect->setEnabled(connected);
        ui->cmdConnect->setEnabled(!connected);
    }
}
