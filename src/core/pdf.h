#ifndef PDF_H
#define PDF_H

#include "hittable_list.h"
#include "onb.h"

class pdf
{
public:
    virtual ~pdf() {}

    virtual double value(const vec3& direction) const = 0;
    virtual vec3 generate() const = 0;
};

class sphere_pdf : public pdf
{
public:
    sphere_pdf() {}

    double value(const vec3& direction) const override
    {
        return 1 / (4 * pi);
    }

    vec3 generate() const override
    {
        return random_unit_vector();
    }
};

class cosine_pdf : public pdf
{
public:
    cosine_pdf(const vec3& w) : uvw(w) {}

    double value(const vec3& direction) const override
    {
        auto cos_theta = dot(unit_vector(direction), uvw.w());
        return std::fmax(0, cos_theta / pi);
    }

    vec3 generate() const override
    {
        return uvw.transform(random_cos_direction());
    }

private:
    onb uvw;
};

class ggx_pdf : public pdf
{
public:
    ggx_pdf(const vec3& normal, const vec3& view_dir, double alpha_x, double alpha_y) 
        : uvw(normal), alpha_x(alpha_x), alpha_y(alpha_y)
    {
        // Transform view direction to local space (aligned with normal)
        wi_local = uvw.local(-view_dir);  // Negate because view_dir points toward surface
    }

    vec3 generate() const override
    {
        double r1 = random_double();
        double r2 = random_double();

        vec3 h_local = sampleGGXVNDF(r1, r2);

        double wi_dot_h = dot(wi_local, h_local);

        vec3 wo_local = 2.0 * wi_dot_h * h_local - wi_local;

        vec3 wo = uvw.world(wo_local);

        return wo;
    }

    double value(const vec3& direction) const override
    {
        vec3 wo_local = uvw.local(unit_vector(direction));

        vec3 h_local = unit_vector(wo_local + wi_local);

        double wi_dot_h = dot(wi_local, h_local);

        double D = ggx_D(h_local, alpha_x, alpha_y);
        double G = smith_G1(wi_local, alpha_x, alpha_y);
        double DV = D * G;

        double pdf_value = DV / (4.0 * wi_dot_h * wi_dot_h);

        return std::max(0.0, pdf_value);
    }
    
    // GGX normal distribution function
    static double ggx_D(const vec3& h, double alpha_x, double alpha_y)
    {
        double x_scale = (h.x() * h.x()) / (alpha_x * alpha_x);
        double y_scale = (h.y() * h.y()) / (alpha_y * alpha_y);
        double z_scale = h.z() * h.z();
    
        double xyz_sqr = (x_scale + y_scale + z_scale) * (x_scale + y_scale + z_scale);
        
        double denom = pi * alpha_x * alpha_y * xyz_sqr;
        return 1.0 / denom;
    }
    
    // Smith G1 term (masking function)
    static double smith_G1(const vec3& v, double alpha_x, double alpha_y)
    {
        double z_v = v.z();
        if (z_v <= 0.0) return 0.0;
        
        // Λ(V) = (-1 + √(1 + (α_x² x_v² + α_y² y_v²) / z_v²)) / 2
        double x_v = v.x();
        double y_v = v.y();
        double alpha_x_sq = alpha_x * alpha_x;
        double alpha_y_sq = alpha_y * alpha_y;
        
        double term = (alpha_x_sq * x_v * x_v + alpha_y_sq * y_v * y_v) / (z_v * z_v);
        double lambda = (-1.0 + std::sqrt(1.0 + term)) / 2.0;
        
        // G_1(V) = 1 / (1 + Λ(V))
        return 1.0 / (1.0 + lambda);
    }

private:
    onb uvw;
    vec3 wi_local;  // Incoming direction in local space
    double alpha_x;
    double alpha_y;

    // Eric Heitz: Sampling the GGX Distribution of Visible Normals
    vec3 sampleGGXVNDF(double r1, double r2) const
    {
        // Section 3.2: transforming the view direction to the hemisphere configuration
        vec3 Vh = unit_vector(vec3(alpha_x * wi_local.x(), alpha_y * wi_local.y(), wi_local.z()));
        
        // Section 4.1: orthonormal basis (with special case if cross product is zero)
        double lensq = Vh.x() * Vh.x() + Vh.y() * Vh.y();
        double inv_sqrt_lensq = lensq > 0 ? 1.0 / std::sqrt(lensq) : 0.0;
        vec3 T1 = lensq > 0 ? vec3(-Vh.y(), Vh.x(), 0) * inv_sqrt_lensq : vec3(1,0,0);
        vec3 T2 = cross(Vh, T1);
        
        // Section 4.2: parameterization of the projected area
        double r = std::sqrt(r1);
        double phi = 2.0 * pi * r2;
        double t1 = r * std::cos(phi);
        double t2 = r * std::sin(phi);
        double s = 0.5 * (1.0 + Vh.z());
        t2 = (1.0 - s) * std::sqrt(std::max(0.0, 1.0 - t1*t1)) + s * t2;
        
        // Section 4.3: reprojection onto hemisphere
        vec3 Nh = t1*T1 + t2*T2 + std::sqrt(std::max(0.0, 1.0 - t1*t1 - t2*t2)) * Vh;
        
        // Section 3.4: transforming the normal back to the ellipsoid configuration
        vec3 Ne = unit_vector(vec3(alpha_x * Nh.x(), alpha_y * Nh.y(), std::max(0.0, Nh.z())));
        return Ne;
    }

};

class hittable_pdf : public pdf
{
public:
    hittable_pdf(const hittable& object, const point3& origin)
    : object(object), origin(origin)
    {}

    double value(const vec3& direction) const override 
    {
        return object.pdf_value(origin, direction);
    }

    vec3 generate() const override
    {
       return object.random(origin);
    }

private:
    const hittable& object;
    point3 origin;
};

class mixture_pdf : public pdf
{
public:
    mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1)
    {
        p[0] = p0;
        p[1] = p1;
    }

    double value(const vec3& direction) const override
    {
        return 0.5 * p[0]->value(direction) + 0.5 * p[1]->value(direction);
    }

    vec3 generate() const override {
        if (random_double() < 0.5)
            return p[0]->generate();
        else
            return p[1]->generate();
    }

private:
    shared_ptr<pdf> p[2];
};

#endif