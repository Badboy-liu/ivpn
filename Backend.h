//
// Created by zql on 2026/4/2.
//

#ifndef IVPN_BACKEND_H
#define IVPN_BACKEND_H
#include <qobjectdefs.h>
#include <QString>
#include <qtmetamacros.h>
#include <QJsonArray>
#include <QJsonObject>
#include <QByteArray>
#include <QDesktopServices>
#include <QTcpServer>
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
    explicit Backend(QObject* parent = nullptr)
        : QObject(parent)
    {
        connect(&manager, &QNetworkAccessManager::finished,
                this, &Backend::onReplyFinished);
    }
    Q_INVOKABLE void startLocalServer(int port);
    Q_INVOKABLE void openUrl();
    Q_INVOKABLE QString convert(QString input, QString target);
    Q_INVOKABLE QString genQr(const QString& text);

    Q_INVOKABLE void loadSubscription(const QString& url);
    Q_INVOKABLE void startSpeedTest();
signals:
    void nodesUpdated(QStringList list,QStringList nodeList);
    void speedProgress(int current, int total);
    void speedFinished();

private slots:
    void onReplyFinished(QNetworkReply* reply) {
    QByteArray raw = reply->readAll();
    reply->deleteLater();

    nodes.clear();
    QByteArray decoded = QByteArray::fromBase64(raw);
    QString text = QString::fromUtf8(decoded);

    for (auto line : text.split("\n")) {
        line = line.trimmed();
        if (line.isEmpty()) continue;

        nodes.append(parse(line));
    }

    QStringList list;
    QStringList nodeList;
    for (auto& n : nodes) {
        list << n.name + " (" + n.server + ")";
        nodeList<<toVless(n);
    }



    // convert(text,"vmess");
    emit nodesUpdated(list,nodeList);
}
private:
    QList<Node> nodes;
    QNetworkAccessManager manager;
    Node parse(const QString& link);
    QString toTarget(const Node& n, const QString& target);

    int testLatency(const Node& n);
    // Q_INVOKABLE QString convert(const QString& vless) {
    //     return NodeParser::vlessToClash(vless);
    // }
};



#endif //IVPN_BACKEND_H