#include "geometry.h"


double Geometry::Distanse(const Geometry::Coordinates& lhs, const Geometry::Coordinates& rhs) {
    return acos(sin(lhs.lat_) * sin(rhs.lat_) +
        cos(lhs.lat_) * cos(rhs.lat_) *
        cos(fabs(lhs.lon_ - rhs.lon_))
    ) * 6371000;
}

