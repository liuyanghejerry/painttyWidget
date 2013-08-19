#include "archivefile.h"
#include <QFile>
#include <QDir>
#include <QCryptographicHash>
#include <QDebug>

ArchiveFile::ArchiveFile(const QString& name,
                         const QString& signature,
                         QObject *parent) :
    QObject(parent),
    signature_(signature)
{
    auto isGood = QDir::current().mkpath("cache");
    if(!isGood){
        qWarning()<<"Cannot create path: cache";
    }
    QCryptographicHash crypto(QCryptographicHash::Sha1);
    crypto.addData(name.toUtf8());
    QString filename = QString("%1%2")
            .arg("cache/")
            .arg(QString::fromUtf8(crypto.result().toHex()));
    backend_ = new QFile(filename, this);
    isGood = backend_->open(QIODevice::ReadWrite|QIODevice::Append);
    if(!isGood){
        qWarning()<<"Cannot open archive file:"<<filename;
    }
    // TODO: try to render content if not empty
}

ArchiveFile::~ArchiveFile()
{
    if(backend_->isOpen()){
        backend_->waitForBytesWritten(3000);
        backend_->close();
    }
}

void ArchiveFile::appendData(const QByteArray &data)
{
    backend_->write(data);
}

void ArchiveFile::setSignature(const QString& sign)
{
    signature_ = sign;
}

void ArchiveFile::reset(const QString& sign)
{
    prune();
    setSignature(sign);
}

void ArchiveFile::prune()
{
    backend_->resize(0);
}

void ArchiveFile::remove()
{
    backend_->close();
    backend_->remove();
}

quint64 ArchiveFile::size()
{
    return backend_->size();
}
