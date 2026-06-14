#include "ray.h"

Ray::Ray(const QPointF& begin, const QPointF& end, double angle) : begin_(begin), end_(end), angle_(angle) {
}

QPointF Ray::GetBegin() const {
    return begin_;
}

QPointF Ray::GetEnd() const {
    return end_;
}

double Ray::GetAngle() const {
    return angle_;
}

Ray Ray::Rotate(double angle) const {
    QPointF delta = end_ - begin_;
    const QPointF matrix[2] = {QPointF(cos(angle), -sin(angle)), QPointF(sin(angle), cos(angle))};
    delta = QPointF(QPointF::dotProduct(matrix[0], delta), QPointF::dotProduct(matrix[1], delta));
    return Ray(begin_, begin_ + delta, angle_ + angle);
}

bool IsCollinear(const Ray& a, const Ray& b) {
    return (std::abs(sin(a.GetAngle()) - sin(b.GetAngle())) < kEps && std::abs(cos(a.GetAngle()) - cos(b.GetAngle())) < kEps);
}

double Distance(const QPointF& p1, const QPointF& p2) {
    return std::sqrt((p2.x() - p1.x()) * (p2.x() - p1.x()) + (p2.y() - p1.y()) * (p2.y() - p1.y()));
}
