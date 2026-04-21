//
// Created by zql on 2026/4/2.
//

#include "node.h"




QString detectType(const QString& link)
{
    if (link.startsWith("vless://")) return "vless";
    if (link.startsWith("vmess://")) return "vmess";
    if (link.startsWith("trojan://")) return "trojan";
    if (link.startsWith("ss://")) return "ss";
    if (link.startsWith("ssr://")) return "ssr";
    if (link.startsWith("hy2://")) return "hysteria2";
    if (link.startsWith("hysteria://")) return "hysteria";
    if (link.startsWith("tuic://")) return "tuic";
    if (link.startsWith("wg://")) return "wireguard";
    return "";
}
QString toVless(Node*n) {
    QString url = "vless://" + n->uuid + "@" + n->server + ":" + QString::number(n->port);

    QUrlQuery query;
    if (!n->network.isEmpty()) query.addQueryItem("type", n->network);
    if (!n->host.isEmpty()) query.addQueryItem("host", n->host);
    if (!n->path.isEmpty()) query.addQueryItem("path", n->path);
    if (n->tls) query.addQueryItem("security", "tls");
    if (!n->sni.isEmpty()) query.addQueryItem("sni", n->sni);

    for (auto it = n->extra.begin(); it != n->extra.end(); ++it)
        query.addQueryItem(it.key(), it.value());

    return url + "?" + query.toString() + "#" + n->name;
}
QString toHy2(Node*n) {
    QString url = "hy2://" + n->password + "@" + n->server + ":" + QString::number(n->port);

    QUrlQuery query;
    if (!n->sni.isEmpty()) query.addQueryItem("sni", n->sni);

    return url + "?" + query.toString() + "#" + n->name;
}
QString toWG(Node*n) {
    QString url = "wg://" + n->server + ":" + QString::number(n->port);

    QUrlQuery query;
    for (auto it = n->extra.begin(); it != n->extra.end(); ++it)
        query.addQueryItem(it.key(), it.value());

    return url + "?" + query.toString();
}
QString toTuic(Node*n) {
    QString url = "tuic://" + n->uuid + ":" + n->password + "@"
        + n->server + ":" + QString::number(n->port);

    QUrlQuery query;
    if (!n->sni.isEmpty()) query.addQueryItem("sni", n->sni);

    return url + "?" + query.toString() + "#" + n->name;
}
QString toHy(Node*n) {
    QString url = "hysteria://" + n->server + ":" + QString::number(n->port);

    QUrlQuery query;
    if (!n->password.isEmpty()) query.addQueryItem("auth", n->password);

    return url + "?" + query.toString();
}
QString toSSR(Node* n) {
    QString base = QString("%1:%2:origin:%3:plain:%4")
        .arg(n->server)
        .arg(n->port)
        .arg(n->method)
        .arg(n->password.toUtf8().toBase64());

    QString params = "remarks=" + n->name.toUtf8().toBase64();

    QString all = base + "/?" + params;

    return "ssr://" + all.toUtf8().toBase64();
}
Node* parseVless(const QString& link)
{
    QUrl url(link);
    QUrlQuery q(url);

    auto n =new  Node();
    n->type = "vless";
    n->uuid = url.userName();
    n->server = url.host();
    n->port = url.port();
    n->name = QUrl::fromPercentEncoding(url.fragment().toUtf8());

    n->network = q.queryItemValue("type");
    n->path = QUrl::fromPercentEncoding(q.queryItemValue("path").toUtf8());
    n->host = q.queryItemValue("host");
    n->tls = (q.queryItemValue("security") == "tls");
    n->sni = q.queryItemValue("sni");
    return n;
}

Node*parseVmess(const QString& link)
{
    QString base64 = link.mid(8);
    QByteArray decoded = QByteArray::fromBase64(base64.toUtf8());

    QJsonObject o = QJsonDocument::fromJson(decoded).object();

    auto n = new Node();

    n->type = "vmess";
    n->server = o["add"].toString();
    n->port = o["port"].toString().toInt();
    n->uuid = o["id"].toString();
    n->name = o["ps"].toString();

    n->network = o["net"].toString();
    n->path = o["path"].toString();
    n->host = o["host"].toString();
    n->tls = (o["tls"].toString() == "tls");

    return n;
}
template<typename T>
T getOrDefault(const YAML::Node& node, const std::string& key, const T& def) {
    if (node[key])
        return node[key].as<T>();
    return def;
}
Node*parseNode(const YAML::Node& proxy) {
    auto n = new Node();

    n->type = QString::fromStdString(getOrDefault(proxy, "type", std::string("")));
    n->name = QString::fromStdString(getOrDefault(proxy, "name", std::string("")));
    n->server = QString::fromStdString(getOrDefault(proxy, "server", std::string("")));
    n->port = getOrDefault(proxy, "port", 0);

    // 通用字段
    n->uuid = QString::fromStdString(getOrDefault(proxy, "uuid", std::string("")));
    n->password = QString::fromStdString(getOrDefault(proxy, "password", std::string("")));
    n->method = QString::fromStdString(getOrDefault(proxy, "cipher", std::string("")));

    n->network = QString::fromStdString(getOrDefault(proxy, "network", std::string("")));
    n->tls = getOrDefault(proxy, "tls", false);
    n->sni = QString::fromStdString(getOrDefault(proxy, "sni", std::string("")));

    // ws
    if (proxy["ws-opts"]) {
        auto ws = proxy["ws-opts"];
        n->path = QString::fromStdString(getOrDefault(ws, "path", std::string("")));
        if (ws["headers"] && ws["headers"]["Host"])
            n->host = QString::fromStdString(ws["headers"]["Host"].as<std::string>());
    }

    // reality / grpc / 其他 → 丢到 extra
    QStringList keys = {
        "uuid", "password", "cipher", "network",
        "path", "host", "sni", "flow"
    };

    for (const auto& k : keys) {
        if (proxy[k.toStdString()]) {
            const auto& value = proxy[k.toStdString()];
            if (value.IsScalar()) {
                n->extra[k] = QString::fromStdString(value.as<std::string>());
            }
        }
    }

    return n;
}
Node*parseTrojan(const QString& link)
{
    QUrl url(link);
    QUrlQuery q(url);

    auto n = new Node();

    n->type = "trojan";
    n->password = url.userName();
    n->server = url.host();
    n->port = url.port();
    n->name = QUrl::fromPercentEncoding(url.fragment().toUtf8());

    n->tls = true;
    n->sni = q.queryItemValue("sni");

    return n;
}
Node* parseSS(const QString& link)
{
    auto n = new Node();

    n->type = "ss";

    QString body = link.mid(5);

    QString decoded;

    if (body.contains("@")) {
        decoded = body;
    } else {
        decoded = QString::fromUtf8(QByteArray::fromBase64(body.toUtf8()));
    }

    // method:password@host:port
    QStringList parts = decoded.split("@");
    QStringList auth = parts[0].split(":");
    QStringList host = parts[1].split(":");

    n->method = auth[0];
    n->password = auth[1];
    n->server = host[0];
    n->port = host[1].toInt();

    return n;
}
Node*parseSSR(const QString& link)
{
    QString base64 = link.mid(6);
    QString decoded = QString::fromUtf8(QByteArray::fromBase64(base64.toUtf8()));

    QStringList parts = decoded.split(":");

    auto n = new Node();

    n->type = "ssr";
    n->server = parts[0];
    n->port = parts[1].toInt();
    n->method = parts[3];
    n->password = QString::fromUtf8(QByteArray::fromBase64(parts[5].toUtf8()));

    return n;
}
Node*parseHysteria2(const QString& link)
{
    QUrl url(link);
    QUrlQuery q(url);

    auto n = new Node();

    n->type = "hysteria2";
    n->server = url.host();
    n->port = url.port();
    n->password = url.userName();
    n->sni = q.queryItemValue("sni");

    return n;
}
Node*parseTuic(const QString& link)
{
    QUrl url(link);
    QUrlQuery q(url);

    auto n = new Node();

    n->type = "tuic";
    n->uuid = url.userName();
    n->password = url.password();
    n->server = url.host();
    n->port = url.port();

    n->sni = q.queryItemValue("sni");

    return n;
}
Node*parseWireGuard(const QString& link)
{
    // wg://base64(json)
    QString base64 = link.mid(5);
    QByteArray decoded = QByteArray::fromBase64(base64.toUtf8());

    QJsonObject o = QJsonDocument::fromJson(decoded).object();

    Node* n = new Node();

    n->type = "wireguard";
    n->server = o["server"].toString();
    n->port = o["port"].toInt();

    n->extra["privateKey"] = o["privateKey"].toString();

    return n;
}
Node*parse(const QString& link)
{
    QString type = detectType(link);

    if (type == "vless") return parseVless(link);
    if (type == "vmess") return parseVmess(link);
    if (type == "trojan") return parseTrojan(link);
    if (type == "ss") return parseSS(link);
    if (type == "ssr") return parseSSR(link);
    if (type == "hysteria2") return parseHysteria2(link);
    if (type == "tuic") return parseTuic(link);
    if (type == "wireguard") return parseWireGuard(link);

    return {};
}
QString toVmess(Node*n) {
    QJsonObject obj;
    obj["v"] = "2";
    obj["ps"] = "converted";
    obj["add"] = n->server;
    obj["port"] = QString::number(n->port);
    obj["id"] = n->uuid;
    obj["aid"] = "0";
    obj["net"] = n->network;
    obj["type"] = "none";
    obj["host"] = n->host;
    obj["path"] = n->path;
    obj["tls"] = n->tls ? "tls" : "";

    QByteArray json = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    return "vmess://" + json.toBase64();
}
QString toSS(Node*n) {
    QString raw = n->method + ":" + n->password + "@" +
                  n->server + ":" + QString::number(n->port);

    return "ss://" + raw.toUtf8().toBase64();
}
QString toTrojan(Node*n) {
    return QString("trojan://%1@%2:%3?security=tls")
        .arg(n->password)
        .arg(n->server)
        .arg(n->port);
}
Node*parseNode(const QString &url) {
    if (url.startsWith("ss://")) return parseSS(url);
    if (url.startsWith("vmess://")) return parseVmess(url);
    if (url.startsWith("vless://")) return parseVless(url);
    if (url.startsWith("trojan://")) return parseTrojan(url);

    return {};
}
