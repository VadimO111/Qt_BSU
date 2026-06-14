#include "controller.h"
#include <algorithm>

Polygon& Controller::operator[](int ind) {
    return polygons_[ind];
}

const Polygon& Controller::operator[](int ind) const {
    return polygons_[ind];
}

const std::vector<Polygon>& Controller::GetPolygons() {
    return polygons_;
}

void Controller::AddPolygon(const Polygon& polygon) {
    polygons_.push_back(polygon);
}

void Controller::AddVertexToLastPolygon(const QPointF& new_vertex) {
    polygons_.back().AddVertex(new_vertex);
}

void Controller::UpdateLastPolygon(const QPointF& new_vertex) {
    polygons_.back().UpdateLastVertex(new_vertex);
}

void Controller::DeleteLastVertex() {
    if (polygons_.back().DeleteLastVertex()) {
        DeleteLastPolygon();
    }
}

void Controller::DeleteLastPolygon() {
    if (polygons_.empty()) {
        return;
    }
    polygons_.pop_back();
}

bool Controller::IsValidUpdateVertex() const {
    return polygons_.size() > 1;
}

const QPointF& Controller::GetLightSource() {
    return light_source_;
}

void Controller::SetLightSource(const QPointF& light_source) {
    light_source_ = light_source;
}

std::vector<Ray> Controller::CastRays() {
    std::vector<Ray> rays;
    for (const Polygon& polygon : polygons_) {
        for (const QPointF vertex : polygon.GetVerticies()) {
            double angle;
            double dx = vertex.x() - cur_light_source_.x();
            double dy = vertex.y() - cur_light_source_.y();
            angle = atan2(dy, dx);
            QPointF far_vertex = QPointF(cur_light_source_.x() + cos(angle) * 100000, cur_light_source_.y() + sin(angle) * 100000);
            Ray ray = Ray(cur_light_source_, far_vertex, angle);
            rays.push_back(ray.Rotate(-0.0001));
            rays.push_back(ray);
            rays.push_back(ray.Rotate(0.0001));
        }
    }
    return rays;
}

void Controller::IntersectRays(std::vector<Ray>* rays) {
    for (std::vector<Ray>::iterator it = rays->begin(); it != rays->end(); it++) {
        QPointF best_end = it->GetEnd();
        for (Polygon& polygon : polygons_) {
            std::optional<QPointF> new_end = polygon.IntersectRay(*it);
            if (new_end == std::nullopt) {
                continue;
            }
            if (Distance(it->GetBegin(), new_end.value()) < Distance(it->GetBegin(), best_end)) {
                best_end = new_end.value();
            }
        }
        *it = Ray(it->GetBegin(), best_end, it->GetAngle());
    }
}

void Controller::RemoveAdjacentRays(std::vector<Ray>* rays) {
    size_t sz = rays->size();
    if (sz <= 1) {
        return;
    }
    std::sort(rays->begin(), rays->end(), [](const Ray& a, const Ray& b){return a.GetAngle() < b.GetAngle();});
    std::vector<bool> chosen(sz, true);
    double kMinDistance = 1e-5;
    std::vector<Ray>::iterator it = rays->begin();
    std::vector<Ray>::iterator prev = rays->begin();
    for (int i = 0; i < rays->size() - 1; i++) {
        if (chosen[i]) {
            prev = it;
        }
        if (Distance(prev->GetEnd(), std::next(it)->GetEnd()) < kMinDistance) {
            chosen[i + 1] = false;
        }
        std::advance(it, 1);
    }
    int cnt = 0;
    it = rays->begin();
    std::vector<Ray>::iterator it1 = rays->begin();
    for (int i = 0; i < rays->size(); i++) {
        if (chosen[i]) {
            cnt++;
            *it = *it1;
            std::advance(it, 1);
        }
        std::advance(it1, 1);
    }
    while (rays->size() > cnt) {
        rays->pop_back();
    }
}

Polygon Controller::CreateLightArea(int ind, int width, int height) {
    cur_light_source_ = light_source_ + delta[ind] * Radius;
    cur_light_source_.setX(qBound(2.0, cur_light_source_.x(), width - 2.0));
    cur_light_source_.setY(qBound(2.0, cur_light_source_.y(), height - 2.0));
    std::vector<Ray> rays = CastRays();
    IntersectRays(&rays);
    RemoveAdjacentRays(&rays);
    std::vector<QPointF> verticies;
    for (const Ray& ray : rays) {
        verticies.push_back(ray.GetEnd());
    }
    return Polygon(verticies);
}

Polygon Controller::CreateStaticLightArea(int ind, int width, int height) {
    cur_light_source_ = static_lights_[ind];
    cur_light_source_.setX(qBound(2.0, cur_light_source_.x(), width - 2.0));
    cur_light_source_.setY(qBound(2.0, cur_light_source_.y(), height - 2.0));
    std::vector<Ray> rays = CastRays();
    IntersectRays(&rays);
    RemoveAdjacentRays(&rays);
    std::vector<QPointF> verticies;
    for (const Ray& ray : rays) {
        verticies.push_back(ray.GetEnd());
    }
    return Polygon(verticies);
}

void Controller::AddStaticLight(const QPointF light) {
    static_lights_.push_back(light);
}

QPointF Controller::GetStaticLight(int ind) {
    if (ind < 0 || ind >= static_lights_.size()) {
        return QPointF(-1, -1);
    }
    return static_lights_[ind];
}

size_t Controller::LightSize() const {
    return static_lights_.size();
}

std::vector<Ray> Controller::GetDebugRays(int ind, int width, int height) {
    cur_light_source_ = light_source_ + delta[ind] * Radius;
    cur_light_source_.setX(qBound(2.0, cur_light_source_.x(), width - 2.0));
    cur_light_source_.setY(qBound(2.0, cur_light_source_.y(), height - 2.0));
    std::vector<Ray> rays = CastRays();
    IntersectRays(&rays);
    RemoveAdjacentRays(&rays);
    return rays;
}

void Controller::RemoveLastPolygon() {
    if (!polygons_.empty()) {
        polygons_.pop_back();
    }
}

void Controller::RemoveLastStaticLight() {
    if (!static_lights_.empty()) {
        static_lights_.pop_back();
    }
}

void Controller::RemoveAllPolygons() {
    polygons_.clear();
}

void Controller::RemoveAllStaticLights() {
    static_lights_.clear();
}
