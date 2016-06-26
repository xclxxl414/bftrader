#ifndef GATEWAYMGR_H
#define GATEWAYMGR_H

#include <QObject>
#include <QMap>
#include <QMutex>

class NeZipDrv;

// 1.对于具体合约,建立一个map<string,<string,string>>，把nezip的code映射到ctp的<symbol,exchange>
//   没什么通用办法了，直接写死成一个配置文件吧，人工判断=
//   比如srs07，就映射到<sr607,czce>
//   比如srd01，就映射到<sr701,czce>
//   比如ic07，就映射到<ic1607,ceec>
//   比如ic01，就映射到<ic1701,ceec>
//   或者直接界面上填： nezipCode ctpSymbol ctpExchange，人工填这三个参数=
//   或者api接受这三元组，不用用户填=
//   或者写成配置文件，软件读=（推荐）
// 2.对于加权指数，如ic sr，映射到ic8888 sr888，另外加一个参数表示复制哪个合约的合约信息如ic1607 sr607
//   这样可以往datafeed添加一条合约信息=
//     string nezipCode
//     string ctpSymbol
//     string ctpExchange
//     bool index
//     string ctpCloner
// 3.nezipCode参考 nezip/system/stklabel.csv
//   ctp参考ctpgateway的contract
struct AskDataTag{
    QString nezipCode;
    QString ctpSymbol;
    QString ctpExchange;
    bool index;
    QString ctpCloner;
};

class GatewayMgr : public QObject {
    Q_OBJECT
public:
    explicit GatewayMgr(QObject* parent = 0);
    void init();
    void shutdown();

    QMap<QString,AskDataTag> tags();

signals:

public slots:
    void askData(const AskDataTag& tag);
    void loadDrv();

private:
    NeZipDrv *nezip_ = nullptr;
    QMap<QString,AskDataTag> tags_;
    QMutex mu_;
};

#endif // GATEWAYMGR_H
