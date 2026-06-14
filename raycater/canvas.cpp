#include "canvas.h"
#include <QPainterPath>
#include <QShortcut>

Canvas::Canvas(QWidget* parent) : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    new QShortcut(QKeySequence(Qt::Key_Up), this, [this]() {
        mode_box_->setCurrentIndex((mode_box_->currentIndex() + 2) % 3);
    });

    new QShortcut(QKeySequence(Qt::Key_Down), this, [this]() {
        mode_box_->setCurrentIndex((mode_box_->currentIndex() + 1) % 3);
    });

    fps_number_ = new QLCDNumber;
    fps_number_->setStyleSheet(
        "QLCDNumber {"
        "   background-color: rgba(3, 50, 100, 255);"
        "   color: #FF5733;"
        "   border: 2px solid black;"
        "   padding: 4px 8px;"
        "   font: bold 14px 'Arial';"
        "   min-width: 80px;"
        "   text-align: center;"
        "}"
        "QLCDNumber: hover {"
        "   background-color: rgba(3, 50, 100, 200);"
        "   border: 2px solid #00BFFF;"
        "}"
        );
    fps_number_->setDigitCount(3);
    fps_number_->display(0);

    click_timer_ = new QTimer(this);
    click_timer_->setSingleShot(true);
    connect(click_timer_, &QTimer::timeout, this, &Canvas::Click);

    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this,  QOverload<>::of(&QWidget::update));
    timer_->start(40);
}

Canvas::~Canvas() = default;

void Canvas::SetMode(const Mode& mode) {
    mode_ = mode;
}

void Canvas::SetFlag(bool fl) {
    is_finished_ = fl;
}

void Canvas::DrawPolygon(const Polygon& polygon, QPainter& painter) {
    size_t sz = polygon.GetVerticies().size();
    for (int i = 0; i < sz; i++) {
        painter.drawLine(polygon[i], polygon[(i + 1) % sz]);
    }
}

void Canvas::FillPolygon(const Polygon& polygon, QPainter& painter) {
    QPainterPath path;
    std::vector<QPointF> verticies = polygon.GetVerticies();

    if (verticies.empty()) {
        return;
    }
    path.moveTo(verticies[0]);
    for (int i = 1; i < verticies.size(); i++) {
        path.lineTo(verticies[i]);
    }
    path.closeSubpath();
    painter.drawPath(path);
}

void Canvas::DrawPolygons(const std::vector<Polygon>& polygons, QPainter& painter) {
    for (int i = 1; i < polygons.size(); i++) {
        DrawPolygon(polygons[i], painter);
    }
    painter.setPen(QPen(Qt::white, 2));
    DrawPolygon(polygons[0], painter);
}

void Canvas::CheckLastPolygon() {
    size_t sz = controller_.GetPolygons().size();
    if (sz == 0) {
        return;
    }
    size_t count_verticies = controller_.GetPolygons()[sz - 1].GetVerticies().size();
    if (count_verticies <= 1) {
        controller_.DeleteLastPolygon();
    }
}

void Canvas::SetMode(QComboBox* mode) {
    mode_box_ = mode;
}

QLCDNumber* Canvas::GetFPSNumber() const {
    return fps_number_;
}

void Canvas::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    int t = clock();
    if (t - last_update_fps > CLOCKS_PER_SEC / 2) {
        fps_number_->display(floor(CLOCKS_PER_SEC / (t - last_time_)));
        last_update_fps = t;
    }
    last_time_ = t;

    QRect rect = this->rect();
    std::vector<QPointF> coordinates = {
        rect.bottomLeft(),
        rect.bottomRight(),
        rect.topRight(),
        rect.topLeft()
    };

    if (controller_.GetPolygons().empty()) {
        controller_.AddPolygon(Polygon(coordinates));
    } else {
        controller_[0] = Polygon(coordinates);
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect, Qt::black);

    //11 Lights
    for (int i = 0; i < 11; i++) {
        if (i) {
            painter.setBrush(QColor(255, 255, 255, 50));
        } else {
            painter.setBrush(QColor(255, 255, 255, 255));
        }
        painter.setPen(Qt::NoPen);
        FillPolygon(controller_.CreateLightArea(i, rect.width(), rect.height()), painter);
    }

    //Static Lights
    for (int i = 0; i < controller_.LightSize(); i++) {
        painter.setBrush(QColor(252, 43, 165, 150));
        painter.setPen(Qt::NoPen);
        FillPolygon(controller_.CreateStaticLightArea(i, rect.width(), rect.height()), painter);
    }

    //Polygons
    painter.setPen(QPen(QColor(155, 219, 228), 4));
    DrawPolygons(controller_.GetPolygons(), painter);

    //Choose vertex
    if (mode_ == Mode::Polygons && !is_finished_) {
        painter.setBrush(QColor(155, 219, 228));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(last_vertex_, 5, 5);
    }

    //Debug Rays
    /*painter.setPen(QPen(Qt::red, 2));
    for (int i = 0; i < 11; i++) {
        if (i) {
            painter.setPen(QPen(Qt::green, 1));
        }
        for (const auto& ray : controller_.GetDebugRays(i, rect.width(), rect.height())) {
            painter.drawLine(ray.GetBegin(), ray.GetEnd());
        }
    }*/

    //Static Light Sources
    for (int i = 0; i < controller_.LightSize(); i++) {
        painter.setBrush(Qt::green);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(controller_.GetStaticLight(i), 5, 5);
    }

    //LightSource
    painter.setBrush(Qt::red);
    painter.setPen(Qt::NoPen);
    int r = 5;
    for (int i = 0; i < 11; i++) {
        if (i) {
            r = 3;
        }
        painter.drawEllipse(controller_.GetLightSource() + delta[i] * Radius, r, r);
    }
}

void Canvas::PolygonsEvent() {
    if (is_left_) {
        QPointF pos = click_point_;
        last_vertex_ = pos;
        if (is_finished_) {
            is_finished_ = false;
            controller_.AddPolygon(Polygon({pos}));
        } else {
            controller_.AddVertexToLastPolygon(pos);
        }
    } else {
        CheckLastPolygon();
        is_finished_ = true;
    }
}

void Canvas::StaticLightEvent() {
    if (!is_left_) {
        return;
    }
    QPointF pos = click_point_;
    controller_.AddStaticLight(pos);
}

void Canvas::mousePressEvent(QMouseEvent* event) {
    click_point_ = event->position();
    is_left_ = event->button() == Qt::LeftButton;
    click_timer_->start(200);
}

void Canvas::Click() {
    if (is_edit_) {
        is_edit_ = false;
        update();
        return;
    }
    if (mode_ == Mode::StaticLight) {
        StaticLightEvent();
    } else if (mode_ == Mode::Polygons){
        PolygonsEvent();
    }
    update();
}

void Canvas::mouseDoubleClickEvent(QMouseEvent* event) {
    click_timer_->stop();
    if (mode_ == Mode::Polygons) {
        if (!controller_.IsValidUpdateVertex()) {
            return;
        }
        is_edit_ = true;
        QRectF sizes = rect();
        QPointF pos = event->position();

        cursor_point_.setX(qBound(2.0, pos.x(), sizes.width() - 2.0));
        cursor_point_.setY(qBound(2.0, pos.y(), sizes.height() - 2.0));

        last_vertex_ = cursor_point_;
        controller_.UpdateLastPolygon(cursor_point_);
    }

    update();
}


void Canvas::mouseMoveEvent(QMouseEvent* event) {
    QRectF sizes = rect();
    QPointF pos = event->position();

    cursor_point_.setX(qBound(2.0, pos.x(), sizes.width() - 2.0));
    cursor_point_.setY(qBound(2.0, pos.y(), sizes.height() - 2.0));

    if (is_edit_ && mode_ == Mode::Polygons) {
        last_vertex_ = cursor_point_;
        controller_.UpdateLastPolygon(cursor_point_);
    } else if (mode_ == Mode::Light){
        controller_.SetLightSource(cursor_point_);
    }

    update();
}

void Canvas::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Backspace) {
        if (mode_ == Mode::Polygons) {
            is_finished_ = true;
            controller_.RemoveLastPolygon();
        } else if(mode_ == Mode::StaticLight) {
            controller_.RemoveLastStaticLight();
        }
    } else if (event->key() == Qt::Key_Delete) {
        if (mode_ == Mode::Polygons) {
            is_finished_ = true;
            controller_.RemoveAllPolygons();
        } else if (mode_ == Mode::StaticLight) {
            controller_.RemoveAllStaticLights();
        }
    }
}
