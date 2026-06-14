#pragma once

#include <QPointF>

const double kEps = 1e-9;

double Distance(const QPointF& p1, const QPointF& p2);

class Ray {
public:
    Ray(const QPointF& begin, const QPointF& end, double angle);

    QPointF GetBegin() const;

    QPointF GetEnd() const;

    double GetAngle() const;

    Ray Rotate(double angle) const;

    bool IsCollinear(const Ray& a, const Ray& b);

private:
    QPointF begin_;
    QPointF end_;
    double angle_;
};

