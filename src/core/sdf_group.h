#ifndef SDF_GROUP_H
#define SDF_GROUP_H

#define MAX_STEPS 100
#define MAX_DEPTH 100
#define EPSILON 0.001

#include "aabb.h"
#include "hittable.h"
#include "sdsphere.h"

class sdf_group : public hittable {
public:
    std::vector<shared_ptr<sdsphere>> objects;

    sdf_group(){};
    sdf_group(shared_ptr<sdsphere> object) { add(object); }

    void add(shared_ptr<sdsphere> object)
    {
        objects.push_back(object);
        bbox = aabb(bbox, object->bounding_box());
    }

    bool hit(const ray &r, interval ray_t, hit_record &rec) const override
    {
        double t = 0.0;
        shared_ptr<sdsphere> closest_sphere = nullptr;

        vec3 dir = unit_vector(r.direction());
        for(int i = 0; i < MAX_STEPS; i++)
        {
            point3 p = r.origin() + t * dir;
            double dist = min_sdf(p, closest_sphere);

            if (dist < EPSILON && ray_t.contains(t))
            {
                if(closest_sphere == nullptr)
                    return false;

                rec.t = t;
                rec.p = r.at(rec.t);
                vec3 outward_normal = calculate_normal(p);
                rec.set_face_normal(r, outward_normal);
                closest_sphere->get_sphere_uv(outward_normal, rec.u, rec.v);
                rec.mat = closest_sphere->get_material();
                return true;
            }

            t += dist;

            if (t > MAX_DEPTH)
                return false;
        }
        return false;
    }

    aabb bounding_box() const override
    {
        return bbox;
    }


private:
    aabb bbox;

    double min_sdf(point3 p, shared_ptr<sdsphere>& closest_sphere) const
    {
        auto dist = MAX_DEPTH;
        double blend_factor = 0.1;
        double min_individual = MAX_DEPTH;
        for(const auto &object: objects)
        {
            auto dist2 = object->sdf(p);

            if(dist2 < min_individual)
            {
                min_individual = dist2;
                closest_sphere = object;
            }

            dist = smooth_min(dist, dist2, blend_factor);
        }

        return dist;
    }

    double clamp(double x, double min_val, double max_val) const
    {
        return std::max(min_val, std::min(x, max_val));
    }

    double smooth_min(double a, double b, double k) const
    {
        auto h = clamp(0.5 + 0.5 * (b - a) / k, 0.0, 1.0);
        return mix(b, a, h) - k * h * (1.0 - h);
    } 

    double mix(double a, double b, double t) const
    {
        return a * (1.0 - t) + b * t;
    }

    vec3 calculate_normal(const point3& p) const {
        const double h = 0.0001;
        shared_ptr<sdsphere> dummy = nullptr;
        return unit_vector(vec3(
            min_sdf(p + vec3(h, 0, 0), dummy) - min_sdf(p - vec3(h, 0, 0), dummy),
            min_sdf(p + vec3(0, h, 0), dummy) - min_sdf(p - vec3(0, h, 0), dummy),
            min_sdf(p + vec3(0, 0, h), dummy) - min_sdf(p - vec3(0, 0, h), dummy)
        ));
    }
};

#endif