#pragma once

#include "controller.h"
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>
#include <QLCDNumber>
#include <QKeyEvent>
#include <QComboBox>

enum class Mode {Polygons, Light, StaticLight};

class Canvas : public QWidget
{
    Q_OBJECT

public:
    Canvas(QWidget* parent = nullptr);

    ~Canvas();

    void SetMode(const Mode& mode);

    void SetFlag(bool fl);

    void DrawPolygon(const Polygon& polygon, QPainter& painter);

    void FillPolygon(const Polygon& polygon, QPainter& painter);

    void DrawPolygons(const std::vector<Polygon>& polygons, QPainter& painter);

    void CheckLastPolygon();

    void PolygonsEvent();

    void StaticLightEvent();

    void Press(QMouseEvent* event);

    void Click();

    void DoubleClick(QMouseEvent* event);

    void SetMode(QComboBox* mode);

    QLCDNumber* GetFPSNumber() const;


protected:
    void paintEvent(QPaintEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;

    void mouseDoubleClickEvent(QMouseEvent* event) override;

    void mouseMoveEvent(QMouseEvent* event) override;

    void keyPressEvent(QKeyEvent* event) override;

private:
    bool is_finished_ = true;
    bool is_edit_ = false;
    bool is_left_ = false;
    QPointF last_vertex_ = QPointF(-1, -1);
    QPointF cursor_point_ = QPointF(-1, -1);
    QTimer* timer_ = nullptr;
    QTimer* click_timer_ = nullptr;
    QPointF click_point_ = QPointF(-1, -1);
    Controller controller_;
    Mode mode_ = Mode::Light;
    QLCDNumber* fps_number_ = nullptr;
    QComboBox* mode_box_ = nullptr;
    int last_update_fps = 0;
    int last_time_ = clock();
};
