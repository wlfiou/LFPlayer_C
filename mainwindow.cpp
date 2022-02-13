#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "lfplayer.h"
using namespace std;
#define file_name "/Users/wanglufei/Desktop/MP4/400_300_25.mp4"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    LFPlayer player;
    player.Open(file_name,nullptr);
    player.start();
    for (; ; ) {
        player.update();
        MSleep(10);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

