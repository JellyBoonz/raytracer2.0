#ifndef SDSPHERE_H
#define SDSPHERE_H

#define MAX_STEPS 100
#define MAX_DEPTH 100

#include "hittable.h"

class sdsphere : public hittable {
public:
    sdsphere(const point3 &center, double radius, shared_ptr<material> mat)
        : center(center), radius(std::fmax(0, radius)), mat(mat)
    {
        auto rvec = vec3(radius, radius, radius);
        bbox = aabb(center - rvec, center + rvec);
    }

    bool hit(const ray &r, interval ray_t, hit_record &rec) const override 
    {
        double t = 0.0;
        
        for(int i = 0; i < MAX_STEPS; i++)
        {
            point3 p = r.origin() + t * unit_vector(r.direction());
            double dist = sdf(p);

            if (dist < e && ray_t.contains(t))
            {
                rec.t = t;
                rec.p = r.at(rec.t);
                vec3 outward_normal = get_normal(rec.p);
                rec.set_face_normal(r, outward_normal);
                get_sphere_uv(outward_normal, rec.u, rec.v);
                rec.mat = mat;
                return true;
            }

            t += dist;

            if(t > MAX_DEPTH)
                return false;
        }

        return false;
    }
    
    double sdf(point3 p) const
    {
        return (p - center).length() - radius;
    }

    shared_ptr<material> get_material() 
    {
        return mat;
    }

    vec3 get_normal(point3 p) const
    {
       return (p - center) / radius; 
    }
 
    static void get_sphere_uv(const point3 &p, double &u, double &v)
    {
        // p: a given point on the sphere of radius one, centered at the origin.
        // u: returned value [0,1] of angle around the Y axis from X=-1.
        // v: returned value [0,1] of angle from Y=-1 to Y=+1.
        //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
        //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
        //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>
        auto theta = std::acos(-p.y());
        auto phi = std::atan2(p.z(), -p.x()) + pi;
    
        u = phi / (2 * pi);
        v = theta / pi;
    }
    
    aabb bounding_box() const override { return bbox; }
private:
    point3 center;
    double radius;
    shared_ptr<material> mat;
    aabb bbox;
    double e = 0.001;


};

#endif