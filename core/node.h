//
// Created by zql on 2026/4/2.
//

#ifndef IVPN_NODE_H
#define IVPN_NODE_H
#include <QString>
#include <QUrl>
#include <QJsonObject>
#include <QUrlQuery>


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

std::shared_ptr<Node> parseSS(const QString &url);

std::shared_ptr<Node> parseVmess(const QString &url);

std::shared_ptr<Node> parseTrojan(const QString &url);

std::shared_ptr<Node> parseVless(const QString &url);

std::shared_ptr<Node> parseSSR(const QString &url);

std::shared_ptr<Node> parseHysteria2(const QString &url);

std::shared_ptr<Node> parseTuic(const QString &url);

std::shared_ptr<Node> parseWireGuard(const QString &url);

QString toWG(std::shared_ptr<Node> n);

QString toTuic(std::shared_ptr<Node> n);

std::shared_ptr<Node> parseNode(const QString &url);

QString toHy(std::shared_ptr<Node> n);

QString toSS(std::shared_ptr<Node> n);

QString toHy2(std::shared_ptr<Node> n);

QString toSSR(std::shared_ptr<Node> n);

QString toVless(std::shared_ptr<Node> n);

QString toVmess(std::shared_ptr<Node> n);

QString toTrojan(std::shared_ptr<Node> n);


#endif //IVPN_NODE_H
