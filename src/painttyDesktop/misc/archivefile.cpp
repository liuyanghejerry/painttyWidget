#include "archivefile.h"
#include "../../common/common.h"
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QCryptographicHash>
#include <QSettings>
#include <QDebug>

ArchiveFile::ArchiveFile(const QString& name,
                         const QString& signature,
                         QObject *parent) :
    QObject(parent),
    signature_(signature),
    backend_(nullptr)
{
    if(!name.isEmpty())
        setName(name);
}

ArchiveFile::ArchiveFile(QObject *parent) :
    ArchiveFile(QString(), QString(), parent)
{
}

ArchiveFile::~ArchiveFile()
{
    if(backend_ && backend_->isOpen()){
        backend_->flush();
        backend_->waitForBytesWritten(3000);
        backend_->close();
    }
}

QByteArray ArchiveFile::readAll() const
{
    if(!backend_)
        return QByteArray();
    backend_->flush();
    backend_->seek(0);
    return backend_->readAll();
}

void ArchiveFile::appendData(const QByteArray &data)
{
    if(!backend_)
        return;
    backend_->seek(backend_->size());
    backend_->write(data);
}

void ArchiveFile::setSignature(const QString& sign)
{
    qDebug()<<"old sign"<<signature_<<"new"<<sign;
    if(!signature_.isEmpty() && sign != signature_)
        prune();
    signature_ = sign;

    QCryptographicHash crypto(QCryptographicHash::Sha1);
    crypto.addData(name_.toUtf8());
    auto&& hash = crypto.result().toHex();

    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    settings.setValue("archives/"+hash, signature_);
    settings.sync();
}

void ArchiveFile::flush()
{
    if(!backend_)
        return;
    backend_->flush();
}

void ArchiveFile::prune()
{
    if(!backend_)
        return;
    qDebug()<<"File pruned";
    backend_->resize(0);
    backend_->seek(0);
}

void ArchiveFile::remove()
{
    if(!backend_)
        return;
    backend_->close();
    backend_->remove();
}

quint64 ArchiveFile::size() const
{
    if(!backend_)
        return 0;
    return backend_->size();
}

QString ArchiveFile::name() const
{
    return name_;
}

void ArchiveFile::setName(const QString &name)
{
    if(name.isEmpty() || name_ == name)
        return;
    name_ = name;
    createFile();
}

QString ArchiveFile::signature() const
{
    return signature_;
}

bool ArchiveFile::createFile()
{
    auto isGood = QDir::current().mkpath("cache");
    if(!isGood){
        qWarning()<<"Cannot create path: cache";
        return isGood;
    }
    QCryptographicHash crypto(QCryptographicHash::Sha1);
    crypto.addData(name_.toUtf8());
    auto hash = crypto.result().toHex();
    QString filename = QString("%1%2")
            .arg("cache/")
            .arg(QString::fromUtf8(hash));

    if(backend_){
        if(backend_->isOpen())
            backend_->close();
        backend_->deleteLater();
        backend_ = nullptr;
    }

    backend_ = new QFile(filename, this);
    isGood = backend_->open(QIODevice::ReadWrite);
    if(!isGood){
        qWarning()<<"Cannot open archive file:"<<filename;
    }

    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat(),
                       qApp);
    if(settings.contains("archives/"+hash)){
        auto&& saved_sign = settings.value("archives/"+hash).toString();
        signature_ = saved_sign;
    }

    return isGood;
}
