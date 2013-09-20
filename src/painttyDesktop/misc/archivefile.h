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
    explicit ArchiveFile(QObject *parent = 0);
    ~ArchiveFile();
    QByteArray readAll() const;
    quint64 size() const;
    QString name() const;
    QString signature() const;
    QString dirName() const;
signals:
    
public slots:
    void setName(const QString &name);
    void appendData(const QByteArray&);
    void setSignature(const QString& sign);
    void flush();
    void prune();
    void remove();
protected:
    QString signature_;
    QString name_;
    QString dir_name_;
    QFile* backend_;
private:
    Q_DISABLE_COPY(ArchiveFile)
    bool createFile();
};

#endif // ARCHIVEFILE_H
