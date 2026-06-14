#pragma once

#include "polygon.h"
#include <vector>

const int Radius = 15;
const QPointF delta[11] = {
    QPointF(0.0000, 0.0000),
    QPointF(1.0000, 0.0000),
    QPointF(0.8090, 0.5878),
    QPointF(0.3090, 0.9511),
    QPointF(-0.3090, 0.9511),
    QPointF(-0.8090, 0.5878),
    QPointF(-1.0000, 0.0000),
    QPointF(-0.8090, -0.5878),
    QPointF(-0.3090, -0.9511),
    QPointF(0.3090, -0.9511),
    QPointF(0.8090, -0.5878)
};

class Controller {
public:
    Polygon& operator[](int ind);

    const Polygon& operator[](int ind) const;

    const std::vector<Polygon>& GetPolygons();

    void AddPolygon(const Polygon& polygon);

    void AddVertexToLastPolygon(const QPointF& new_vertex);

    void UpdateLastPolygon(const QPointF& new_vertex);

    void DeleteLastVertex();

    void DeleteLastPolygon();

    bool IsValidUpdateVertex() const;

    const QPointF& GetLightSource();

    void SetLightSource(const QPointF& light_source);

    std::vector<Ray> CastRays();

    void IntersectRays(std::vector<Ray>* rays);

    void RemoveAdjacentRays(std::vector<Ray>* rays);

    Polygon CreateLightArea(int ind, int width, int height);

    Polygon CreateStaticLightArea(int ind, int width, int height);

    void AddStaticLight(const QPointF light);

    QPointF GetStaticLight(int ind);

    size_t LightSize() const;

    std::vector<Ray> GetDebugRays(int ind, int width, int height);

    void RemoveLastPolygon();

    void RemoveLastStaticLight();

    void RemoveAllPolygons();

    void RemoveAllStaticLights();

private:
    QPointF cur_light_source_ = QPointF(2, 2);
    QPointF light_source_ = QPointF(2, 2);
    std::vector<QPointF> static_lights_;
    std::vector<Polygon> polygons_;
};
