#include "gatewaymgr.h"
#include "NeZipDrv.h"
#include "servicemgr.h"
#include <QCoreApplication>
#include <QDir>

GatewayMgr::GatewayMgr(QObject* parent)
    : QObject(parent)
{
}

void GatewayMgr::init()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    // qRegisterMetaType
    qRegisterMetaType<AskDataTag>("AskDataTag");

    nezip_ = new NeZipDrv();
}

void GatewayMgr::shutdown()
{
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    delete nezip_;
    nezip_ = nullptr;
}

void GatewayMgr::loadDrv()
{
    BfInfo(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    QString drvPath = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(QStringLiteral("Nezip/System/Stockdrv.dll"));
    bool ok = nezip_->load(qPrintable(drvPath));
    if (ok) {
        BfInfo("load nezipdrv ok: (%s)", qPrintable(drvPath));
    } else {
        BfInfo("load nezipdrv fail: (%s)", qPrintable(drvPath));
    }
}

void GatewayMgr::askData(const AskDataTag& tag)
{
    //BfInfo(__FUNCTION__);
    g_sm->checkCurrentOn(ServiceMgr::LOGIC);

    if (!nezip_->inited()) {
        BfInfo("please loaddrv first");
        return;
    }

    BfInfo("askdata:(%s)-->(%s.%s)", qPrintable(tag.nezipCode), qPrintable(tag.ctpSymbol), qPrintable(tag.ctpExchange));
    QMutexLocker locker(&mu_);
    if (tags_.contains(tag.nezipCode)) {
        BfInfo("ask already:(%s)", qPrintable(tag.nezipCode));
        return;
    }
    tags_.insert(tag.nezipCode, tag);

    //askServer设置为true则向远程服器同步数据，完成后主动推送数据=
    KLINETYPE klineType = TRACE_KLINE;
    nezip_->askdata(qPrintable(tag.nezipCode), klineType, false, true, 1, true);

    //askServer设置为true则向远程服器同步数据，完成后主动推送数据=
    klineType = MIN1_KLINE;
    nezip_->askdata(qPrintable(tag.nezipCode), klineType, false, true, 1, true);

    //askServer设置为true则向远程服器同步数据，完成后主动推送数据=
    klineType = MIN5_KLINE;
    nezip_->askdata(qPrintable(tag.nezipCode), klineType, false, true, 1, true);

    //askServer设置为true则向远程服器同步数据，完成后主动推送数据=
    klineType = DAY_KLINE;
    nezip_->askdata(qPrintable(tag.nezipCode), klineType, false, true, 1, true);
}

QMap<QString, AskDataTag> GatewayMgr::tags()
{
    QMutexLocker locker(&mu_);
    return tags_;
}
