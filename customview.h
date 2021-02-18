#ifndef CUSTOMVIEW_H
#define CUSTOMVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QScrollBar>
#include <QTouchEvent>
#include <QMimeData>

#include "qimagerotatetoqpixmap.h"

class CustomView : public QGraphicsView
{
    Q_OBJECT

public:
    CustomView(QWidget *parent = nullptr) : QGraphicsView(parent)
    {
        viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
        setDragMode(ScrollHandDrag);
        setAcceptDrops(true);
        // viewport()->setAcceptDrops(true);

        graphicsscene = new QGraphicsScene();
        setScene(graphicsscene);
    }

    bool viewportEvent(QEvent *event) override
    {
        switch (event->type()) {
        case QEvent::TouchBegin:
        case QEvent::TouchUpdate:
        case QEvent::TouchEnd:
        {
            QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
            QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
            if (touchPoints.count() == 2) {
                // determine scale factor
                const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
                const QTouchEvent::TouchPoint &touchPoint1 = touchPoints.last();
                qreal currentScaleFactor =
                        QLineF(touchPoint0.pos(), touchPoint1.pos()).length()
                        / QLineF(touchPoint0.startPos(), touchPoint1.startPos()).length();
                if (touchEvent->touchPointStates() & Qt::TouchPointReleased) {
                    // if one of the fingers is released, remember the current scale
                    // factor so that adding another finger later will continue zooming
                    // by adding new scale factor to the existing remembered value.
                    totalScaleFactor *= currentScaleFactor;
                    currentScaleFactor = 1;
                }
                setTransform(QTransform::fromScale(totalScaleFactor * currentScaleFactor,
                                                   totalScaleFactor * currentScaleFactor));
            }
            return true;
        }
        default:
            break;
        }
        return QGraphicsView::viewportEvent(event);
    }

    void SetPixmap(const QString &strDir, int orientation = 0)
    {
        graphicsscene->clear();                                             // graphicssceneをクリア
        totalScaleFactor = 1;                                               // totalScaleFactorをクリア
        setTransform(QTransform::fromScale(totalScaleFactor, totalScaleFactor));

        QImage image(strDir);                                               // 画像を回転させpixmapに変換
        QPixmap pixmap(QImageRotateToQPixmap::qimageRotateToQPixmap(image, orientation, Qt::SmoothTransformation));
        setSceneRect(0, 0, pixmap.width(), pixmap.height());                // graphicssceneのサイズ設定
        graphicsscene->addPixmap(pixmap);                                   // graphicssceneにpixmapを設定
    }

protected:
    void dragEnterEvent(QDragEnterEvent *e) override
    {
        if (e->mimeData()->hasText()) {
            setAcceptDrops(true);
            e->acceptProposedAction();
            qDebug("dragEnterEvent: %s", qPrintable(e->mimeData()->text()));
        }
   }

    void dropEvent(QDropEvent *e) override
    {
        if (e->mimeData()->hasText())
        {
            QString csvtext = e->mimeData()->text();
            QStringList csvlist = csvtext.split(",");
            SetPixmap(csvlist[0], csvlist[1].toInt());
            qDebug("dropEvent: %s", qPrintable(e->mimeData()->text()));
        }

    }
    void dragMoveEvent(QDragMoveEvent *event) override {}
    void dragLeaveEvent(QDragLeaveEvent *event) override {}

private:
    qreal totalScaleFactor = 1;
    QGraphicsScene *graphicsscene;
};


#endif // CUSTOMVIEW_H
