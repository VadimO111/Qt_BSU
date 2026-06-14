#include "mainwindow.h"
#include <QDebug>
#include <QPainter>
#include <QRect>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    central_widget_ = new QWidget(this);
    setCentralWidget(central_widget_);
    layout_ = new QVBoxLayout(central_widget_);
    up_layout_ = new QHBoxLayout();
    main_layout_ = new QHBoxLayout();

    mode_ = new QComboBox(this);
    mode_->addItem("Light");
    mode_->addItem("Polygons");
    mode_->addItem("Static Light");

    text_label_ = new QLabel("FPS", this);
    text_label_->setStyleSheet(
        "QLabel {"
        "   background-color: rgba(3, 50, 100, 255);"
        "   color: #FF5733;"
        "   border: 2px solid black;"
        "   padding: 4px 8px;"
        "   font: bold 14px 'Arial';"
        "   min-width: 80px;"
        "   text-align: center;"
        "}"
        "QLabel:hover {"
        "   background-color: rgba(3, 50, 100, 200);"
        "   border: 2px solid #00BFFF;"
        "}"
        );

    canvas_ = new Canvas(this);
    fps_number_ = canvas_->GetFPSNumber();
    canvas_->SetMode(mode_);

    main_layout_->addWidget(canvas_);
    up_layout_->addWidget(mode_, 10);
    up_layout_->addWidget(text_label_, 1);
    up_layout_->addWidget(fps_number_, 1);
    layout_->addLayout(up_layout_, 1);
    layout_->addLayout(main_layout_, 100);

    connect(mode_, &QComboBox::currentIndexChanged, this, &MainWindow::ModeUpdated);
}

void MainWindow::ModeUpdated() {
    if (mode_->currentIndex() != 1) {
        canvas_->SetFlag(true);
        canvas_->CheckLastPolygon();
    }
    if (mode_->currentIndex() == 0) {
        canvas_->SetMode(Mode::Light);
    } else if (mode_->currentIndex() == 1){
        canvas_->SetMode(Mode::Polygons);
    } else {
        canvas_->SetMode(Mode::StaticLight);
    }
}

MainWindow::~MainWindow() {
}
