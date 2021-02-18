#ifndef QIMAGEROTATETOQPIXMAP_H
#define QIMAGEROTATETOQPIXMAP_H

#include <QImage>
#include <QTransform>
#include <QPixmap>

class QImageRotateToQPixmap{
public:
    static QPixmap qimageRotateToQPixmap(const QImage &imagesource, int orientation, Qt::TransformationMode mode = Qt::FastTransformation)
    {
        QImage image(imagesource);

        switch (orientation) {
            case 2: case 5: case 7:
                // 水平反転
                image = image.mirrored(true, false);
                break;
            case 4:
                // 垂直反転
                image = image.mirrored(false, true);
                break;
            default:
                break;
        }
        // 回転処理
        QTransform t;
        switch (orientation) {
            case 3:
                // image = image.transformed(QMatrix().rotate(180));
                image = image.transformed(t.rotate(180), mode);
                break;
            case 6: case 7:
                // image = image.transformed(QMatrix().rotate(90));
                image = image.transformed(t.rotate(90), mode);
                break;
            case 5: case 8:
                // image = image.transformed(QMatrix().rotate(270), Qt::FastTransformation);
                image = image.transformed(t.rotate(270), mode);
                break;
            default:
                break;
        }
        // 描画処理(QImageをQPixmapに変換)
        return QPixmap::fromImage(image);
    }
};

#endif // QIMAGEROTATETOQPIXMAP_H
