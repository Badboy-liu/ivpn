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
#include <QUuid>

#include "./core/http_utils.h"
#include "core/NodeModel.h"

#include "core/QRProvider.h"

class Backend : public QObject
{
    Q_OBJECT
public:
    NodeModel* nodeModel() { return m_nodeModel; }

    void loadNodes(const QList<std::shared_ptr<Node>>& nodes);
    explicit Backend(QObject* parent = nullptr)
        : QObject(parent)
    {
        connect(&manager, &QNetworkAccessManager::finished,
                this, &Backend::onReplyFinished);
        m_nodeModel = new NodeModel(this);

    }
    Q_INVOKABLE void startLocalProxy(const Node &node, quint16 port, const QStringList &mixinKeys = {});
    // Q_INVOKABLE void startLocalServer(int port);
    Q_INVOKABLE void openUrl();
    Q_INVOKABLE QString convert(QString input, QString target);
    Q_INVOKABLE QList<Node*> convertNode(QString input, QString target);
    Q_INVOKABLE QString genQr(std::shared_ptr<Node> node);

    Q_INVOKABLE void loadSubscription(const QString& url);
    Q_INVOKABLE void startSpeedTest();
signals:
    void nodesUpdated();
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
        std::shared_ptr<Node> node = parse(line);
        nodes.append(node);
        QString key = node->uuid;
        if (key.isEmpty()) {
            key = QUuid::createUuid().toString(QUuid::WithoutBraces); // 生成 UUID，不带花括号
        }
        nodeModel()->addNode(node);
        nodeMap.insert(key, node);
        // nodeMap.insert(node.uuid, node);
    }


    // convert(text,"vmess");
    emit nodesUpdated();
}
private:
    NodeModel* m_nodeModel;
    QList<std::shared_ptr<Node>> nodes;
    QNetworkAccessManager manager;
    QProcess *proxyProcess = nullptr;
    QMap<QString, std::shared_ptr<Node>> nodeMap;
    // QString writeConfig(const Node &node, quint16 port);
    QString writeConfig(const Node &node, quint16 port, const QStringList &mixinNodeKeys = {});
    // void startLocalProxy(const Node &node, quint16 port, const QStringList &mixinKeys );

    void stopLocalProxy();

    std::shared_ptr<Node> parse(const QString& link);
    QString toTarget(const std::shared_ptr<Node>&n, const QString& target);

    int testLatency(const std::shared_ptr<Node>&n) ;
    // Q_INVOKABLE QString convert(const QString& vless) {
    //     return NodeParser::vlessToClash(vless);
    // }
};



#endif //IVPN_BACKEND_H