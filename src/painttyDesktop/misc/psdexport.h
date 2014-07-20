#ifndef PSDEXPORT_H
#define PSDEXPORT_H

#include <QByteArray>
#include <QImage>

QByteArray imagesToPSD(const QList<QImage> &imageList, const QImage &preview);

#endif // PSDEXPORT_H
