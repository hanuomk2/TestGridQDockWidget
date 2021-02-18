#ifndef CUSTOMLISTWIDGET_H
#define CUSTOMLISTWIDGET_H

#include <QListWidget>
#include <QDir>
#include <QMouseEvent>
#include <QMimeData>
#include <QDrag>
#include <QTransform>

#include "qimagerotatetoqpixmap.h"

extern "C" {
    #include "exif-master/exif.h"
}

const int ICON_SIZE=160;
const int DOCKWIDGET_GRID_SIZE=ICON_SIZE+32;

class CustomListWidget : public QListWidget
{
    Q_OBJECT

public:
    CustomListWidget(QWidget *parent = nullptr) : QListWidget(parent)
    {
        setViewMode(QListView::ViewMode::IconMode);
        setWrapping(false);
        setFlow(QListView::Flow::TopToBottom);
        setDragEnabled(true);
        setAcceptDrops(false);
        setDragDropMode(QAbstractItemView::DragDropMode::DragOnly);
        setGridSize(QSize(DOCKWIDGET_GRID_SIZE, DOCKWIDGET_GRID_SIZE));
        setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    }

    void createListWidgetIcon(QString strFolderPath)
    {
        currentfolderpath = strFolderPath;

        QDir dir(currentfolderpath);
        QStringList listDir;
        QStringList listImage;
        QString strDir;
        QString strImage;

        QStringList listFilter;
        listFilter << "*.jpg" << "*.jpeg" << "*.bmp" << "*.gif" << "*.png";

        clear();

        // フォルダの名前、アイコン(固定)をlistwidgetに登録する
        listDir = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for(int i=0; i < listDir.size(); i++)
        {
            strDir = listDir.at(i);
            QListWidgetItem *item = new QListWidgetItem(QIcon(":/Folder.png"), strDir);
            // item->setTextAlignment(Qt::AlignHCenter|Qt::AlignBottom);
            addItem(item);
        }

        // Imageファイルの名前、アイコンをlistwidgetに登録する
        listImage = dir.entryList(listFilter, QDir::Files|QDir::Readable, QDir::Name);
        for(int i=0; i < listImage.size(); i++)
        {
            strImage = listImage.at(i);
            QListWidgetItem *item = new QListWidgetItem(strImage);
            if(!getExifThumbnailImage(currentfolderpath + "/" + strImage, item)) {
                QIcon mIcon(currentfolderpath + "/" + strImage);
                item->setIcon(mIcon);
            }
            addItem(item);
        }
    }

    void mousePressEvent(QMouseEvent *e)
    {
        //マウス左ボタンをクリックしたら
        if(e->button() == Qt::LeftButton)
        {
            //そのマウスの位置を保存
            startPos = e->pos();
        }

        //デフォルトの動作へ
        QListWidget::mousePressEvent(e);
    }

    void mouseMoveEvent(QMouseEvent *e)
    {
        ////マウス左ボタンをクリック中なら
        if(e->buttons() & Qt::LeftButton)
        {
            //保存していたマウスの位置と現在のマウスの位置の差を求める
            //manhattanLengthは絶対値を求めるもの
            int distance = (e->pos() - startPos).manhattanLength();

            //上で求めた差分とアプリで設定してあるドラッグ＆ドロップを開始する差分
            //を比べ、それ以上ならドラッグを開始する。
            //なおstartDragDistance()はデフォルトでは4が返る
            //変更はsetStartDragDistance(int)
            if(distance >= 4) // QApplication::startDragDistance())
            {
                QMimeData *mimeData = new QMimeData;
                QString strDir = currentfolderpath + "/" + currentItem()->text();
                int orientation = currentItem()->data(Qt::UserRole).toInt();
                QString csvtext = strDir + "," + QString::number(orientation);
                mimeData->setText(csvtext);

                QDrag *drag = new QDrag(this);
                drag->setMimeData(mimeData);
                drag->setPixmap(currentItem()->icon().pixmap(QSize(ICON_SIZE, ICON_SIZE)));
                drag->exec(Qt::CopyAction);
            }
            QListWidget::mouseMoveEvent(e);
        }
    }

private:
    QPixmap pixmap;
    QString currentfolderpath;
    QPoint startPos;//マウスの位置

    bool getExifThumbnailImage(const QString &strPATH, QListWidgetItem *item)
    {
        //構造体、変数等の初期化
        void **ifdArray = nullptr;
        TagNodeInfo *tag;
        int result;

        // QStringをchar*に変換
        char* image_full_path = strPATH.toLocal8Bit().data();
        unsigned int len = (unsigned int)strPATH.toLocal8Bit().size();

        /* IFDテーブルの作成 *********************************************************************/
        // parse the JPEG header and create the pointer array of the IFD tables
        ifdArray = createIfdTableArray(image_full_path, &result);

        // check result status
        switch (result) {
        case 0: // no IFDs
            qDebug("[%s] does not seem to contain the Exif segment.\n", image_full_path);
            break;
        case ERR_READ_FILE:
            qDebug("failed to open or read [%s].\n", image_full_path);
            break;
        case ERR_INVALID_JPEG:
            qDebug("[%s] is not a valid JPEG file.\n", image_full_path);
            break;
        case ERR_INVALID_APP1HEADER:
            qDebug("[%s] does not have valid Exif segment header.\n", image_full_path);
            break;
        case ERR_INVALID_IFD:
            qDebug("[%s] contains one or more IFD errors. use -v for details.\n", image_full_path);
            break;
        default:
            qDebug("[%s] createIfdTableArray: result=%d\n", image_full_path, result);
            break;
        }

        if (!ifdArray) return false;                    // IDFテーブルがなかった場合等

         /* 1st IFDにサムネイル画像があれば、それを表示する **************************************/
         unsigned char *p;

         // try to get thumbnail data from 1st IFD
         p = getThumbnailDataOnIfdTableArray(ifdArray, &len, &result);
         if (p) {
             //サムネイルデータをJPGフォーマットでQImageに流し込む
             QImage image;
             image.loadFromData(p, (int)len, "JPG");

             /* TAG_Orientationを基に画像の向きを正す**************************************/
             tag = getTagInfo(ifdArray, IFD_0TH, TAG_Orientation);
             if (tag) {
                 if (!tag->error) {
                     // itemにアイコンを登録(qimageRotateToQpixmap呼び出し)
                     QIcon mIcon(QImageRotateToQPixmap::qimageRotateToQPixmap(image, tag->numData[0]));
                     item->setIcon(mIcon);
                     // TAG_OrientationはQt::UserRoleに保存する
                     item->setData(Qt::UserRole, tag->numData[0]);
                 }
                 freeTagInfo(tag);
             }
             // free IFD table array
             freeIfdTableArray(ifdArray);
         }
         else
         {
             qDebug("getThumbnailDataOnIfdTableArray: ret=%d\n", result);
             // Thumbnailがなかった場合等

             // free IFD table array
             freeIfdTableArray(ifdArray);

             return false;
         }

        return true;
    }

};



#endif // CUSTOMLISTWIDGET_H
