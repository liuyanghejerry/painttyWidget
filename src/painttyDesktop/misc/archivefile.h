#ifndef ARCHIVEFILE_H
#define ARCHIVEFILE_H

#include <QObject>
class QFile;

class ArchiveFile : public QObject
{
    Q_OBJECT
public:
    explicit ArchiveFile(const QString &name,
                         const QString &signature,
                         QObject *parent = 0);
    ~ArchiveFile();
    quint64 size();
    
signals:
    
public slots:
    void appendData(const QByteArray&);
    void setSignature(const QString& sign);
    void prune();
    void remove();
    void reset(const QString& sign);
protected:
    QString signature_;
    QFile* backend_;
private:
    Q_DISABLE_COPY(ArchiveFile)
    
};

#endif // ARCHIVEFILE_H
