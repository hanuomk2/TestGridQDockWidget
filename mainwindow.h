#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QToolBar>
#include <QLineEdit>
#include <QDockWidget>

#include "customlistwidget.h"
#include "customview.h"

const int DESKTOP_WINDOW_WIDTH=1280;
const int DESKTOP_WINDOW_HEIGHT=800;
const int DOCKWIDGET_FLOWTING_WIDTH=200;
const int DOCKWIDGET_FLOWTING_HEIGHT=364;
const int DOCKWIDGET_MINIMUM_WIDTH=DOCKWIDGET_GRID_SIZE+24;
const int DOCKWIDGET_MINIMUM_HEIGHT=DOCKWIDGET_GRID_SIZE+40;

const QString DEFAULT_PATH="/Users";

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void calledDockLocationChanged(Qt::DockWidgetArea area);
    void openFolderDialog();
    void upFolderToParent();
    void collectImage();

    void selectImage();

private:
    QAction *actionFolderDialog;
    QAction *actionFolderToParent;
    QAction *actionEnterKey;

    QToolBar *toolbar;
    QLineEdit *lineedit;
    QDockWidget *dockwidget;

    CustomListWidget *customlistwidget;
    CustomView *customview;

    QString strDirPATH = DEFAULT_PATH;

    void createActions();
    void createToolBar();
    void createDockWidget();
};
#endif // MAINWINDOW_H
