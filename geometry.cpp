#include "geometry.h"


double Geometry::Distanse(const Geometry::Coordinates& p1, const Geometry::Coordinates& p2) {
    return acos(sin(p1.lat_) * sin(p2.lat_) +
        cos(p1.lat_) * cos(p2.lat_) *
        cos(fabs(p1.lon_ - p2.lon_))
    ) * 6371000;
}

