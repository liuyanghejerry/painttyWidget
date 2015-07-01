#include "sponsorlabel.h"

#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSettings>

#include "../../common/common.h"

SponsorLabel::SponsorLabel(QWidget *parent) :
    QLabel(parent)
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished,
            [this](QNetworkReply* r) {
        if(r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() / 200 == 1) {
            this->setText(r->readAll());
        }
    });

    QSettings settings(GlobalDef::SETTINGS_NAME,
                       QSettings::defaultFormat());
    bool hide_sponser = settings.value("global/hide_sponser", false).toBool();
    if(hide_sponser) {
        return;
    }
    manager->get(QNetworkRequest(QUrl("http://mrspaint.com/sponsor.html")));
}
