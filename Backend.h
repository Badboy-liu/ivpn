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
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QFile>
#include <utility>
#include "core/QRProvider.h"
#include <yaml-cpp/yaml.h>

class Backend : public QObject
{
    Q_OBJECT
public:
    NodeModel* nodeModel() { return m_nodeModel; }
    QList<Node*> parseNodeList(QString& content);
    void loadNodes(const QList<Node*> nodes);
    explicit Backend(QObject* parent = nullptr)
        : QObject(parent)
    {
        connect(&manager, &QNetworkAccessManager::finished,
                this, &Backend::onReplyFinished);

        m_nodeModel = new NodeModel(this);

        connect(this, &Backend::latencyReady, this, [this]()
        {


            QList<Node*> filtered;

            for (auto n : m_nodeModel->m_nodes)
            {
                if (n->latency >= 0)
                    filtered.append(n);
            }

            std::sort(filtered.begin(), filtered.end(),
                      [](Node* a, Node* b)
                      {
                          if (a->latency < 0 && b->latency < 0) return false;
                          if (a->latency < 0) return false; // a 在后
                          if (b->latency < 0) return true; // a 在前

                          return a->latency < b->latency;
                      });

            m_nodeModel->setNodes(filtered);

        });
    }
    Q_INVOKABLE void startLocalProxy(const Node &node, quint16 port, const QStringList &mixinKeys = {});
    Q_INVOKABLE void startLocalServer(int port);
    Q_INVOKABLE void openUrl();
    Q_INVOKABLE QString convert(QString input, QString target);
    // Q_INVOKABLE QString convertNode(QString input, QString target);
    Q_INVOKABLE QString genQr(Node* node);

    Q_INVOKABLE void loadSubscription(const QString& url);
    Q_INVOKABLE void startSpeedTest();
    Q_PROPERTY(NodeModel* nodeModel READ nodeModel NOTIFY nodeModelChanged);

signals:
    void latencyReady();
    void nodeModelChanged();
    // void nodesUpdated(QList<Node*> nodes);
    void speedProgress(int current, int total);
    void speedFinished();
private slots:
    void onReplyFinished(QNetworkReply* reply) ;


private:
    NodeModel* m_nodeModel;
    QList<Node*> nodes;
    QNetworkAccessManager manager;
    QProcess *proxyProcess = nullptr;
    QMap<QString, Node*> nodeMap;
    // QString writeConfig(const Node &node, quint16 port);
    QString writeConfig(const Node &node, quint16 port, const QStringList &mixinNodeKeys = {});
    // void startLocalProxy(const Node &node, quint16 port, const QStringList &mixinKeys );

    void stopLocalProxy();

    Node* parse(const QString& link);
    QString toTarget(const std::shared_ptr<Node>&n, const QString& target);

    int testLatency(const Node* n) ;
    // Q_INVOKABLE QString convert(const QString& vless) {
    //     return NodeParser::vlessToClash(vless);
    // }
};



#endif //IVPN_BACKEND_H