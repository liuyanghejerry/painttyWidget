#include "PSDExport.h"
#include <QList>
#include <QDataStream>
#include <QDebug>

QByteArray imageToLayerData(const QList<QImage> &imageList);
QByteArray imageToChannelData(const QImage &image, int *alphaSize, int *redSize, int *greenSize, int *blueSize);
QByteArray RLECompress(const QByteArray &source);

#define BYTE_ORDER QDataStream::BigEndian

//File Header Section
#define SIGNATURE quint32(0x38425053) //"8BPS"
#define VERSION quint16(1)
#define RESERVED_P1 quint16(0)
#define RESERVED_P2 quint16(0)
#define RESERVED_P3 quint16(0) //total of 6 bytes of zero
#define IMAGE_CHANNEL_NUMBER quint16(4)
#define IMAGE_HEIGHT(h) quint32(h)
#define IMAGE_WIDTH(w) quint32(w)
#define DEPTH quint16(8)
#define COLOR_MODE quint16(3)

//Color Mode Data Section
#define COLOR_MODE_SIZE_AND_DATA quint32(0)

//Image Resources Section
#define IMAGE_RESOURCES_SIZE_AND_DATA quint32(0)

//Layer and Mask Information Section
#define LAYER_CHANNEL_COUNT quint16(4)
#define ALPHA_CHANNEL_ID qint16(-1)
#define RED_CHANNEL_ID qint16(0)
#define GREEN_CHANNEL_ID qint16(1)
#define BLUE_CHANNEL_ID qint16(2)
#define BLEND_MODE_SIGNATURE quint32(0x3842494D) //"8BIM"
#define BLEND_MODE_KEY quint32(0x6E6F726D) //"norm"
#define OPACITY quint8(255)
#define CLIPPING quint8(0)
#define FLAGS quint8(8)
#define FILTER quint8(0)

QByteArray imagesToPSD(const QList<QImage> &imageList)
{
    QByteArray psdData;
    QDataStream psdDataStream(&psdData, QIODevice::WriteOnly);
    psdDataStream << SIGNATURE << VERSION << RESERVED_P1 << RESERVED_P2 << RESERVED_P3;
    psdDataStream << IMAGE_CHANNEL_NUMBER;
    psdDataStream << IMAGE_HEIGHT(imageList.at(0).height())
                  << IMAGE_WIDTH(imageList.at(0).width());
    psdDataStream << DEPTH << COLOR_MODE;
    psdDataStream << COLOR_MODE_SIZE_AND_DATA << IMAGE_RESOURCES_SIZE_AND_DATA;

    QByteArray layerAndMaskInfoSection;
    QDataStream layerAndMaskInfoSectionStream(&layerAndMaskInfoSection, QIODevice::WriteOnly);
    layerAndMaskInfoSectionStream << quint32(0);
    layerAndMaskInfoSection.append(imageToLayerData(imageList));
    layerAndMaskInfoSectionStream.device()->seek(0);
    layerAndMaskInfoSectionStream << quint32(layerAndMaskInfoSection.size() - 4);
    psdData.append(layerAndMaskInfoSection);
    psdData.append(QByteArray(6, 0));
    return psdData;
}

QByteArray imageToLayerData(const QList<QImage> &imageList)
{
    QByteArray layerRecord;
    QByteArray channelImageData;
    QDataStream layerRecordStream(&layerRecord, QIODevice::WriteOnly);
    QDataStream channelImageDataStream(&channelImageData, QIODevice::WriteOnly);
    layerRecordStream.setByteOrder(BYTE_ORDER);
    channelImageDataStream.setByteOrder(BYTE_ORDER);

    foreach (const QImage &image, imageList)
    {
        int a, r, g, b;
        QByteArray channelData = imageToChannelData(image, &a, &r, &g, &b);
        layerRecordStream << quint32(0) << quint32(0) //top, left
                          << IMAGE_HEIGHT(image.height()) //bottom
                          << IMAGE_WIDTH(image.width()); //right
        layerRecordStream << LAYER_CHANNEL_COUNT;
//        layerRecordStream << ALPHA_CHANNEL_ID << quint32(channelData.size() / 4);
//        layerRecordStream << RED_CHANNEL_ID << quint32(channelData.size() / 4);
//        layerRecordStream << GREEN_CHANNEL_ID << quint32(channelData.size() / 4);
//        layerRecordStream << BLUE_CHANNEL_ID << quint32(channelData.size() / 4);
        layerRecordStream << ALPHA_CHANNEL_ID << quint32(a);
        layerRecordStream << RED_CHANNEL_ID << quint32(r);
        layerRecordStream << GREEN_CHANNEL_ID << quint32(g);
        layerRecordStream << BLUE_CHANNEL_ID << quint32(b);
        layerRecordStream << BLEND_MODE_SIGNATURE << BLEND_MODE_KEY;
        layerRecordStream << OPACITY << CLIPPING << FLAGS << FILTER
                          << quint32(4) << quint32(0); //extra data field to empty
        channelImageDataStream.writeRawData(channelData.constData(), channelData.size());
    }

    QByteArray layerInfo;
    QDataStream layerInfoStream(&layerInfo, QIODevice::WriteOnly);
    layerInfoStream << quint32(0); //Length of the layers info section, rounded up to a multiple of 2
    layerInfoStream << qint16(-imageList.count()); //Layer count
    layerInfoStream.writeRawData(layerRecord.constData(), layerRecord.size());
    layerInfoStream.writeRawData(channelImageData.constData(), channelImageData.size());
    if (layerInfo.size() & 1)
        layerInfoStream << quint8(0);
    layerInfoStream.device()->seek(0);
    layerInfoStream << quint32(layerInfo.size() - 4);
    return layerInfo;
}

QByteArray imageToChannelData(const QImage &image, int *alphaSize, int *redSize, int *greenSize, int *blueSize)
{
    if (image.format() != QImage::Format_ARGB32)
        return imageToChannelData(image.convertToFormat(QImage::Format_ARGB32),
                                  alphaSize, redSize, greenSize, blueSize);

    int pixelCount = image.width() * image.height();
    QByteArray alphaChannel;
    QByteArray redChannel;
    QByteArray greenChannel;
    QByteArray blueChannel;
    alphaChannel.reserve(pixelCount + 2);
    redChannel.reserve(pixelCount + 2);
    greenChannel.reserve(pixelCount + 2);
    blueChannel.reserve(pixelCount + 2);

//    alphaChannel.append(char(0));
//    alphaChannel.append(char(0));
//    redChannel.append(char(0));
//    redChannel.append(char(0));
//    greenChannel.append(char(0));
//    greenChannel.append(char(0));
//    blueChannel.append(char(0));
//    blueChannel.append(char(0));

    for (int line = 0; line < image.height(); line++)
    {
        const QRgb *lineData = (const QRgb*)image.constScanLine(line);
        for (int col = 0; col < image.width(); col++)
        {
            alphaChannel.append(qAlpha(lineData[col]));
            redChannel.append(qRed(lineData[col]));
            greenChannel.append(qGreen(lineData[col]));
            blueChannel.append(qBlue(lineData[col]));
        }
    }
    alphaChannel = qCompress(alphaChannel);
    alphaChannel.remove(0, 2);
    alphaChannel[0] = 0;
    alphaChannel[1] = 2;
    *alphaSize = alphaChannel.size();
    redChannel = qCompress(redChannel);
    redChannel.remove(0, 2);
    redChannel[0] = 0;
    redChannel[1] = 2;
    *redSize = redChannel.size();
    greenChannel = qCompress(greenChannel);
    greenChannel.remove(0, 2);
    greenChannel[0] = 0;
    greenChannel[1] = 2;
    *greenSize = greenChannel.size();
    blueChannel = qCompress(blueChannel);
    blueChannel.remove(0, 2);
    blueChannel[0] = 0;
    blueChannel[1] = 2;
    *blueSize = blueChannel.size();
    return alphaChannel + redChannel + greenChannel + blueChannel;
}

QByteArray RLECompress(const QByteArray &source)
{
    qint8 count = 1;
    quint8 lastChar = source.at(0);
    QByteArray dest;
    for(int i = 1; i < source.size(); i++)
    {
        if (source.at(i) == lastChar && count < 127)
        {
            count++;
        }
        else
        {
            //output same char
            dest.append(char(-count + 1)); //-(cout-1)
            dest.append(lastChar);
            lastChar = source.at(i);
            count = 1;
        }
    }
    //output same char
    dest.append(char(-count + 1)); //-(cout-1)
    dest.append(lastChar);
    return dest;
}
