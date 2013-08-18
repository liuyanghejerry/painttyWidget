#ifndef ROUTER_H
#define ROUTER_H

#include <functional>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHash>

// name space for default handler type
namespace RouterZone{
typedef std::function<void (const QJsonObject&)> DefaultHandler;
}


template<typename RouterFunc_ = RouterZone::DefaultHandler>
class Router
{
public:
    void addRule(const QString &rule)
    {
        records_.insert(rule, FuncTable());
    }

    void removeRule(const QString &rule)
    {
        records_.remove(rule);
    }

    void regHandler(const QString &rule,
                 const QString &request,
                 RouterFunc_ func)
    {
        if(!records_.contains(rule)){
            addRule(rule);
        }
        records_[rule].insert(request, func);
    }

    void unregHandler(const QString &rule,
                   const QString &request)
    {
        if(records_.contains(rule)){
            records_[rule].remove(request);
        }
    }

    void clear()
    {
        records_.clear();
    }

    void onData(const QJsonObject &obj)
    {
        auto rules = records_.keys();
        for(auto item: rules){
            if(obj.contains(item)){
                auto request = obj.value(item).toString();
                if(records_[item].contains(request)){
                    auto func = records_[item][request];
                    func(obj);
                    break;
                }
            }
        }
    }
private:
    typedef QHash<QString, RouterFunc_> FuncTable;
    QHash<QString, FuncTable > records_;
};

#endif // ROUTER_H
