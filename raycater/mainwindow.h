#pragma once

#include "canvas.h"
#include <QMainWindow>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private slots:
    void ModeUpdated();

private:
    QVBoxLayout* layout_ = nullptr;
    QHBoxLayout* up_layout_ = nullptr;
    QHBoxLayout* main_layout_ = nullptr;
    QComboBox* mode_ = nullptr;
    QWidget* central_widget_ = nullptr;
    Canvas* canvas_ = nullptr;
    QLCDNumber* fps_number_ = nullptr;
    QLabel* text_label_ = nullptr;
};
