#pragma once

#include "ray.h"
#include <QPointF>
#include <vector>

class Polygon {
public:
    Polygon(const std::vector<QPointF>& vertices);

    QPointF& operator[](int ind);

    const QPointF& operator[](int ind) const;

    Polygon& operator=(const Polygon& other);

    const std::vector<QPointF>& GetVerticies() const;

    void AddVertex(const QPointF& vertex);

    void UpdateLastVertex(const QPointF& new_vertex);

    bool DeleteLastVertex();

    std::optional<QPointF> IntersectRay(const Ray& ray);

private:
    std::vector<QPointF> verticies_;
};
