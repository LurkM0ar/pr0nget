#include <QtCore/qmath.h>
#include "qlabcolor.h"

#include <QDebug>

QLabColor::QLabColor(const QRgb &other) : QColor(other) {

}

void QLabColor::getLabF(qreal *L, qreal *A, qreal *B) const {
    qreal r, g, b;
    qreal x, y, z;
    getRgbF(&r, &g, &b);

    if (r > 0.04045f) r = qPow(((r + 0.055f) / 1.055f), 2.4f);
    else r /= 12.92f;
    if (g > 0.04045f) g = qPow(((g + 0.055f) / 1.055f), 2.4f);
    else g /= 12.92f;
    if (b > 0.04045f) b = qPow(((b + 0.055f) / 1.055f), 2.4f);
    else b /= 12.92f;

    r *= 100.0f;
    g *= 100.0f;
    b *= 100.0f;

    x = r * 0.4124f + g * 0.3576f + b * 0.1805f;
    y = r * 0.2126f + g * 0.7152f + b * 0.0722f;
    z = r * 0.0193f + g * 0.1192f + b * 0.9505f;

    x /= 95.047f;
    y /= 100.000f;
    z /= 108.883f;

    if (x > 0.008856f) x = qPow(x, 1.0f/3.0f);
    else x = (7.787f * x) + (16.0f / 116.0f);
    if (y > 0.008856f) y = qPow(y, (1.0f/3.0f));
    else y = (7.787f * y) + (16.0f / 116.0f);
    if (z > 0.008856f) z = qPow(z, (1.0f/3.0f));
    else z = (7.787f * z) + (16.0f / 116.0f);

    if(L)
	*L = (116.0f * y) - 16.0f;
    if(A)
	*A = 500.0f * (x - y);
    if(B)
	*B = 300.0f * (y - z);
}

qreal QLabColor::operator-(const QLabColor &color) const {
    qreal L1, A1, B1;
    qreal L2, A2, B2;

    this->getLabF(&L1, &A1, &B1);
    color.getLabF(&L2, &A2, &B2);

    qreal ret = sqrt(qPow(L1 - L2, 2.0f) + qPow(A1 - A2, 2.0f) + qPow(B1 - B2, 2.0f));

    return ret;
}
