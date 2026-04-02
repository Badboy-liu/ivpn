//
// Created by zql on 2026/4/2.
//

#ifndef IVPN_NODE_H
#define IVPN_NODE_H
#include <QString>
#include <QUrl>
#include <QJsonObject>
#include <QUrlQuery>



struct Node {
    QString type;      // vless / vmess / trojan / ss / ssr / hysteria / tuic / wg

    QString name;
    QString server;
    int port = 0;

    QString uuid;
    QString password;
    QString method;    // ss 用

    QString network;   // ws / tcp / grpc
    QString path;
    QString host;

    bool tls = false;
    QString sni;

    // 额外字段
    QMap<QString, QString> extra;
};


// 只声明
QString detectType(const QString &url);

Node parseSS(const QString &url);
Node parseVmess(const QString &url);
Node parseTrojan(const QString &url);
Node parseVless(const QString &url);
Node parseSSR(const QString &url);
Node parseHysteria2(const QString &url);
Node parseTuic(const QString &url);
Node parseWireGuard(const QString &url);
QString toWG(const Node& n);
QString toTuic(const Node& n) ;
Node parseNode(const QString &url);
QString toHy(const Node& n);
QString toSS(const Node &n);
QString toHy2(const Node& n);
QString toSSR(const Node& n) ;
QString toVless(const Node& n);
QString toVmess(const Node &n);
QString toTrojan(const Node &n);



#endif //IVPN_NODE_H