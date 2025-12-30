#ifndef ONB_h
#define ONB_h

#include "rtweekend.h"

class onb 
{
public:
    onb(const vec3& n)
    {
        axis[2] = unit_vector(n);
        vec3 a = (std::fabs(n.x()) > 0.9) ? vec3(0,1,0) : vec3(1,0,0);
        axis[1] = unit_vector(cross(axis[2], a));
        axis[0] = cross(axis[2], axis[1]);
    }

    const vec3& u() const { return axis[0]; }
    const vec3& v() const { return axis[1]; }
    const vec3& w() const { return axis[2]; }

    vec3 transform(const vec3& v) const
    {
        return (v[0] * axis[0]) + (v[1] * axis[1]) + (v[2] * axis[2]);
    }
    
    // Transform from world space to local space
    vec3 local(const vec3& v) const
    {
        return vec3(dot(v, axis[0]), dot(v, axis[1]), dot(v, axis[2]));
    }
    
    // Transform from local space to world space (same as transform)
    vec3 world(const vec3& v) const
    {
        return transform(v);
    }
    
private:
    vec3 axis[3];
};

#endif