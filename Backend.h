//
// Created by zql on 2026/4/2.
//

#ifndef IVPN_BACKEND_H
#define IVPN_BACKEND_H
#include <qobjectdefs.h>
#include <QString>
#include <qtmetamacros.h>

#include "core/node.h"
#include "core/QRCodeGen.h"
#include "core/NodeParser.h"
#include <QObject>   // 👈 必须加这个！
#include <qeventloop.h>
#include <QNetworkAccessManager.h>
#include <QElapsedTimer.h>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher.h>
#include <QNetworkReply>

#include "./core/http_utils.h"

#include "core/QRProvider.h"

class Backend : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE QString convert(QString input, QString target);
    Q_INVOKABLE QString genQr(const QString& text);

    Q_INVOKABLE void loadSubscription(const QString& url);
    Q_INVOKABLE void startSpeedTest();
signals:
    void nodesUpdated(QStringList list);
    void speedProgress(int current, int total);
    void speedFinished();
private:
    QList<Node> nodes;

    Node parse(const QString& link);
    QString toTarget(const Node& n, const QString& target);

    int testLatency(const Node& n);
    // Q_INVOKABLE QString convert(const QString& vless) {
    //     return NodeParser::vlessToClash(vless);
    // }
};



#endif //IVPN_BACKEND_H