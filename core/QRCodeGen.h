//
// Created by zql on 2026/4/2.
//

#ifndef IVPN_QRCODEGEN_H
#define IVPN_QRCODEGEN_H


#pragma once
#include <QImage>
#include <QString>

class QRCodeGen
{
public:
    static QImage generate(const QString& text);
    static QString generateBase64(const QString& text);

};


#endif //IVPN_QRCODEGEN_H