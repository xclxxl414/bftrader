#ifndef CTPMGR_H
#define CTPMGR_H

#include "bftrader.pb.h"
#include "ringbuffer.h"

#include <QMap>
#include <QObject>
#include <QQueue>
#include <QStringList>
#include <QTimer>
#include <functional>

using namespace bftrader;

class MdSm;
class TdSm;
class Logger;
class Profile;

// todo(hege):增加一个错误处理函数,等req返回了可以找到cmd然后执行，之后才删除=
struct CtpCmd {
    std::function<int(int)> fn;
    quint32 delayTick;
    quint32 expires;
};

// 1.完成登录/自动登录/确认账单/订阅合约，负责行情（tick+contract）的高效维护（ringbuffer+map）
// 2.订阅什么合约由gateway统一确定，策略不管这事=
// 3.在queryInstrument之前重新初始化后界面，1秒后重新初始化内存+开始查询就可以了，这样就不会有问题了=
//   onGotContracts后开始刷新界面，queryInstrument+login都延迟一秒，用途之一就是这个=
// 4. tdsm/mdsm/ctputils的c++文件能包括ctp头文件，其他文件不准包含，便于移植=
class CtpMgr : public QObject {
    Q_OBJECT
public:
    explicit CtpMgr(QObject* parent = 0);
    void init();
    void shutdown();

    // 可跨线程调用=
    bool running();

    // tick+contract高效维护=
    void resetData();

    QStringList getIds();
    QStringList getIdsAll();
    void insertContract(QString id, void* contract);
    void* getContract(QString id);
    void freeContracts();

    void initRingBuffer(int itemLen, QStringList ids);
    void freeRingBuffer();
    RingBuffer* getRingBuffer(QString id);
    void* getLatestTick(QString id);
    void* getPreLatestTick(QString id);

    // 分配BfOrderId
    QString genOrderId();

signals:
    void requestSent(int reqId);
    void tradeWillBegin();
    void gotContracts(QStringList ids, QStringList idsAll);
    void gotTick(void* curTick, void* preTick);
    void gotAccount(const BfAccountData& account);
    void gotOrder(const BfOrderData& order);
    void gotTrade(const BfTradeData& trade);
    void gotPosition(const BfPositionData& pos);
    void gotCtpError(int code, QString msg, QString msgEx);

public slots:
    void showVersion();
    void runCmd(CtpCmd* cmd);
    void start(QString password);
    void stop();
    void queryAccount();
    void sendOrderWithId(QString byOrderId, const BfSendOrderReq& req);
    void sendOrder(const BfSendOrderReq& req);
    void queryPosition();
    void cancelOrder(const BfCancelOrderReq& req);
    void queryOrders();

private slots:
    void onGotContracts(QStringList ids, QStringList idsAll);
    void onMdSmStateChanged(int state);
    void onTdSmStateChanged(int state);
    void onRunCmdInterval();

private:
    Profile* profile();
    Logger* logger();
    bool initMdSm();
    void startMdSm();
    bool initTdSm();
    void startTdSm();
    void tryStartSubscrible();
    void resetCmds();

private:
    MdSm* mdsm_ = nullptr;
    bool mdsm_logined_ = false;
    TdSm* tdsm_ = nullptr;
    bool tdsm_logined_ = false;
    bool autoLoginTd_ = true;
    bool autoLoginMd_ = true;
    QString password_;

    QMap<QString, void*> contracts_;
    QMap<QString, RingBuffer*> rbs_;
    const int ringBufferLen_ = 256;
    QStringList ids_;
    QStringList ids_all_;

    QQueue<CtpCmd*> cmds_;
    int reqId_ = 0;
    QTimer* cmdRunnerTimer_ = nullptr;
};

#endif // CTPMGR_H
