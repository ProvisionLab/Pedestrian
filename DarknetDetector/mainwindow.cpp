#include <QtCore>
#include <QCursor>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <opencv2/opencv.hpp>


#define CLASSNUM 4
const float nms=.5;
const float thresh = 0.2;

void convert_detections(float *predictions, int classes, int num, int square, int side, int w, int h, float thresh, float **probs, box *boxes, int only_objectness)
{
    int i,j,n;
    //int per_cell = 5*num+classes;
    for (i = 0; i < side*side; ++i){
        int row = i / side;
        int col = i % side;
        for(n = 0; n < num; ++n){
            int index = i*num + n;
            int p_index = side*side*classes + i*num + n;
            float scale = predictions[p_index];
            int box_index = side*side*(classes + num) + (i*num + n)*4;
            boxes[index].x = (predictions[box_index + 0] + col) / side * w;
            boxes[index].y = (predictions[box_index + 1] + row) / side * h;
            boxes[index].w = pow(predictions[box_index + 2], (square?2:1)) * w;
            boxes[index].h = pow(predictions[box_index + 3], (square?2:1)) * h;
            for(j = 0; j < classes; ++j){
                int class_index = i*classes;
                float prob = scale*predictions[class_index+j];
                probs[index][j] = (prob > thresh) ? prob : 0;
            }
            if(only_objectness){
                probs[index][0] = scale;
            }
        }
    }
}

void draw_yolo_detections(cv::Mat& im, box *boxes, float **probs, int total, int classes, int w, int h)
{
    int i, j;
    for(i = 0; i < total; ++i)
    {
        float xmin = boxes[i].x - boxes[i].w/2.;
        float xmax = boxes[i].x + boxes[i].w/2.;
        float ymin = boxes[i].y - boxes[i].h/2.;
        float ymax = boxes[i].y + boxes[i].h/2.;

        if (xmin < 0) xmin = 0;
        if (ymin < 0) ymin = 0;
        if (xmax > w) xmax = w;
        if (ymax > h) ymax = h;

        for(j = 0; j < classes; ++j)
        {
            if (probs[i][j])
            {
                cv::Rect rect;
                rect.x = xmin;
                rect.y = ymin;
                rect.width = xmax - xmin;
                rect.height = ymax - ymin;
                std::cout << probs[i][j] << " " << rect << std::endl;
                cv::rectangle(im, rect, cv::Scalar(0,255,0), 3);
            }
        }
    }
}

image mat_to_image(const cv::Mat& src)
{
    unsigned char *data = (unsigned char *)src.data;
    int h = src.rows;
    int w = src.cols;
    int c = src.channels();
    image out = make_image(w, h, c);
    int i, j, k, count=0;
    for(k= 0; k < c; ++k)
    {
        for(i = 0; i < h; ++i)
        {
            for(j = 0; j < w; ++j)
            {
                out.data[count++] = data[(i * w + j)*c + k]/255.;
            }
        }
    }
    return out;
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    needPause(false)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(displayFrame()));
    timer->setInterval(100);

    net = parse_network_cfg("yolo-tiny.cfg");
    load_weights(&net, "yolo-tiny.weights");

    l = net.layers[net.n-1];
    set_batch_network(&net, 1);

    boxes =(box*) calloc(l.side*l.side*l.n, sizeof(box));
    probs =(float**) calloc(l.side*l.side*l.n, sizeof(float *));
    for(int j = 0; j < l.side*l.side*l.n; ++j)
    {
        probs[j] =(float*) calloc(l.classes, sizeof(float *));
    }

    ui->title_2->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragPosition.x() > 0 && ui->titlebar->geometry().contains(event->pos()))
    {
        auto delta = event->pos() - m_dragPosition;

        if (event->buttons() & Qt::LeftButton) {
            move(this->pos() + delta);
            event->accept();
        }
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (ui->titlebar->geometry().contains(event->pos()))
    {
        if (event->button() == Qt::LeftButton)
        {
            m_dragPosition = event->pos();
            event->accept();
            needPause = true;
        }
    }

    if (ui->bg->geometry().contains(event->pos()))
    {
        //    int x = getMouseX(), y = getMouseY();
        if (!timer->isActive())
        {
            return;
        }
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *)
{
    needPause = false;
    m_dragPosition.setX(-1);
}

void MainWindow::on_bnUpload_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Open Video"), "", tr("Video File (*.mp4 *.avi)"));;
    if (!filename.isEmpty())
    {
        lastComputedFrame = -1;

        QApplication::processEvents();
        lastVideo = filename.toStdString();
        frameReader.initSource(lastVideo);
        if (!frameReader.getTotalNumFrames())
        {
            QMessageBox::information(0, "Information", "Can't open' " + filename);
            lastVideo = "";
            return;
        }
        timer->start();
        ui->bnUpload->hide();
        ui->bg->show();
        frameReader.pause(true);
    }
}

void MainWindow::on_bnPlay_clicked()
{
    frameReader.pause();
}

void MainWindow::on_bnMClose_clicked()
{
    this->close();
}

void MainWindow::on_bnMMin_clicked()
{
    this->showMinimized();
}

void MainWindow::displayFrame()
{
    if (!needPause)
    {
        processFrame();
    }
}

void MainWindow::processFrame()
{
    bool isNew = false;
    if (!frameReader.getFrame(frame, isNew))
    {
        QMessageBox msgBox;
        msgBox.setText("End of video");
        msgBox.setWindowTitle("End");
        msgBox.exec();
        return;
    }

    cv::resize(frame, frameResized, cv::Size(ui->bg->width(), ui->bg->height())); // fix resizing
    cvtColor(frameResized, colorFrame, CV_BGR2RGB);

    width = frameResized.cols;
    height = frameResized.rows;


    if (frameReader.getFrameNum() != lastComputedFrame)
    {
        ui->title_2->setVisible(true);
        QCoreApplication::processEvents();

        cv::Mat resized;
        cv::resize(frameResized, resized, cv::Size(net.w, net.h));

        //image im = load_image_color("/home/ivan/darknet/predictions.png",0,0);
        //image sized = resize_image(im, net.w, net.h);

        //std::cout << __LINE__ << std::endl;

        image sized = mat_to_image(resized);
        //std::cout << __LINE__ << std::endl;
        float *X = sized.data;
        float *predictions = network_predict(net, X);
        //std::cout << __LINE__ << std::endl;
        convert_detections(predictions, l.classes, l.n, l.sqrt, l.side, colorFrame.cols, colorFrame.rows, thresh, probs, boxes, 0);
        //std::cout << __LINE__ << std::endl;
        if (nms)
        {
            do_nms_sort(boxes, probs, l.side*l.side*l.n, l.classes, nms);
            //std::cout << __LINE__ << std::endl;
        }
        //std::cout << __LINE__ << std::endl;
        free_image(sized);
        //std::cout << __LINE__ << std::endl;

        //draw_detections(im, l.side*l.side*l.n, thresh, boxes, probs, voc_names, voc_labels, CLASSNUM);

        lastComputedFrame = frameReader.getFrameNum();

        // show info
        std::cout << "Frame: " << frameReader.getFrameNum() << " / " << frameReader.getTotalNumFrames() << std::endl;
    }

    draw_yolo_detections(colorFrame, boxes, probs, l.side*l.side*l.n, CLASSNUM, colorFrame.cols, colorFrame.rows);

    // draw image
    QImage img = QImage((unsigned char*)colorFrame.data, colorFrame.cols, colorFrame.rows, colorFrame.step, QImage::Format_RGB888);
    QPixmap pix = QPixmap::fromImage(img);
    ui->bg->setPixmap(pix);
    ui->title_2->setVisible(false);
}

void MainWindow::on_bnNext_clicked()
{
    frameReader.next();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (char(event->key()))
    {
    case ' ':
        on_bnPlay_clicked();
        break;
    default:
        break;
    }
}

int MainWindow::getMouseX()
{
    int fixedX = ui->bg->mapFromGlobal(QCursor::pos()).x();
    if (fixedX < 0)
    {
        fixedX = 0;
    }
    else
        if (fixedX >= width)
        {
            return width - 1;
        }
    return fixedX;
}

int MainWindow::getMouseY()
{
    int fixedY = ui->bg->mapFromGlobal(QCursor::pos()).y();
    if (fixedY < 0)
    {
        fixedY = 0;
    }
    else if (fixedY >= height)
    {
        return height - 1;
    }
    return fixedY;
}

void MainWindow::on_bnExit_clicked()
{
    ui->bnUpload->show();
    ui->bg->hide();
    timer->stop();
}
