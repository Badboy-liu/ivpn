//
// Created by zql on 2026/4/2.
//

#include "QRCodeGen.h"

#include <iostream>
#include <ZXing/MultiFormatWriter.h>
#include <ZXing/BitMatrix.h>

using namespace ZXing;

QImage QRCodeGen::generate(const QString& text)
{
    MultiFormatWriter writer(BarcodeFormat::QRCode);
    auto matrix = writer.encode(text.toStdString(), 256, 256);

    QImage img(256, 256, QImage::Format_RGB888);
    img.fill(Qt::white);

    for (int y = 0; y < matrix.height(); y++) {
        for (int x = 0; x < matrix.width(); x++) {
            if (matrix.get(x, y))
                img.setPixel(x, y, qRgb(0, 0, 0));
        }
    }

    return img;
}

#include "QRCodeGen.h"

#include <QImage>
#include <QBuffer>
using namespace ZXing;

QString QRCodeGen::generateBase64(const QString& text)
{
    std::string utf8Text = text.toUtf8().toStdString();
    MultiFormatWriter writer(BarcodeFormat::QRCode);
    BitMatrix matrix;
    try {
        matrix = writer.encode(utf8Text, 256, 256);
    }catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }


    QImage img(256, 256, QImage::Format_RGB888);
    img.fill(Qt::white);

    for (int y = 0; y < matrix.height(); y++) {
        for (int x = 0; x < matrix.width(); x++) {
            if (matrix.get(x, y))
                img.setPixel(x, y, qRgb(0, 0, 0));
        }
    }

    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "PNG");

    return "data:image/png;base64," + bytes.toBase64();
}