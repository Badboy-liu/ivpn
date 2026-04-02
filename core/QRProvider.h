//
// Created by zql on 2026/4/2.
//

#ifndef IVPN_QRPROVIDER_H
#define IVPN_QRPROVIDER_H


#pragma once
#include <QQuickImageProvider>
#include <QImage>
#include <QPainter>
#include <QUrl>

class QRProvider : public QQuickImageProvider {
public:
    QRProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

    QImage requestImage(const QString &id, QSize *, const QSize &) override {
        QString text = QUrl::fromPercentEncoding(id.toUtf8());

        // 简单画个假二维码（演示用）
        QImage img(200, 200, QImage::Format_RGB32);
        img.fill(Qt::white);

        QPainter p(&img);
        p.setBrush(Qt::black);

        // 简单画格子模拟二维码
        int size = text.length() % 20 + 10;
        int cell = 200 / size;

        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if ((i + j + text.length()) % 3 == 0) {
                    p.drawRect(i * cell, j * cell, cell, cell);
                }
            }
        }

        return img;
    }
};

#endif //IVPN_QRPROVIDER_H