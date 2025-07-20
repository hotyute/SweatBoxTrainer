#include "airport.h"
#include "tools.h"
#include "geoutils.h"
#include <algorithm>

Point2* TaxiPath::getPrevPoint(Point2* to, Point2* next) {
    if (!to || !next) return nullptr;
    if (to->index < next->index && to->index > 0)
        return points[to->index - 1].get();
    if (to->index > next->index && to->index + 1 < static_cast<int>(points.size()))
        return points[to->index + 1].get();
    return nullptr;
}

Point2* TaxiPath::getNextPoint(Point2* p, Point2* p2) {
    if (!p || !p2 || p->index < 0 || p2->index < 0) return p2;
    if (p->index < p2->index) {
        for (int i = p->index + 1; i < p2->index; ++i)
            if (auto& pt = points[i]; pt && pt.get() != p) return pt.get();
    }
    else if (p->index > p2->index) {
        for (int i = p->index - 1; i > p2->index; --i)
            if (auto& pt = points[i]; pt && pt.get() != p) return pt.get();
    }
    return p2;
}

void TaxiPath::getPoints(Point2* p, Point2* p2, std::vector<Point2*>& out) {
    if (!p || !p2 || p->index < 0 || p2->index < 0) return;
    if (p->index < p2->index) {
        for (int i = p->index + 1; i < p2->index; ++i)
            out.push_back(points[i].get());
    }
    else if (p->index > p2->index) {
        for (int i = p->index - 1; i > p2->index; --i)
            out.push_back(points[i].get());
    }
}

Point2* TaxiPath::angleTest(const Point2& orig, Point2& p, Point2& p2) {
    Point2* f = &p;
    while (isHeavyAngle(orig, p, p2)) {
        if (f == &p2) break;
        f = getNextPoint(f, &p2);
    }
    return f;
}

Point2* TaxiPath::getClosestPoint(double latitude, double longitude) {
    Point2 probe(longitude, latitude);
    Point2* best = nullptr;
    double bestDist = -1;
    for (auto& pt : points) {
        if (!pt) continue;
        double d = dist(probe.y_, probe.x_, pt->y_, pt->x_);
        if (bestDist < 0 || d < bestDist) { bestDist = d; best = pt.get(); }
    }
    return best;
}

Point2* TaxiPath::getClosest(TaxiPath* next) {
    if (!next) return nullptr;
    Point2* best = nullptr;
    double bestDist = -1;
    for (auto& np : next->points) {
        if (!np) continue;
        for (auto& tp : points) {
            if (!tp || *tp == *np) continue;
            double d = dist(np->y_, np->x_, tp->y_, tp->x_);
            if (bestDist < 0 || d < bestDist) { bestDist = d; best = tp.get(); }
        }
    }
    return best;
}

Point2* TaxiPath::intersect(Point2& p) {
    for (auto& tp : points)
        if (tp && *tp != p && taxiIntersect(p, *tp))
            return tp.get();
    return nullptr;
}

Point2* TaxiPath::getStart() { return points.empty() ? nullptr : points.front().get(); }
Point2* TaxiPath::getEnd() { return points.empty() ? nullptr : points.back().get(); }