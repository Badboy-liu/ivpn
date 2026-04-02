//
// Created by zql on 2026/4/2.
//

#include "Backend.h"

Node Backend::parse(const QString& link) {
    Node node = parseNode(link);
    return node;
}

// Backend.cpp
QString Backend::convert(QString input, QString target) {
    QStringList lines = input.split("\n",Qt::SkipEmptyParts);
    QStringList results;

    for (auto line : lines)
    {
        Node node = parseNode(line);

        QString out;

        if (target == "vmess") out =  toVmess(node);
        if (target == "vless") out =   toVless(node);
        if (target == "trojan") out =   toTrojan(node);
        if (target == "ss") out =   toSS(node);
        if (target == "ssr") out =   toSSR(node);
        if (target == "hysteria2") out =   toHy2(node);
        if (target == "hysteria") out =   toHy(node);
        if (target == "tuic") out =   toTuic(node);
        if (target == "wireguard") out =   toWG(node);
        // qDebug() << "line:" << line;
        // qDebug() << "host:" << n.host;
        // qDebug() << "port:" << n.port;
        // qDebug() << "id:" << n.name;
        results.append(out);
    }

    return results.join("\n");
}

QString Backend::genQr(const QString& text)
{
    return QRCodeGen::generateBase64(text);
}
int Backend::testLatency(const Node& n) {
    QTcpSocket socket;
    QElapsedTimer timer;

    timer.start();
    socket.connectToHost(n.server, n.port);

    if (socket.waitForConnected(2000)) {
        int ms = timer.elapsed();
        socket.disconnectFromHost();
        return ms;
    }

    return 9999;
}

void Backend::startSpeedTest() {
    qDebug() << "Start speedtest";
    QtConcurrent::run([this]() {
        int total = nodes.size();
        for (int i = 0; i < total; ++i) {
            int ms = testLatency(nodes[i]);

            nodes[i].latency = ms;

            // 更新节点名称显示
            QStringList displayList;
            QStringList nodeList;

            for (auto &n: nodes) {
                QString s = n.name + " (" + n.server + ")";
                if (n.latency >= 0) {
                    s += " <span style='color:green;'>[" + QString::number(n.latency) + "ms]</span>";
                    displayList.append(s);
                }

                nodeList << toVless(n);
            }

            emit speedProgress(i + 1, total);
            emit nodesUpdated(displayList, nodeList); // 每测速一个节点就更新 UI
        }
    });
}

// void Backend::loadSubscription(const QString& url) {
//     QByteArray raw = httpGet(url);
//
//     nodes.clear();
//
//     QByteArray decoded = QByteArray::fromBase64(raw);
//     QString text = QString::fromUtf8(decoded);
//
//     for (auto line : text.split("\n")) {
//         line = line.trimmed();
//         if (line.isEmpty()) continue;
//
//         nodes.append(parse(line));
//     }
//
//     QStringList list;
//     for (auto& n : nodes)
//         list << n.name + " (" + n.server + ")";
//
//     emit nodesUpdated(list);
// }
void Backend::loadSubscription(const QString& url) {
    QUrl qurl(url);
    QNetworkRequest request(qurl);
    manager.get(request); // 异步
}

QByteArray httpGet(QString url) {
    QNetworkAccessManager mgr;
    QEventLoop loop;

    auto reply = mgr.get(QNetworkRequest(QUrl(url)));
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    loop.exec();
    return reply->readAll();
}