#include "polygon.h"

Polygon::Polygon(const std::vector<QPointF>& vertices) : verticies_(vertices) {
}

QPointF& Polygon::operator[](int ind) {
    return verticies_[ind];
}

const QPointF& Polygon::operator[](int ind) const {
    return verticies_[ind];
}

Polygon& Polygon::operator=(const Polygon& other) {
    if (this != &other) {
        this->verticies_ = other.verticies_;
    }
    return *this;
}

const std::vector<QPointF>& Polygon::GetVerticies() const{
    return verticies_;
}

void Polygon::AddVertex(const QPointF& vertex) {
    if (!verticies_.empty() && verticies_.back() == vertex) {
        return;
    }
    verticies_.push_back(vertex);
}

void Polygon::UpdateLastVertex(const QPointF& new_vertex) {
    verticies_.back() = new_vertex;
}

bool Polygon::DeleteLastVertex() {
    verticies_.pop_back();
    return verticies_.empty();
}

std::optional<QPointF> Polygon::IntersectRay(const Ray& ray) {
    if (verticies_.size() <= 1) {
        return std::nullopt;
    }
    int best_ind = 0;
    double mn = 2e9;
    QPointF begin1 = ray.GetBegin();
    QPointF dir1 = ray.GetEnd() - ray.GetBegin();
    for (int i = 0; i < verticies_.size(); i++) {
        QPointF begin2 = verticies_[i];
        QPointF end2 = verticies_[(i + 1) % verticies_.size()];
        QPointF dir2 = end2 - begin2;
        double num = dir2.y() * (begin2.x() - begin1.x()) - dir2.x() * (begin2.y() - begin1.y());
        double denom = dir1.x() * dir2.y() - dir1.y() * dir2.x();
        double t1;
        double t2;
        if (std::abs(denom) < kEps) {
            continue;
        }
        t1 = num / denom;
        if (dir2.y() != 0.0) {
            t2 = (begin1.y() + dir1.y() * t1 - begin2.y()) / dir2.y();
        } else {
            t2 = (begin1.x() + dir1.x() * t1 - begin2.x()) / dir2.x();
        }
        if (t1 < -kEps || t2 < -kEps || t2 > 1.0 + kEps) {
            continue;
        }
        mn = std::min(mn, t1);
    }
    if (mn == 2e9) {
        return std::nullopt;
    }
    return std::optional<QPointF>(QPointF(begin1.x() + mn * dir1.x(), begin1.y() + mn * dir1.y()));
}
