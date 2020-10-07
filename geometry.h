#pragma once
#include <cmath>

const double PI = 3.1415926535;

namespace Geometry {
    struct Coordinates {
        double lat_, lon_;

        Coordinates() = default;

        Coordinates(double latitude, double longitude) :
            lat_(latitude* PI / 180.0),
            lon_(longitude* PI / 180.0)
        {
        }

    };

    double Distanse(const Coordinates& lhs, const Coordinates& rhs);

} //end of namespace Geometry

