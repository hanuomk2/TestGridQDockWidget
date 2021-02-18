#include "mainwindow.h"
#include <QTextEdit>
#include <QCommonStyle>
#include <QScroller>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QDrag>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setFixedSize(DESKTOP_WINDOW_WIDTH, DESKTOP_WINDOW_HEIGHT);

    customview = new CustomView();
    setCentralWidget(customview);

    createActions();
    createToolBar();
    createDockWidget();
}

MainWindow::~MainWindow()
{
}

void MainWindow::calledDockLocationChanged(Qt::DockWidgetArea area)
{
    qDebug("calledDockLocationChanged %0x", area);
    switch (area) {
        case Qt::TopDockWidgetArea:     // 上のドックエリア
        case Qt::BottomDockWidgetArea:  // 下のドックエリア
            customlistwidget->setFlow(QListView::Flow::LeftToRight);
            dockwidget->resize(DESKTOP_WINDOW_WIDTH, DOCKWIDGET_MINIMUM_HEIGHT);
            break;
        case Qt::LeftDockWidgetArea:    // 左のドックエリア
        case Qt::RightDockWidgetArea:   // 右のドックエリア
            customlistwidget->setFlow(QListView::Flow::TopToBottom);
            dockwidget->resize(DOCKWIDGET_MINIMUM_WIDTH, DESKTOP_WINDOW_HEIGHT);
            break;
        case Qt::NoDockWidgetArea:      // ドックエリア外(フローティング)
            customlistwidget->setFlow(QListView::Flow::TopToBottom);
            dockwidget->resize(DOCKWIDGET_FLOWTING_WIDTH, DOCKWIDGET_FLOWTING_HEIGHT);
            break;
        case Qt::DockWidgetArea_Mask:   //使用しない
            break;
    }
}

void MainWindow::createActions()
{
    //各アクションのアイコン等設定
    actionFolderDialog = new QAction(QCommonStyle().standardIcon(QStyle::SP_DirIcon),
                                     tr("&OpenFolderDialog"),
                                     this);
    actionFolderToParent = new QAction(QCommonStyle().standardIcon(QStyle::SP_FileDialogToParent),
                                       tr("&FolderToParent"),
                                       this);
    actionEnterKey = new QAction(QCommonStyle().standardIcon(QStyle::SP_ArrowRight/*SP_CommandLink*/),
                                 tr("&EnterKey"),
                                 this);

    //各ボタンのシグナル・スロット設定
    connect(actionFolderDialog, SIGNAL(triggered()), this, SLOT(openFolderDialog()));
    connect(actionFolderToParent, SIGNAL(triggered()), this, SLOT(upFolderToParent()));
    connect(actionEnterKey, SIGNAL(triggered()), this, SLOT(collectImage()));
}

void MainWindow::createToolBar()
{
    toolbar = new QToolBar();
    toolbar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    toolbar->addAction(actionFolderDialog);
    toolbar->addAction(actionFolderToParent);
    lineedit = new QLineEdit;
    lineedit->setText(strDirPATH);
    toolbar->addWidget(lineedit);
    toolbar->addAction(actionEnterKey);

    addToolBar(Qt::TopToolBarArea, toolbar);

    //lineEditで[Enter]キー押下時のシグナル・スロットを設定
    connect(lineedit, SIGNAL(returnPressed()), this, SLOT(collectImage()));
}

void MainWindow::createDockWidget()
{
    //customlistwidgetの初期化
    customlistwidget = new CustomListWidget();
    QScroller::grabGesture(customlistwidget, QScroller::LeftMouseButtonGesture);

    //customlistWidgetでアイテム選択(クリック)時のシグナル・スロットを設定
    connect(customlistwidget, SIGNAL(clicked(QModelIndex)), this, SLOT(selectImage()));

    //dockwidgetの初期化
    dockwidget = new QDockWidget();
    dockwidget->setAllowedAreas(Qt::AllDockWidgetAreas);
    dockwidget->setWidget(customlistwidget);
    dockwidget->setMinimumSize(DOCKWIDGET_MINIMUM_WIDTH, DOCKWIDGET_MINIMUM_HEIGHT);
    dockwidget->setMaximumSize(DESKTOP_WINDOW_WIDTH, DESKTOP_WINDOW_HEIGHT);
    dockwidget->resize(DESKTOP_WINDOW_WIDTH, DOCKWIDGET_MINIMUM_HEIGHT);
    addDockWidget(Qt::LeftDockWidgetArea, dockwidget);

    //シグナル・スロット設定
    connect(dockwidget, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(calledDockLocationChanged(Qt::DockWidgetArea)));

    //customlistwidgetにイメージを挿入
    customlistwidget->createListWidgetIcon(strDirPATH);
}


void MainWindow::openFolderDialog()
{
    // ファイルダイアログを表示
    QString strDir = QFileDialog::getExistingDirectory(this, "Choose Folder", lineedit->text());
    // ファイルダイアログで正常な戻り値が出力された場合
    if(!strDir.isEmpty()) {
        strDirPATH = strDir;
        lineedit->setText(strDirPATH);
        customlistwidget->createListWidgetIcon(strDirPATH);
    }
}

void MainWindow::upFolderToParent()
{
    QStringList listDirPath = lineedit->text().split("/");
    QString strDir = "";
    int roopend = listDirPath.size()-2;
    for(int i=0; i < roopend; i++)
        strDir += listDirPath.at(i) + "/";
    strDir += listDirPath.at(roopend);
    if(roopend <= 0) strDir += "/";

    // ルートフォルダではない場合
    if(!strDir.isEmpty()) {
        strDirPATH = strDir;
        lineedit->setText(strDirPATH);
        customlistwidget->createListWidgetIcon(strDirPATH); // customlistwidgetにイメージを挿入
    }
}

void MainWindow::collectImage()
{
    QString strDir = lineedit->text();
    QDir dir(strDir);

    if(dir.exists()) {
        strDirPATH = strDir;
        customlistwidget->createListWidgetIcon(strDirPATH); // customlistwidgetにイメージを挿入
    }
}

//customlistWidgetで画像(フォルダ含む)を選択
void MainWindow::selectImage()
{
    //現在選択されているアイテムを取得
    QListWidgetItem* item = customlistwidget->currentItem();
    QString strDir = strDirPATH + "/" + item->text();
    QDir dir(strDir);

    //アイテムがフォルダの場合、listWidgetを更新
    if(dir.exists()) {
        strDirPATH = strDir;
        lineedit->setText(strDirPATH);
        customlistwidget->createListWidgetIcon(strDirPATH); // listwidgetにイメージを挿入
    }
    //アイテムが画像の場合、customsview->customsceneで画像をセット
    else customview->SetPixmap(strDir, item->data(Qt::UserRole).toInt());
}

