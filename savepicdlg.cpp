#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPushButton>
#include <QTimer>

#include "savepicdlg.h"

SavePicDlg::SavePicDlg(const QString &pic, QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f) {
    setupUi(this);

    buttonBox->button(QDialogButtonBox::Close)->setDefault(false);
    buttonBox->button(QDialogButtonBox::Close)->setAutoDefault(false);
    buttonBox->button(QDialogButtonBox::Save)->setDisabled(true);
    buttonBox->button(QDialogButtonBox::Save)->setDefault(true);
    netman = new QNetworkAccessManager(this);
    connect(netman, SIGNAL(finished(QNetworkReply*)), this, SLOT(gotReply(QNetworkReply*)));

    imgName = pic;
    times = 0;
    loadPic();
}

void SavePicDlg::loadPic() {
    netman->get(QNetworkRequest(QUrl("http://i.imgur.com/" + imgName + ".jpg")));
}


void SavePicDlg::gotReply(QNetworkReply *reply) {
    reply->deleteLater();
    if(reply->error() != QNetworkReply::NoError) {
	if(times++ < 3)
	    QTimer::singleShot(500 + (qrand() & 511) * times, this, SLOT(loadPic()));
	else
	    labelPic->setText("Failed to download image: " + reply->errorString());
	return;
    }

    QByteArray imgdata = reply->readAll();
    if(!img.loadFromData(imgdata)) {
	if(times++ < 3)
	    QTimer::singleShot(500 + (qrand() & 511) * times, this, SLOT(loadPic()));
	else
	    labelPic->setText("Failed to load image");
	return;
    }

    QPixmap px = QPixmap::fromImage(img);
    int y = px.size().height(), x = px.size().width();
    int maxy = labelPic->maximumHeight(), maxx = labelPic->maximumWidth();
    if(y > maxy || x > maxx) {
	if(x / maxx > y / maxy)
	    px = px.scaledToWidth(maxx, Qt::SmoothTransformation);
	else
	    px = px.scaledToHeight(maxy, Qt::SmoothTransformation);
    }
    labelPic->setPixmap(px);
    buttonBox->button(QDialogButtonBox::Save)->setDisabled(false);
}

void SavePicDlg::accept() {
    img.save(imgName + ".jpg");
    QDialog::accept();
}

