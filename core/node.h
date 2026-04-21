//
// Created by zql on 2026/4/2.
//

#ifndef IVPN_NODE_H
#define IVPN_NODE_H
#include <QString>
#include <QUrl>
#include <QJsonObject>
#include <QUrlQuery>
#include <yaml-cpp/yaml.h>

class  Node : public QObject
{
    Q_OBJECT
    public:
    QString type; // vless / vmess / trojan / ss / ssr / hysteria / tuic / wg
    QString name;
    QString server;
    int port = 0;
    int latency = -1; // 默认未测速
    QString uuid;
    QString password;
    QString method; // ss 用

    QString network; // ws / tcp / grpc
    QString path;
    QString host;

    bool tls = false;
    QString sni;

    // 额外字段
    QMap<QString, QString> extra;

    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString server MEMBER server)
    Q_PROPERTY(QString uuid MEMBER uuid)
    Q_PROPERTY(int latency MEMBER latency)
};


// 只声明
QString detectType(const QString &url);
Node* parseNode(const YAML::Node& proxy);
Node* parseSS(const QString &url);

Node* parseVmess(const QString &url);

Node* parseTrojan(const QString &url);

Node* parseVless(const QString &url);

Node* parseSSR(const QString &url);

Node* parseHysteria2(const QString &url);

Node* parseTuic(const QString &url);

Node* parseWireGuard(const QString &url);

QString toWG(Node* n);

QString toTuic(Node* n);

Node* parseNode(const QString &url);

QString toHy(Node* n);

QString toSS(Node* n);

QString toHy2(Node* n);

QString toSSR(Node* n);

QString toVless(Node* n);

QString toVmess(Node* n);

QString toTrojan(Node* n);


#endif //IVPN_NODE_H
