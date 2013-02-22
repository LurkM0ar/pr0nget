#ifndef __PICWIDGET_H__
#define __PICWIDGET_H__

#include <QImage>
#include "ui_picwidget.h"

class QNetworkAccessManager;
class QNetworkReply;

class Pr0nGet : public QObject {
    Q_OBJECT

public:
    Pr0nGet(QObject *parent = 0);
    QPair<QString, QPair<QImage, int> > getPr0n();

private:
    QList<QPair<QString, QPair<QImage, int> > > sets;
    QNetworkAccessManager *netman;
    enum  { PR0N_IDLE, PR0N_HEAD, PR0N_IMG, PR0N_PAGE } pr0nPhase;
    QString curname;
    QImage curimg;
    static const int maxSize = 10;
    int pageGets;

    void tryAgain();
    void refill();

private slots:
    void gotReply(QNetworkReply *);
    void getViews();

signals:
    void imagesAvailable(int);
    void attempting();
    void guessing();
};

class PicWidget : public QWidget, private Ui::PicWidget {
    Q_OBJECT

public:
    PicWidget(QWidget * parent = 0);
    static const unsigned int numMetrics = 31;

protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    Pr0nGet stash;
    QString old_name;
    QString old_views;
    QPixmap old_pix;

public slots:
    void newImage();
    void oldImage();

private slots:
    void handleStashChange(int backlog);

signals:
    void imageUpdated();
    void attemptEvt();
    void guessEvt();
    void pr0nEvt();
    void saveEvt();
};

#endif
