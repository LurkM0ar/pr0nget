#include <QApplication>
#include <QMessageBox>

#include "pr0nmain.h"

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    Pr0nMain MainWin;

    MainWin.show();
    return app.exec();
}

