#ifndef QUEUEDIALOG_H
#define QUEUEDIALOG_H

#include <QDialog>
#include "uldaq.h"
#include "testutilities.h"

namespace Ui {
class QueueDialog;
}

class QueueDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QHash<int, int> chanList READ chanList WRITE setChanList NOTIFY chanListChanged)
    Q_PROPERTY(QHash<int, AiInputMode> modeList READ modeList WRITE setModeList NOTIFY modeListChanged)
    Q_PROPERTY(QHash<int, Range> rangeList READ rangeList WRITE setRangeList NOTIFY rangeListChanged)
    Q_PROPERTY(int numQueueElements READ numQueueElements WRITE setNumQueueElements NOTIFY numQueueElementsChanged)
    Q_PROPERTY(QHash<int, DaqOutChanType> chanOutTypeList READ chanOutTypeList WRITE setChanOutTypeList NOTIFY chanOutTypeListChanged)

public:
    explicit QueueDialog(QWidget *parent = 0, UtFunctionGroup utFuncGroup = FUNC_GROUP_AIN);
    ~QueueDialog();

    void setChanList(QHash<int, int> chanList)
    {
        mChanList = chanList;
        emit chanListChanged(chanList);
    }

    void setChanTypeList(QHash<int, DaqInChanType> chanTypeList)
    {
        mChanTypeList = chanTypeList;
        emit chanTypeListChanged(chanTypeList);
    }

    void setChanOutTypeList(QHash<int, DaqOutChanType> chanOutTypeList)
    {
        mChanOutTypeList = chanOutTypeList;
        emit chanOutTypeListChanged(chanOutTypeList);
    }

    void setModeList(QHash<int, AiInputMode> modeList)
    {
        mModeList = modeList;
        emit modeListChanged(modeList);
    }

    void setRangeList(QHash<int, Range> rangeList)
    {
        mRangeList = rangeList;
        emit rangeListChanged(rangeList);
    }

    void setNumQueueElements(int numElements)
    {
        mNumQueueElements = numElements;
        emit numQueueElementsChanged(numElements);
    }

    QHash<int, int>           chanList() { return mChanList; }
    QHash<int, AiInputMode>   modeList() { return mModeList; }
    QHash<int, Range>         rangeList() {return mRangeList; }
    QHash<int, DaqInChanType> chanTypeList() { return mChanTypeList; }
    QHash<int, DaqOutChanType> chanOutTypeList() { return mChanOutTypeList; }
    int numQueueElements () { return mNumQueueElements; }

private slots:
    void updateNumElements(int newNumElements);
    void lstQueueRowSelected(int rowSelected);
    void onLoadQueue();
    void chanTypeSelection(int);
    void autoFill();
    void deleteElement();
    void dlgDone();

private:
    Ui::QueueDialog *ui;

    QHash<int, int>           mChanList;
    QHash<int, AiInputMode>   mModeList;
    QHash<int, Range>         mRangeList;
    QHash<int, DaqInChanType> mChanTypeList;
    QHash<int, DaqOutChanType> mChanOutTypeList;
    int mCurElement = 0;
    int mNumQueueElements = 0;
    bool inputFunctions = false;
    bool isAutoSet = false;

    void updateQueueList(int newNumElements);

signals:
    void chanListChanged(QHash<int, int>);
    void chanTypeListChanged(QHash<int, DaqInChanType>);
    void chanOutTypeListChanged(QHash<int, DaqOutChanType>);
    void modeListChanged(QHash<int, AiInputMode>);
    void rangeListChanged(QHash<int, Range>);
    void numQueueElementsChanged(int);
};

#endif // QUEUEDIALOG_H
