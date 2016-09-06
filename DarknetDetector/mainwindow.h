#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include "framereader.h"
#include "timer.h"
#include <QTimer>
#include <atomic>

#include "detection_layer.h"
#include "cost_layer.h"
#include "utils.h"
#include "parser.h"
#include "box.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    int getMouseX();
    int getMouseY();

private slots:
    void on_bnUpload_clicked();
    void on_bnPlay_clicked();
    void on_bnNext_clicked();
    void on_bnMClose_clicked();
    void on_bnMMin_clicked();

private slots:
    void displayFrame();
    void processFrame();

    void on_bnExit_clicked();

private:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent * event);

private:
    QPoint  m_dragPosition = QPoint{-1,0};
    Ui::MainWindow *ui;

private:
    QTimer *timer; // the timer that will refresh the widget

    FrameReader frameReader;
    std::string lastVideo;

    cv::Mat frame, frameResized;
    cv::Mat colorFrame;

    int width, height;

    bool needPause;
    int lastComputedFrame;

    network net;
    detection_layer l;
    box *boxes;
    float **probs;
};

#endif // MAINWINDOW_H
