#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "rtweekend.h"
#include "hittable.h"
#include "hittable_list.h"

class triangle : public hittable {
public:
    triangle(const point3& p1, const point3& p2, const point3& p3, const point2& t1, const point2& t2, const point2& t3, shared_ptr<material> mat)
      : p1(p1), p2(p2), p3(p3), t1(t1), t2(t2), t3(t3), mat(mat)
    {
        e1 = p2 - p1;
        e2 = p3 - p1;

        normal = unit_vector(cross(e1, e2));

        point3 min = point3(std::fmin(p1.x(), std::fmin(p2.x(), p3.x())),
                            std::fmin(p1.y(), std::fmin(p2.y(), p3.y())),
                            std::fmin(p1.z(), std::fmin(p2.z(), p3.z())));
        point3 max = point3(std::fmax(p1.x(), std::fmax(p2.x(), p3.x())),
                            std::fmax(p1.y(), std::fmax(p2.y(), p3.y())),
                            std::fmax(p1.z(), std::fmax(p2.z(), p3.z())));
        bbox = aabb(min, max);
    }

    aabb bounding_box() const override { return bbox; }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override
    {
        vec3 o = r.origin();
        vec3 d = r.direction();

        vec3 pvec = cross(d, e2);
        double det = dot(e1, pvec);

        if (std::fabs(det) < 1e-8)
            return false;

        double inv_det = 1.0 / det;

        vec3 tvec = o - p1;
        double u = dot(tvec, pvec) * inv_det;

        if (u < 0.0 || u > 1.0)
            return false;

        vec3 qvec = cross(tvec, e1);
        double v = dot(d, qvec) * inv_det;

        if (v < 0.0 || u + v > 1.0)
            return false;

        double t = dot(e2, qvec) * inv_det;

        if (!ray_t.contains(t))
            return false;
        
        rec.t = t;
        rec.p = r.at(rec.t);

        if(t1 == t2 && t2 == t3)
        {
            rec.u = u;
            rec.v = v;
        }
        else
        {
            double w = 1.0 - u - v;  // Third barycentric coordinate
    
            // Interpolate texture coordinates
            double tex_u = w * t1.u() + u * t2.u() + v * t3.u();
            double tex_v = w * t1.v() + u * t2.v() + v * t3.v();
    
            rec.u = tex_u;
            rec.v = tex_v;
        }
        
        rec.mat = mat;
        rec.set_face_normal(r, normal);

        return true;
    }

private:
    point3 p1, p2, p3;
    point2 t1, t2, t3;
    vec3 e1, e2;
    vec3 normal;
    shared_ptr<material> mat;
    aabb bbox;
};

#endif