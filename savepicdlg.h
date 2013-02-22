#ifndef __SAVEPICDLG_H__
#define __SAVEPICDLG_H__

#include <QImage>

#include "ui_savepicdlg.h"

class QNetworkAccessManager;
class QNetworkReply;

class SavePicDlg : public QDialog, private Ui::SavePicDlg {
    Q_OBJECT

public:
    SavePicDlg(const QString &pic, QWidget *parent = 0, Qt::WindowFlags f = 0);

private:
    QNetworkAccessManager *netman;
    QString imgName;
    QImage img;
    int times;

private slots:
    void gotReply(QNetworkReply *);
    void loadPic();
    void accept();
};

#endif
