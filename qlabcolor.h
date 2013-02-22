#ifndef __QLABCOLOR_H__
#include <QColor>

class QLabColor : public QColor {

public:
    QLabColor(const QRgb &other);
    void getLabF(qreal *L, qreal *A, qreal *B) const;

    qreal operator-(const QLabColor &color) const;
};

#endif
