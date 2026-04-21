//
// Created by zql on 2026/4/2.
//

#include "Backend.h"



void Backend::onReplyFinished(QNetworkReply* reply)
{
    QByteArray raw = reply->readAll();
    reply->deleteLater();

    nodes.clear();
    QByteArray::FromBase64Result result = QByteArray::fromBase64Encoding(
         raw,
         QByteArray::AbortOnBase64DecodingErrors);

    QString text = QString::fromUtf8(result.decoded);

    m_nodeModel->setNodes(parseNodeList(text));

}


QList<Node*> Backend::parseNodeList(QString& content)
{
    QList<Node*> nodes;
    if (content.contains("proxies:") || content.contains("proxies:"))
    {


        // YAML::Node yml = YAML::Load(yamlStr);
        YAML::Node root = YAML::Load(content.toStdString());
        YAML::Node ps = root["proxies"];
        if (!ps.IsNull())
        {
            for (const auto& item : ps)
            {
                auto node = parseNode(item);
                nodeMap.insert(node->uuid, node);
                nodes.append(node);
            }
        }
    }else
    {


        for (auto line : content.split("\n")) {
            line = line.trimmed();
            if (line.isEmpty()) continue;
            Node* node = parse(line);
            nodes.append(node);
            QString key = node->uuid;
            if (key.isEmpty()) {
                key = QUuid::createUuid().toString(QUuid::WithoutBraces); // 生成 UUID，不带花括号
            }
            nodeModel()->addNode(node);
            nodeMap.insert(key, node);
            // nodes.append(node);
            // nodeMap.insert(node.uuid, node);
        }
    }
    return nodes;
};

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
        Node *mNode = nodeMap.value(key);

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

Node* Backend::parse(const QString& link) {
    Node* node = parseNode(link);
    return node;
}

// Backend.cpp
QString Backend::convert(QString input, QString target) {

    QStringList lines = input.split("\n",Qt::SkipEmptyParts);
    QList<Node*> nodes;

    if (input.trimmed().isEmpty())
    {
        nodes = this->m_nodeModel->m_nodes;
    }else
    {
        nodes = parseNodeList(input);
    }
    QStringList results;
    if (!nodes.isEmpty())
    {

        for (auto node : nodes)
        {
            if (target=="vmess")
            {
                results.append(toVmess(node));
            }else if (target == "vless")
            {
                results.append(toVmess(node));
            }else if (target == "wg")
            {
                results.append(toWG(node));
            }else if (target == "tuic")
            {
                results.append(toTuic(node));
            }else if (target == "hy")
            {
                results.append(toHy(node));
            }else if (target == "ss")
            {
                results.append(toSS(node));
            }else if (target == "hy2")
            {
                results.append(toHy2(node));
            }else if (target == "ssr")
            {
                results.append(toSSR(node));
            }else if (target == "trojan")
            {
                results.append(toTrojan(node));
            }
        }

    }
    m_nodeModel->setNodes(nodes);
    return results.join("\n");
}

// QString Backend::convertNode(QString input, QString target) {
//
// }


void Backend::openUrl() {
    // 打开默认浏览器访问
    QUrl url("http://127.0.0.1:" + QString::number(12345));
    QDesktopServices::openUrl(url);
}

QString Backend::genQr(Node* node)
{
    return QRCodeGen::generateBase64(toVmess(node));
}
int Backend::testLatency(const Node* n) {
    QTcpSocket socket;
    QElapsedTimer timer;

    timer.start();
    socket.connectToHost(n->server, n->port);

    if (socket.waitForConnected(2000)) {
        int ms = timer.elapsed();
        socket.disconnectFromHost();
        return ms;
    }

    return -1;
}

void Backend::startSpeedTest() {
    qDebug() << "Start speedtest";
    QtConcurrent::run([this]() {
        int total = m_nodeModel->m_nodes.size();
        for (int i = 0; i < total; ++i) {
            int ms = testLatency(m_nodeModel->m_nodes[i]);

            m_nodeModel->m_nodes[i]->latency = ms;

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
             // 每测速一个节点就更新 UI
            // emit nodesUpdated(m_nodeModel->m_nodes);
        }

        emit latencyReady();
    });
    // m_nodeModel->setNodes(m_nodeModel->m_nodes);
}

void Backend::startLocalServer(int port)
{

    QTcpServer* server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, [this, server]() {
        QTcpSocket *socket = server->nextPendingConnection();

        QStringList nodeStrings;
        for (auto &n : m_nodeModel->m_nodes)
            nodeStrings << toVmess(n); // 或 toVless / toTrojan

        QByteArray allNodes = nodeStrings.join("\n").toUtf8().toBase64();

        QByteArray response = "HTTP/1.1 200 OK\r\n"
                              "Content-Type: text/plain\r\n"
                              "Content-Length: " + QByteArray::number(allNodes.size()) + "\r\n"
                              "\r\n" +
                              allNodes;

        socket->write(response);
        socket->flush();
        socket->disconnectFromHost();
    });

    if (!server->listen(QHostAddress::LocalHost, port)) {
        qWarning() << "Failed to start local server on port" << port;
    } else {
        qDebug() << "Local node server started at http://127.0.0.1:" << port;
    }
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



void Backend::loadNodes(const QList<Node*> nodes)
{
    m_nodeModel->setNodes(nodes);
    // emit nodesUpdated();
}