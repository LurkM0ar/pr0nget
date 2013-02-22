#ifndef __PR0NMAIN_H__
#define __PR0NMAIN_H__

#include <QGridLayout>
#include <QDateTime>
#include <QLabel>

#include "ui_pr0nmain.h"

class Pr0nMain : public QMainWindow, private Ui::Pr0nMain {
    Q_OBJECT

public:
    Pr0nMain(QWidget * parent = 0);

private:
    static const unsigned int numCols = 8;
    static const unsigned int numRows = 4;
    QGridLayout *table;
    QLabel *labelAttempts, *labelGuessed, *labelPr0n, *labelSaved;
    int attempts, guesses, prons, saves;
    void updateStatus();
    QDateTime t;

    QString getFrequency(const QString &what, unsigned int value);

signals:
    void nextSet();
    void prevSet();
    void moveToTop(int, int);

private slots:
    void newSet();
    void moveToTop();

    void bumpAttempts();
    void bumpGuesses();
    void bumpPr0ns();
    void bumpSaves();
};

#endif
