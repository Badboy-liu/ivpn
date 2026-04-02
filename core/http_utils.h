//
// Created by zql on 2026/4/2.
//

#ifndef IVPN_HTTP_UTILS_H
#define IVPN_HTTP_UTILS_H


#pragma once
#include <QString>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QNetworkRequest>
QByteArray httpGet(const QString& url);


#endif //IVPN_HTTP_UTILS_H