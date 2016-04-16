#ifndef CTPCMDMGR_H
#define CTPCMDMGR_H

#include <QObject>
#include <QQueue>

class CThostFtdcMdApi;
class CThostFtdcTraderApi;
class CtpCmd;

//统一管理和派发信号=
//CTP
class CtpCmdMgr : public QObject {
    Q_OBJECT
public:
    explicit CtpCmdMgr(QObject* parent = 0);

    void init();
    void shutdown();

    void setTdApi(CThostFtdcTraderApi* tdapi) { tdapi_ = tdapi; }
    void setMdApi(CThostFtdcMdApi* mdapi) { mdapi_ = mdapi; }
    CThostFtdcMdApi* mdapi() { return mdapi_; }
    CThostFtdcTraderApi* tdapi() { return tdapi_; }

public slots:
    void onRunCmd(void* p, unsigned int delayTick);
    void onReset();

private:
    void setInterval(int ms) { interval_ = ms; }
    void start() { timer_id_ = startTimer(interval_); }
    void stop()
    {
        if (timer_id_ != -1)
            killTimer(timer_id_);
        timer_id_ = -1;
    }
    void timerEvent(QTimerEvent* event) override
    {
        runNow();
    }
    //取第一个然后执行，如果网络限制，就留着下次执行-
    void runNow();

private:
    CThostFtdcTraderApi* tdapi_ = nullptr;
    CThostFtdcMdApi* mdapi_ = nullptr;
    QQueue<CtpCmd*> cmds_;
    int interval_ = 100;
    int timer_id_ = -1;
};

#endif // CTPCMDMGR_H
