#ifndef ERRORTABLE_H
#define ERRORTABLE_H

#include <QObject>
#include <QString>
#include <unordered_map>

class ErrorTable
{
public:
    static QString toString(int errcode);
private:
    ErrorTable() = delete;
    ~ErrorTable() = delete;
    ErrorTable(const ErrorTable&) = delete;
    ErrorTable & operator=(const ErrorTable&) = delete;
    const static std::unordered_map<int, QString> errMsg_;
};

#endif // ERRORTABLE_H
