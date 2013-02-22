#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QApplication>
#include <QMessageBox>
#include <QShortcut>
#include <QRegExp>
#include <QTimer>

#include <QtCore/qmath.h>
#include <QMouseEvent>

#include "qlabcolor.h"
#include "picwidget.h"
#include "savepicdlg.h"

#include <QDebug>

#define stds	metrics[0]
#define means	metrics[1]
#define mins	metrics[2]
#define maxs	metrics[3]
#define stdv	metrics[4]
#define meanv	metrics[5]
#define minv	metrics[6]
#define maxv	metrics[7]
#define stdr	metrics[8]
#define meanr	metrics[9]
#define minr	metrics[10]
#define maxr	metrics[11]
#define stdg	metrics[12]
#define meang	metrics[13]
#define ming	metrics[14]
#define maxg	metrics[15]
#define stdb	metrics[16]
#define meanb	metrics[17]
#define minb	metrics[18]
#define maxb	metrics[19]
#define stde	metrics[20]
#define meane	metrics[21]
#define mine	metrics[22]
#define maxe	metrics[23]
#define stdq	metrics[24]
#define meanq	metrics[25]
#define rr	metrics[26]
#define rg	metrics[27]
#define rb	metrics[28]
#define rk	metrics[29]
#define rw	metrics[30]

PicWidget::PicWidget(QWidget *parent) : QWidget(parent) {
    setupUi(this);
    connect(&stash, SIGNAL(imagesAvailable(int)), this, SLOT(handleStashChange(int)));
    connect(&stash, SIGNAL(attempting()), this, SIGNAL(attemptEvt()));
    connect(&stash, SIGNAL(guessing()), this, SIGNAL(guessEvt()));
    labelStat->setText("Searching ...");
}

void PicWidget::handleStashChange(int backlog) {
    if(!labelPic->pixmap() || labelPic->pixmap()->isNull())
	newImage();
    emit pr0nEvt();
}

void PicWidget::newImage() {
    QPair<QString, QPair<QImage, int> > p = stash.getPr0n();
    if(p.first.isNull())
	return;

    old_pix = labelPic->pixmap() ? *(labelPic->pixmap()) : QPixmap();
    old_name = lineEditName->text();
    old_views = labelStat->text();

    lineEditName->setText(p.first);

    int views = p.second.second;
    if(views < 0)
	labelStat->setText("Pic views: N/A");
    else if(views < 50)
	labelStat->setText("Pic views: <span style=\"color:#3a3;\">" + QString::number(views) + "</span>");
    else if(views > 2000)
	labelStat->setText("Pic views: <span style=\"color:#a33;\">" + QString::number(views) + "</span>");
    else
	labelStat->setText("Pic views: <span style=\"color:#ee4;\">" + QString::number(views) + "</span>");

    QPixmap pix = QPixmap::fromImage(p.second.first);
    labelPic->setPixmap(pix);

    emit imageUpdated();
}

void PicWidget::mouseReleaseEvent(QMouseEvent *event) {
    if(event->button() == Qt::LeftButton) {
	// dblclick only
    } else if(event->button() == Qt::RightButton) {
	oldImage();
    } else if(event->button() == Qt::MiddleButton) {
	QDesktopServices::openUrl(QUrl("http://i.imgur.com/" + lineEditName->text()));
    }
}

void PicWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    if(event->button() == Qt::LeftButton) {
	SavePicDlg s(lineEditName->text());
	if(s.exec() == QDialog::Accepted)
	    emit saveEvt();
    }
}

void PicWidget::oldImage() {
    if(old_name.isEmpty() || old_pix.isNull())
	return;

    QString oldold_name = lineEditName->text();
    QString oldold_views = labelStat->text();
    lineEditName->setText(old_name);
    labelStat->setText(old_views);
    QPixmap oldold_pix = labelPic->pixmap() ? *(labelPic->pixmap()) : QPixmap();
    labelPic->setPixmap(old_pix);
    old_pix = oldold_pix;
    old_name = oldold_name;
    old_views = oldold_views;
}


Pr0nGet::Pr0nGet(QObject *parent) : QObject(parent) {
    pr0nPhase = PR0N_IDLE;
    netman = new QNetworkAccessManager(this);
    connect(netman, SIGNAL(finished(QNetworkReply*)), this, SLOT(gotReply(QNetworkReply*)));
    refill();
}

void Pr0nGet::refill() {
    if(pr0nPhase != PR0N_IDLE || sets.size() >= maxSize)
	return;
    tryAgain();
}

void Pr0nGet::tryAgain() {
    const QString chars="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    QString s;
    pr0nPhase = PR0N_HEAD;
    pageGets = 0;
    s.resize(5);
    for(int i=0; i<5; i++)
	s[i] = chars.at(qrand() % chars.size());
    curname = s;
    netman->head(QNetworkRequest(QUrl("http://i.imgur.com/" + s + ".jpg")));
    emit attempting();
}

void Pr0nGet::getViews() {
    pr0nPhase = PR0N_PAGE;
    netman->get(QNetworkRequest(QUrl("http://imgur.com/" + curname)));
}

void Pr0nGet::gotReply(QNetworkReply *reply) {
    reply->deleteLater();

    if(pr0nPhase == PR0N_HEAD) {
	if(reply->error() != QNetworkReply::NoError) {
	    tryAgain();
	    return;
	}
	if(reply->operation() != QNetworkAccessManager::HeadOperation ||
           (reply->hasRawHeader("Content-Length") && reply->rawHeader("Content-Length") == QByteArray("503")) ||
	   (reply->header(QNetworkRequest::ContentTypeHeader) != "image/jpeg" && reply->header(QNetworkRequest::ContentTypeHeader) != "image/png"))
	    tryAgain();
	else {
	    pr0nPhase = PR0N_IMG;
	    netman->get(QNetworkRequest(QUrl("http://i.imgur.com/" + curname + "s.jpg")));
	}
	return;
    }

    if(pr0nPhase == PR0N_PAGE) {
	QRegExp re("views\">([,0-9]+)</span> view");
	int views;

	if(reply->error() == QNetworkReply::NoError && 
	   reply->header(QNetworkRequest::ContentTypeHeader) == "text/html" &&
	   re.indexIn(reply->readAll())>=0) {
	    views = re.cap(1).remove(',').toInt();
	    if(views > 10000) {
		tryAgain();
		return;
	    }
	} else if(pageGets++<3) {
	    QTimer::singleShot(500 + (qrand() & 511) * pageGets, this, SLOT(getViews()));
	    return;
	} else
	    views = -1;

	pr0nPhase = PR0N_IDLE;
	sets << qMakePair(curname, qMakePair(curimg, views));
	emit imagesAvailable(sets.count());
	refill();
	return;
    }

    if(pr0nPhase != PR0N_IMG) {
	tryAgain();
	return;
    }

    QByteArray imgdata = reply->readAll();
    QImage img;
    qreal metrics[31];
    if(!img.loadFromData(imgdata) || img.size().width() != 90 || img.size().height() != 90) {
	tryAgain();
	return;
    }

    emit guessing();

    unsigned int n = 0;
    qreal h;
    qreal s, deltas, m2s = 0;
    qreal v, deltav, m2v = 0;
    qreal r, deltar, m2r = 0;
    qreal g, deltag, m2g = 0;
    qreal b, deltab, m2b = 0;
    qreal e, deltae, m2e = 0;
    int red = 0, green = 0, blue = 0, black = 0, white = 0;

    means = meanv = meanr = meang = meanb = meane = 0;
    maxs = maxv = maxr = maxg = maxb = maxe = 0;
    mins = minv = minr = ming = minb = mine = 100000;

    qreal sobel[88][88];
    for(int x=0; x<90; x++) {
    	for(int y=0; y<90; y++) {
	    QColor px = img.pixel(x, y);
    	    px.getHslF(&h, &s, &v);
    	    px.getRgbF(&r, &g, &b);
    	    n++;

    	    mins = qMin(mins, s);
    	    minv = qMin(minv, v);
    	    minr = qMin(minr, r);
    	    ming = qMin(ming, g);
    	    minb = qMin(minb, b);
    	    maxs = qMax(maxs, s);
    	    maxv = qMax(maxv, v);
    	    maxr = qMax(maxr, r);
    	    maxg = qMax(maxg, g);
    	    maxb = qMax(maxb, b);

    	    if(v < 0.3)
    		black += 1;
    	    else if(s < 0.25)
    		white++;
    	    else if(h < 0.16)
		red++;
	    else if(h < 0.5)
		green++;
	    else if(h < 0.84)
		blue++;
	    else
		red++;

    	    deltas = s - means; means += deltas/(qreal)n; m2s += deltas * (s - means);
    	    deltav = v - meanv; meanv += deltav/(qreal)n; m2v += deltav * (v - meanv);
    	    deltar = r - meanr; meanr += deltar/(qreal)n; m2r += deltar * (r - meanr);
    	    deltag = g - meang; meang += deltag/(qreal)n; m2g += deltag * (g - meang);
    	    deltab = b - meanb; meanb += deltab/(qreal)n; m2b += deltab * (b - meanb);

    	    if(x>0 && x<89 && y>0 && y<89) {
    		qreal gx = 0, gy = 0;
		gx += QLabColor(img.pixel(x-1, y-1))-QLabColor(img.pixel(x+1, y-1));
		gx += 2* (QLabColor(img.pixel(x-1, y))-QLabColor(img.pixel(x+1, y)));
		gx += QLabColor(img.pixel(x-1, y+1))-QLabColor(img.pixel(x+1, y+1));

		gy += QLabColor(img.pixel(x-1, y-1))-QLabColor(img.pixel(x-1, y+1));
		gy += 2* (QLabColor(img.pixel(x, y-1))-QLabColor(img.pixel(x, y+1)));
		gy += QLabColor(img.pixel(x+1, y-1))-QLabColor(img.pixel(x+1, y+1));

		e = (gx+gy)/8.0/200;
		e = qMin(e, 1.0);
    		sobel[x-1][y-1] = e;
    		mine = qMin(mine, e);
    		maxe = qMax(maxe, e);
    		deltae = e - meane; meane += deltae/(qreal)n; m2e += deltae * (e - meane);
    	    }
    	}
    }
    stds = m2s/(qreal)n;
    stdv = m2v/(qreal)n;

    stdr = m2r/(qreal)n;
    stdg = m2g/(qreal)n;
    stdb = m2b/(qreal)n;
    stde = m2e/(qreal)(n-89*4);

    rr = (qreal)red / (qreal)n;
    rg = (qreal)green / (qreal)n;
    rb = (qreal)blue / (qreal)n;
    rk = (qreal)black / (qreal)n;
    rw = (qreal)white / (qreal)n;

    qreal q, deltaq, m2q = 0;
    meanq = 0;
    for(int x=0; x<88; x++) {
    	for(int y=0; y<88; y++) {
    	    q = (sobel[x][y] - mine) / (maxe-mine);
    	    deltaq = q - meanq; meanq += deltaq/(qreal)n; m2q += deltaq * (q - meanq);
    	}
    }
    stdq = m2q/(qreal)(88*88);

    int match = 0;

    match += (metrics[0x13] * 0.288584547124842 < metrics[0x0d] && metrics[0x11] * 0.945816284838141 < metrics[0x0d] && metrics[0x17] * 0.242651798439745 > metrics[0x0c] && metrics[0x11] < 0.673337540592417 && metrics[0x1a] * 0.214235338601252 > metrics[0x10] && metrics[0x19] * 0.0650414573500413 > metrics[0x14] && metrics[0x1b] * 0.666998482898052 < metrics[0x10] && metrics[0x1d] * 0.204368132905508 > metrics[0x16] && metrics[0x1d] * 0.722960477632277 < metrics[0x1a] && metrics[0x0b] * 0.122404788204943 > metrics[0x08] && metrics[0x12] * 0.190223693037652 < metrics[0x10] && metrics[0x06] < 0.0994488884887055 && metrics[0x19] * 0.135723077334749 < metrics[0x04] && metrics[0x16] * 0.485412924242215 < metrics[0x14] && metrics[0x00] < 0.112097626224248 && metrics[0x15] < 0.0748170319550532);

    match += (metrics[0x12] * 0.820644076117588 < metrics[0x04] && metrics[0x13] * 0.0856123762321097 > metrics[0x15] && metrics[0x07] * 0.695487814288885 > metrics[0x0d] && metrics[0x1a] * 0.231524362936227 > metrics[0x0a] && metrics[0x09] * 0.947794459674942 > metrics[0x1d] && metrics[0x11] < 0.606052533385441 && metrics[0x1a] * 0.210325287279719 > metrics[0x04] && metrics[0x01] * 0.27866410753062 > metrics[0x1b] && metrics[0x08] < 0.104992962046779 && metrics[0x10] * 0.481843775798666 < metrics[0x0c] && metrics[0x11] * 0.26573705791008 > metrics[0x00] && metrics[0x15] * 0.0834045251277225 > metrics[0x14] && metrics[0x17] * 0.478001762666352 > metrics[0x19] && metrics[0x1c] < 0.150886937714393 && metrics[0x0d] * 0.0125805264925418 > metrics[0x14] && metrics[0x18] * 0.389846724779559 < metrics[0x04]);

    match += (metrics[0x1c] < 0.182361391480622 && metrics[0x09] * 0.918553768414867 > metrics[0x0d] && metrics[0x11] * 0.31270123343954 > metrics[0x18] && metrics[0x0d] * 0.0251198158213519 < metrics[0x04] && metrics[0x08] * 0.132361055380255 > metrics[0x16] && metrics[0x13] * 0.47557798550022 < metrics[0x09] && metrics[0x19] < 0.144421851394021 && metrics[0x0a] < 0.193862795703886 && metrics[0x1a] * 0.64275909631111 > metrics[0x15] && metrics[0x17] < 0.421205794150211 && metrics[0x04] < 0.0831780220590872 && metrics[0x11] < 0.675556769319279 && metrics[0x03] * 0.074931038396322 > metrics[0x0e] && metrics[0x17] * 0.341492760806108 > metrics[0x08] && metrics[0x01] * 0.259928914911796 > metrics[0x00] && metrics[0x18] < 0.0524398339191379);

    match += (metrics[0x0d] < 0.691572227017875 && metrics[0x1a] * 0.215213756365202 > metrics[0x10] && metrics[0x01] * 0.267282522905706 > metrics[0x15] && metrics[0x17] < 0.622199061112365 && metrics[0x19] * 0.963436417027182 > metrics[0x00] && metrics[0x11] * 0.94539805598502 < metrics[0x0d] && metrics[0x1d] < 0.479752221725882 && metrics[0x18] * 0.176803285290742 > metrics[0x16] && metrics[0x10] * 0.75930186786541 > metrics[0x1b] && metrics[0x18] < 0.0524950127332495 && metrics[0x09] * 0.550761414293433 < metrics[0x0d] && metrics[0x0c] < 0.0885262668477225 && metrics[0x15] * 0.288598468755424 < metrics[0x04] && metrics[0x06] < 0.116247885638064 && metrics[0x0b] * 0.0869277896535969 > metrics[0x15] && metrics[0x0d] * 0.229911262314236 > metrics[0x15]);

    match += (metrics[0x16] * 0.550378509853587 < metrics[0x14] && metrics[0x00] * 0.341144626967903 < metrics[0x10] && metrics[0x15] * 0.372299090025077 < metrics[0x0c] && metrics[0x1d] * 0.723619540134862 < metrics[0x1a] && metrics[0x11] < 0.589591262315295 && metrics[0x0d] * 0.151329584005243 > metrics[0x18] && metrics[0x15] * 0.08297638282394 > metrics[0x14] && metrics[0x0e] * 0.415394907526974 < metrics[0x00] && metrics[0x1c] < 0.179399584642042 && metrics[0x1a] * 0.202988897396068 > metrics[0x06] && metrics[0x00] * 0.766696694858613 < metrics[0x19] && metrics[0x19] < 0.145932445764423 && metrics[0x0d] < 0.739653411292455 && metrics[0x15] < 0.0832058591490359 && metrics[0x1a] * 0.231045665744467 > metrics[0x00] && metrics[0x08] < 0.104880635353524);

    match += (metrics[0x18] * 0.161825920199345 > metrics[0x16] && metrics[0x0b] * 0.60527406880172 > metrics[0x11] && metrics[0x17] * 0.465976267102771 > metrics[0x19] && metrics[0x00] * 0.39959592886418 < metrics[0x0c] && metrics[0x09] > 0.426358697780479 && metrics[0x03] * 0.0563411187762455 > metrics[0x12] && metrics[0x00] * 0.961004803887878 < metrics[0x19] && metrics[0x0c] < 0.0879634059693934 && metrics[0x11] * 0.583711715860158 > metrics[0x0a] && metrics[0x0d] < 0.693766305983893 && metrics[0x1a] * 0.212523601519468 > metrics[0x04] && metrics[0x0b] * 0.150406570312796 > metrics[0x1b] && metrics[0x17] * 0.637447506482268 < metrics[0x1a] && metrics[0x15] < 0.0761307034815815 && metrics[0x15] * 0.267364645297473 < metrics[0x04] && metrics[0x19] * 0.484340365748071 > metrics[0x18]);

    match += (metrics[0x15] < 0.0775212118678397 && metrics[0x1a] * 0.233977272419878 > metrics[0x00] && metrics[0x09] > 0.413158943548712 && metrics[0x0c] < 0.0917598063926519 && metrics[0x05] > 0.311474674974388 && metrics[0x18] * 0.491714340289668 < metrics[0x1d] && metrics[0x17] * 0.032228771602945 < metrics[0x04] && metrics[0x18] * 0.247204970872005 < metrics[0x10] && metrics[0x18] < 0.054800914194427 && metrics[0x1b] * 0.544874335316049 < metrics[0x10] && metrics[0x11] * 0.949957245506489 < metrics[0x0d] && metrics[0x17] * 0.241654624901241 > metrics[0x04] && metrics[0x09] * 0.886974889889306 > metrics[0x0d] && metrics[0x19] * 0.486183215662756 > metrics[0x18] && metrics[0x08] < 0.110470211783561 && metrics[0x06] < 0.117604452080904);

    match += (metrics[0x05] * 0.998486819360441 > metrics[0x0d] && metrics[0x1a] * 0.237475892755608 > metrics[0x00] && metrics[0x18] < 0.0524325505720853 && metrics[0x00] * 0.773268867408479 < metrics[0x19] && metrics[0x07] * 0.0830430085281506 > metrics[0x04] && metrics[0x1d] < 0.545221059008249 && metrics[0x0c] * 0.158916651491406 > metrics[0x14] && metrics[0x0a] < 0.22015678864981 && metrics[0x1a] * 0.429081765921033 < metrics[0x0d] && metrics[0x1c] < 0.18509626114491 && metrics[0x19] * 0.556317282858899 > metrics[0x12] && metrics[0x11] < 0.641457884364748 && metrics[0x14] < 0.00650386272213055 && metrics[0x01] * 0.282852813444549 > metrics[0x08] && metrics[0x16] * 0.52880588884511 < metrics[0x14] && metrics[0x15] < 0.0858048533593845);

    match /= 3;

    match += (metrics[0x0b] * 0.0856601821953618 > metrics[0x15] && metrics[0x0a] < 0.112319840179353 && metrics[0x1a] * 0.231112597163698 > metrics[0x00] && metrics[0x17] * 0.0376869306524128 < metrics[0x04] && metrics[0x18] < 0.054495902932203 && metrics[0x01] * 0.629756832250841 > metrics[0x19] && metrics[0x08] * 0.522203498150212 < metrics[0x19] && metrics[0x14] < 0.00519464319792462 && metrics[0x04] < 0.0661007759537959 && metrics[0x09] * 0.911486059377836 > metrics[0x0d] && metrics[0x0d] * 0.431552360133729 > metrics[0x19] && metrics[0x0f] * 0.483641596275881 < metrics[0x09] && metrics[0x0d] < 0.659539902054981 && metrics[0x09] * 0.138551073788904 > metrics[0x0c] && metrics[0x17] > 0.185699761947632 && metrics[0x1c] < 0.149582152817125);

    match += (metrics[0x15] < 0.102950805811496 && metrics[0x0d] * 0.95130473314666 < metrics[0x05] && metrics[0x18] < 0.0507537842378873 && metrics[0x19] * 0.284031230331301 > metrics[0x12] && metrics[0x13] * 0.319604676008812 < metrics[0x0d] && metrics[0x08] * 0.767003224005403 < metrics[0x1d] && metrics[0x01] * 0.251255492495005 > metrics[0x15] && metrics[0x16] * 0.579483309365962 < metrics[0x14] && metrics[0x00] * 0.895991068481749 < metrics[0x19] && metrics[0x0d] < 0.684614774924352 && metrics[0x17] * 0.816104989307068 < metrics[0x1a] && metrics[0x1b] < 0.0876146281586614 && metrics[0x0c] < 0.0697297582418166 && metrics[0x15] * 0.375145813128857 < metrics[0x0c] && metrics[0x1a] > 0.241223208430199 && metrics[0x09] > 0.393198141870016);

    match += (metrics[0x1d] * 0.0828850109174866 < metrics[0x1a] && metrics[0x0d] > 0.293245404007791 && metrics[0x0b] * 0.154276386136821 < metrics[0x01] && metrics[0x0e] < 0.0879184957396006 && metrics[0x02] < 0.524592356087428 && metrics[0x09] * 0.136322305777703 > metrics[0x0c] && metrics[0x18] < 0.0523634839541138 && metrics[0x1c] < 0.180161661288956 && metrics[0x16] * 0.480811788426518 < metrics[0x14] && metrics[0x17] < 0.419637320267327 && metrics[0x11] * 0.0279994034713162 < metrics[0x10] && metrics[0x05] * 0.198860807825721 > metrics[0x00] && metrics[0x09] * 0.887404201565939 > metrics[0x0d] && metrics[0x0f] * 0.483036041044127 < metrics[0x09] && metrics[0x0d] * 0.983583807737222 < metrics[0x05] && metrics[0x11] < 0.599892513277478);

    match += (metrics[0x0d] < 0.666274398951188 && metrics[0x08] * 0.242527497769693 > metrics[0x14] && metrics[0x17] * 0.952105170019877 < metrics[0x09] && metrics[0x0b] * 0.0989327979184651 > metrics[0x06] && metrics[0x12] * 0.856825214836377 < metrics[0x04] && metrics[0x15] < 0.068786915266756 && metrics[0x19] * 0.483903873590229 > metrics[0x18] && metrics[0x11] * 0.266311694513941 > metrics[0x00] && metrics[0x19] * 0.930513527988619 > metrics[0x0c] && metrics[0x05] > 0.32106609855018 && metrics[0x11] < 0.598760284547087 && metrics[0x09] * 0.55266730160692 < metrics[0x0d] && metrics[0x1a] > 0.258154142168522 && metrics[0x16] * 0.554003334031634 < metrics[0x14] && metrics[0x15] * 0.308399932282121 < metrics[0x0c] && metrics[0x17] * 0.235444283845606 > metrics[0x0c]);

    match += (metrics[0x16] * 0.550165213084441 < metrics[0x14] && metrics[0x00] * 0.400183222216572 < metrics[0x0c] && metrics[0x0d] * 0.200779585314997 > metrics[0x10] && metrics[0x05] * 0.93983859301213 > metrics[0x11] && metrics[0x0d] < 0.650453732043047 && metrics[0x09] > 0.444288504882369 && metrics[0x19] < 0.119215455681836 && metrics[0x0d] * 0.0966491645604002 < metrics[0x19] && metrics[0x19] * 0.741673684477977 > metrics[0x0c] && metrics[0x0b] * 0.109370043219847 > metrics[0x06] && metrics[0x17] < 0.421356551800347 && metrics[0x0d] > 0.252698883624689 && metrics[0x18] * 0.964908827652824 < metrics[0x15] && metrics[0x15] * 0.666584191327779 > metrics[0x1b] && metrics[0x09] * 0.263677747533247 < metrics[0x1a] && metrics[0x18] * 0.160764908200289 < metrics[0x10]);

    match += (metrics[0x15] < 0.0682359605530607 && metrics[0x0b] * 0.446955067733178 < metrics[0x09] && metrics[0x11] * 0.521495593678473 > metrics[0x15] && metrics[0x00] < 0.104692480844687 && metrics[0x04] * 0.217059623458507 > metrics[0x14] && metrics[0x19] * 0.933978778928921 > metrics[0x08] && metrics[0x0d] < 0.723624990068732 && metrics[0x18] * 0.96144887590474 < metrics[0x15] && metrics[0x15] * 0.0837505581154154 > metrics[0x14] && metrics[0x0b] * 0.123951548538042 > metrics[0x0a] && metrics[0x18] * 0.180187298958487 > metrics[0x14] && metrics[0x1a] > 0.252593218801724 && metrics[0x17] * 0.465597220113583 > metrics[0x19] && metrics[0x1d] * 0.309420931330241 > metrics[0x12] && metrics[0x1a] * 0.158724774727244 > metrics[0x18] && metrics[0x1b] < 0.0990131164900383);

    match += (metrics[0x03] * 0.123136011917211 > metrics[0x19] && metrics[0x1a] * 0.972816398004834 > metrics[0x1d] && metrics[0x0a] < 0.124087194880882 && metrics[0x09] * 0.911603366219676 > metrics[0x0d] && metrics[0x19] > 0.0364020696296592 && metrics[0x08] < 0.0922452265992675 && metrics[0x0f] * 0.0770755696745802 > metrics[0x15] && metrics[0x0f] * 0.104228578573341 > metrics[0x00] && metrics[0x18] * 0.749438365306688 < metrics[0x1d] && metrics[0x09] * 0.553480879936309 < metrics[0x0d] && metrics[0x1c] < 0.150179840394031 && metrics[0x09] * 0.84284527313137 > metrics[0x17] && metrics[0x18] * 0.417769721012458 < metrics[0x04] && metrics[0x09] * 0.142402574807985 > metrics[0x0e] && metrics[0x11] < 0.602798406523419 && metrics[0x13] * 0.00556734720120033 > metrics[0x14]);

    match += (metrics[0x1d] < 0.478510464101877 && metrics[0x09] * 0.274824407078025 > metrics[0x1c] && metrics[0x00] * 0.348275610626526 < metrics[0x10] && metrics[0x04] > 0.0104485094511233 && metrics[0x09] * 0.20842365381451 > metrics[0x06] && metrics[0x11] * 0.273825519691716 > metrics[0x00] && metrics[0x1a] > 0.202927311758518 && metrics[0x04] < 0.0831148905141532 && metrics[0x17] < 0.419741332477145 && metrics[0x01] * 0.257130089325102 > metrics[0x08] && metrics[0x19] < 0.119611983650735 && metrics[0x1d] * 0.683542588736142 < metrics[0x01] && metrics[0x11] < 0.608555216776931 && metrics[0x16] * 0.487917271479883 < metrics[0x14] && metrics[0x05] * 0.769105841064921 < metrics[0x0d] && metrics[0x03] * 0.652885510825811 > metrics[0x0d]);

    if(match>=4) {
	curimg = img;
	getViews();
    } else
	tryAgain();
}

QPair<QString, QPair<QImage, int> > Pr0nGet::getPr0n() {
    refill();
    if(sets.count())
	return sets.takeFirst();
    else
	return qMakePair(QString(), qMakePair(QImage(), -1));
}
