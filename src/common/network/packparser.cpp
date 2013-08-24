#include <QCoreApplication>
#include <QDebug>
#include "packparser.h"

PackParser::PackParser(QObject *parent) :
    QObject(parent),
    pack_size(0),
    last_pack_unfinished(false)
{
}


void PackParser::onRawData(const QByteArray &rawbytes)
{
    raw_buffer_.open(QBuffer::ReadWrite);
    raw_buffer_.write(rawbytes);
    processBuffer();
}

// take binary data and pack it to a TCP pack
QByteArray PackParser::packRaw(const QByteArray &content)
{
    auto result = content;
    quint32 length = content.length();
    uchar c1, c2, c3, c4;
    c1 = length & 0xFF;
    length >>= 8;
    c2 = length & 0xFF;
    length >>= 8;
    c3 = length & 0xFF;
    length >>= 8;
    c4 = length & 0xFF;

    result.prepend(c1);
    result.prepend(c2);
    result.prepend(c3);
    result.prepend(c4);

    return result;
}

// unused
//QByteArray PackParser::unpackRaw(const QByteArray &content)
//{
//    //
//}

void PackParser::processBuffer()
{
    if(last_pack_unfinished){
        if(raw_buffer_.bytesAvailable() >= pack_size){
            QByteArray info = raw_buffer_.read(pack_size);
            last_pack_unfinished = false;
            emit newRawPack(info);
        }else{
            return;
        }
    }

    if(raw_buffer_.size() < 4){
        return;
    }

    // in case we have a lot of data to process
    QCoreApplication::processEvents();
    char c1, c2, c3, c4;
    raw_buffer_.getChar(&c1);
    raw_buffer_.getChar(&c2);
    raw_buffer_.getChar(&c3);
    raw_buffer_.getChar(&c4);
    pack_size = (uchar(c1) << 24) + (uchar(c2) << 16)
            + (uchar(c3) << 8) + uchar(c4);

    processBuffer();
}

void PackParser::onRawPack(const QByteArray &rawpack)
{
    bool isCompressed = rawpack[0] & 0x1;
    PACK_TYPE pack_type = PACK_TYPE((rawpack[0] & binL<110>::value) >> 0x1);
    QByteArray data_without_header = rawpack.right(rawpack.length()-1);
    if(isCompressed){
        QByteArray tmp = qUncompress(data_without_header);
        if(tmp.isEmpty()){
            qWarning()<<"bad input"<<data_without_header.toHex();
            return;
        }
        emit newPack(ParserResult(pack_type, tmp));
    }else{
        emit newPack(ParserResult(pack_type, data_without_header));
    }
}

QByteArray PackParser::assamblePack(bool compress,
                                    PACK_TYPE pt,
                                    const QByteArray& bytes)
{
    auto body = bytes;
    if(compress){
        body = qCompress(bytes);
    }
    QByteArray result;
    char header = (compress & 0x1) | (pt << 0x1);
    result.append(header);
    result.append(body);
    return result;
}
