//
// Created by zql on 2026/4/2.
//

#include "NodeParser.h"

#include <QUrl>
#include <QUrlQuery>

QString NodeParser::vlessToClash(const QString& vless)
{
    QUrl url(vless);
    QUrlQuery query(url);

    QString uuid = url.userName();
    QString server = url.host();
    int port = url.port();
    QString name = QUrl::fromPercentEncoding(url.fragment().toUtf8());

    QString network = query.queryItemValue("type");
    QString security = query.queryItemValue("security");
    QString path = QUrl::fromPercentEncoding(query.queryItemValue("path").toUtf8());
    QString host = query.queryItemValue("host");
    QString sni = query.queryItemValue("sni");

    bool tls = (security == "tls");

    QString yaml;
    yaml += "proxies:\n";
    yaml += "  - name: \"" + name + "\"\n";
    yaml += "    type: vless\n";
    yaml += "    server: " + server + "\n";
    yaml += "    port: " + QString::number(port) + "\n";
    yaml += "    uuid: " + uuid + "\n";
    yaml += "    network: " + network + "\n";
    yaml += "    tls: " + QString(tls ? "true\n" : "false\n");
    yaml += "    udp: true\n";

    if (network == "ws") {
        yaml += "    ws-opts:\n";
        yaml += "      path: " + path + "\n";
        if (!host.isEmpty()) {
            yaml += "      headers:\n";
            yaml += "        Host: " + host + "\n";
        }
    }

    if (!sni.isEmpty())
        yaml += "    servername: " + sni + "\n";

    return yaml;
}