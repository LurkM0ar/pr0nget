#include <QMessageBox>
#include <QShortcut>
#include <QDateTime>

#include "pr0nmain.h"
#include "picwidget.h"

#include <QDebug>


Pr0nMain::Pr0nMain(QWidget *parent) : QMainWindow(parent) {
    setupUi(this);

    attempts = guesses = prons = saves = 0;
    QWidget *w = new QWidget;
    table = new QGridLayout;

    for(unsigned int y=0; y<numRows; y++) {
	for(unsigned int x=0; x<numCols; x++) {
	    PicWidget *pw = new PicWidget();
	    table->addWidget(pw, y, x);
	    connect(this, SIGNAL(nextSet()), pw, SLOT(newImage()));
	    connect(this, SIGNAL(prevSet()), pw, SLOT(oldImage()));
	    connect(pw, SIGNAL(imageUpdated()), this, SLOT(moveToTop()));
	    connect(pw, SIGNAL(attemptEvt()), this, SLOT(bumpAttempts()));
	    connect(pw, SIGNAL(guessEvt()), this, SLOT(bumpGuesses()));
	    connect(pw, SIGNAL(pr0nEvt()), this, SLOT(bumpPr0ns()));
	    connect(pw, SIGNAL(saveEvt()), this, SLOT(bumpSaves()));
	}
    }

    t = QDateTime::currentDateTime();

    QFrame *hLine = new QFrame;
    hLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    hLine->setLineWidth(1);
    table->addWidget(hLine, numRows, 0, 1, -1);

    QBoxLayout *statusLayOut = new QBoxLayout(QBoxLayout::LeftToRight);
    labelAttempts = new QLabel;
    statusLayOut->addWidget(labelAttempts);
    labelGuessed = new QLabel;
    statusLayOut->addWidget(labelGuessed);
    labelPr0n = new QLabel;
    statusLayOut->addWidget(labelPr0n);
    labelSaved = new QLabel;
    statusLayOut->addWidget(labelSaved);
    table->addLayout(statusLayOut, numRows+1, 0, 1, -1);

    w->setLayout(table);
    setCentralWidget(w);

    QShortcut *sc = new QShortcut(QKeySequence(" "), this);
    connect(sc, SIGNAL(activated()), this, SLOT(newSet()));

    sc = new QShortcut(QKeySequence("Backspace"), this);
    connect(sc, SIGNAL(activated()), this, SIGNAL(prevSet()));

    qsrand(QDateTime::currentMSecsSinceEpoch());
}

void Pr0nMain::bumpAttempts() {
    attempts++;
    updateStatus();
}
void Pr0nMain::bumpGuesses() {
    guesses++;
    updateStatus();
}
void Pr0nMain::bumpPr0ns() {
    prons++;
    updateStatus();
}

void Pr0nMain::bumpSaves() {
    saves++;
    updateStatus();
}

QString Pr0nMain::getFrequency(const QString &what, unsigned int value) {
    qint64 elapsed = t.msecsTo(QDateTime::currentDateTime());
    if(!value)
	return "none yet";
    if(value*1000/elapsed > 0)
	return QString::number(value) + " (" + QString::number(value*1000/elapsed) + " " + what + "/s)";
    if(60*value*1000/elapsed > 0)
	return QString::number(value) + " (" + QString::number(60*value*1000/elapsed) + " " + what + "/m)";
    return QString::number(value) + " (" + QString::number(60*60*value*1000/elapsed) + " " + what + "/h)";
}

void Pr0nMain::updateStatus() {
    labelAttempts->setText("Bruteforce attempts: " + getFrequency("tries", attempts));
    labelGuessed->setText("Images found: " + getFrequency("guesses", guesses));
    labelPr0n->setText("Possible pr0n: " + getFrequency("images", prons));
    labelSaved->setText("Pr0n saved: " + getFrequency("images", saves));
}

void Pr0nMain::newSet() {
    emit nextSet();
}

void Pr0nMain::moveToTop() {
    QObject *sobj = sender();
    if(!sobj)
	return;
    PicWidget *pw = qobject_cast<PicWidget *>(sobj);
    if(!pw)
	return;

    QList<QWidget *> list;
    for(unsigned int y=0; y<numRows; y++) {
	for(unsigned int x=0; x<numCols; x++) {
	    QLayoutItem *litem = table->itemAtPosition(y, x);
	    if(litem->widget() != pw)
		list << litem->widget();
	    else
		list.prepend(litem->widget());
	    table->removeWidget(litem->widget());
	}
    }

    for(unsigned int y=0; y<numRows; y++) {
	for(unsigned int x=0; x<numCols; x++) {
	    QWidget *w = list.takeFirst();
	    table->addWidget(w, y, x);
	}
    }
}
