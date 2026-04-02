//
// Created by zql on 2026/4/2.
//

#include "http_utils.h"



QByteArray httpGet(const QString& url) {
    QNetworkAccessManager manager;
    QEventLoop loop;

    QUrl qurl(url);
    QNetworkRequest request(qurl);
    QNetworkReply* reply = manager.get(request);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    loop.exec();

    QByteArray data = reply->readAll();
    reply->deleteLater();

    return data;
}