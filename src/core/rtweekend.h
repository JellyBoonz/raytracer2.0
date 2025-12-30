#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <thread>

// C++ Std Usings

using std::make_shared;
using std::shared_ptr;

// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

inline double degrees_to_radians(double degrees)
{
    return degrees * pi / 180.0;
}

inline double random_double()
{
    thread_local static std::mt19937 generator(std::random_device{}());
    thread_local static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(generator);
}

inline double random_double(double min, double max)
{
    thread_local static std::mt19937 generator(std::random_device{}());
    thread_local static std::uniform_real_distribution<double> distribution(min, max);
    return distribution(generator);
}

inline int random_int(int min, int max)
{
    return int(random_double(min, max + 1));
}


// Common Headers

#include "color.h"
#include "interval.h"
#include "ray.h"
#include "vec3.h"
#include "vec2.h"

inline vec3 random_cos_direction()
{
    auto r1 = random_double();
    auto r2 = random_double();

    auto phi = 2*pi*r1;
    auto x = std::cos(phi) * std::sqrt(r2);
    auto y = std::sin(phi) * std::sqrt(r2);
    auto z = std::sqrt(1-r2);

    return vec3(x, y, z);
}

#endif