//
// Created by zql on 2026/4/2.
//

#include "Backend.h"
#include "Backend.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QFile>


QString Backend::writeConfig(const Node &node, quint16 port, const QStringList &mixinNodeKeys ) {
    QJsonObject config;

    // ===== inbound：混合模式代理 =====
    QJsonObject inbound;
    inbound["type"] = "mixed";         // mixed 支持 HTTP + SOCKS
    inbound["listen"] = "127.0.0.1";
    inbound["port"] = int(port);

    // mixed 配置
    QJsonObject mixedCfg;
    mixedCfg["tcp"] = true;            // TCP 流量
    mixedCfg["udp"] = false;           // UDP 可选
    inbound["mixed"] = mixedCfg;

    // http 代理支持
    QJsonObject httpCfg;
    httpCfg["enabled"] = true;
    httpCfg["listen"] = "127.0.0.1";
    httpCfg["port"] = int(port);
    inbound["http"] = httpCfg;

    // ===== 主客户端节点 =====
    QJsonObject client;
    client["type"] = node.type;        // vmess/vless/trojan/ss
    client["server"] = node.server;
    client["port"] = node.port;
    if (node.type == "vmess" || node.type == "vless") {
        client["uuid"] = node.uuid.isEmpty() ? node.password : node.uuid;
    }
    if (!node.password.isEmpty()) client["password"] = node.password;

    // clients 数组
    QJsonArray clientsArray;
    clientsArray.append(client);

    // ===== mixin 节点 =====
    for (const QString &key : mixinNodeKeys) {
        if (!nodeMap.contains(key)) continue;
        std::shared_ptr<Node> mNode = nodeMap.value(key);

        QJsonObject mixClient;
        mixClient["type"] = mNode->type;
        mixClient["server"] = mNode->server;
        mixClient["port"] = mNode->port;
        if (mNode->type == "vmess" || mNode->type == "vless") {
            mixClient["uuid"] = mNode->uuid.isEmpty() ? mNode->password : mNode->uuid;
        }
        if (!mNode->password.isEmpty()) mixClient["password"] = mNode->password;

        clientsArray.append(mixClient);
    }

    inbound["clients"] = clientsArray;
    config["inbounds"] = QJsonArray{inbound};

    // ===== outbound：默认直连 =====
    QJsonObject outbound;
    outbound["type"] = "direct";

    QJsonArray outbounds;
    outbounds.append(outbound);
    config["outbounds"] = outbounds;

    // ===== 写入临时文件 =====
    QTemporaryFile tmpFile;
    tmpFile.setAutoRemove(false);  // 退出不删除
    tmpFile.open();
    tmpFile.write(QJsonDocument(config).toJson());
    tmpFile.close();

    return tmpFile.fileName();  // 返回配置路径
}

void Backend::startLocalProxy(const Node &node, quint16 port, const QStringList &mixinKeys) {
    stopLocalProxy();  // 如果已有进程，先关掉

    // 生成支持 mixin 的配置
    QString configFile = writeConfig(node, port, mixinKeys);

    proxyProcess = new QProcess(this);
    connect(proxyProcess, &QProcess::readyReadStandardOutput, [this]() {
        qDebug() << proxyProcess->readAllStandardOutput();
    });
    connect(proxyProcess, &QProcess::readyReadStandardError, [this]() {
        qDebug() << proxyProcess->readAllStandardError();
    });
    connect(proxyProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this](int code, QProcess::ExitStatus status){
        qDebug() << "Local proxy exited with code" << code;
    });

    QString exePath = "backend/sing-box.exe"; // sing-box 可执行路径
    QStringList args = {"run", "-c", configFile};

    proxyProcess->start(exePath, args);
    if (!proxyProcess->waitForStarted(2000)) {
        qWarning() << "Failed to start local proxy";
    } else {
        qDebug() << "Local proxy started at 127.0.0.1:" << port;
        if (!mixinKeys.isEmpty())
            qDebug() << "Mixin nodes:" << mixinKeys;
    }
}

void Backend::stopLocalProxy() {
    if (proxyProcess) {
        proxyProcess->kill();
        proxyProcess->deleteLater();
        proxyProcess = nullptr;
    }
}

std::shared_ptr<Node> Backend::parse(const QString& link) {
    std::shared_ptr<Node> node = parseNode(link);
    return node;
}

// Backend.cpp
QString Backend::convert(QString input, QString target) {
    QStringList lines = input.split("\n",Qt::SkipEmptyParts);
    QStringList results;

    for (auto line : lines)
    {
        std::shared_ptr<Node> node = parseNode(line);

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

QList<Node*> Backend::convertNode(QString input, QString target) {
    QStringList lines = input.split("\n",Qt::SkipEmptyParts);

    QList<Node*> nodes;
    for (auto line : lines)
    {
        std::shared_ptr<Node> node = parseNode(line);

        nodes.append(node.get());
    }

    return nodes;
}


void Backend::openUrl() {
    // 打开默认浏览器访问
    QUrl url("http://127.0.0.1:" + QString::number(12345));
    QDesktopServices::openUrl(url);
}

QString Backend::genQr(std::shared_ptr<Node> node)
{
    return QRCodeGen::generateBase64(toVmess(node));
}
int Backend::testLatency(const std::shared_ptr<Node>&n) {
    QTcpSocket socket;
    QElapsedTimer timer;

    timer.start();
    socket.connectToHost(n->server, n->port);

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

            nodes[i]->latency = ms;

            // 更新节点名称显示
            // QStringList displayList;

            // for (auto &n: nodes) {
            //     QString s = n.name + " (" + n.server + ")";
            //     if (n.latency >= 0) {
            //         s += " <span style='color:green;'>[" + QString::number(n.latency) + "ms]</span>";
            //         displayList.append(s);
            //     }
            // }

            emit speedProgress(i + 1, total);
            emit nodesUpdated(); // 每测速一个节点就更新 UI
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



void Backend::loadNodes(const QList<std::shared_ptr<Node>>& nodes)
{
    m_nodeModel->setNodes(nodes);
    emit nodesUpdated();
}