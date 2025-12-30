#ifndef MATERIAL_H
#define MATERIAL_H

#include <cstdio>
#include <ctime>
#include "rtweekend.h"
#include "hittable.h"
#include "pdf.h"
#include "texture.h"
#include "onb.h"

class scatter_record {
  public:
    color attenuation;
    shared_ptr<pdf> pdf_ptr;
    bool skip_pdf;
    ray skip_pdf_ray;
};

class material
{
public:
    virtual ~material() = default;

    virtual color emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p) const {
        return color(0,0,0);
    }

    virtual bool scatter(
        const ray &r_in, const hit_record &rec, scatter_record& srec) const
    {
        return false;
    }

    virtual double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const
    {
        return 0;
    }

    virtual color eval_brdf(const ray& r_in, const hit_record& rec, const ray& scattered) const
    {
        // Default implementation for backward compatibility
        // Returns attenuation * scattering_pdf for simple materials
        scatter_record srec;
        if (scatter(r_in, rec, srec)) {
            return srec.attenuation * scattering_pdf(r_in, rec, scattered);
        }
        return color(0, 0, 0);
    }

    virtual bool use_light_sampling() const
    {
        // Return true to use mixture PDF (light + material), false to use only material PDF
        return true;
    }
};

class lambertian : public material
{
public:
    lambertian(const color &albedo) : tex(make_shared<solid_color>(albedo)) {}
    lambertian(shared_ptr<texture> tex) : tex(tex) {}

    bool scatter(const ray &r_in, const hit_record &rec, scatter_record& srec)
        const override
    {
        srec.attenuation = tex->value(rec.u, rec.v, rec.p);
        srec.pdf_ptr = make_shared<cosine_pdf>(rec.normal);
        srec.skip_pdf = false;
        return true;
    }

    double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const override
    {
        auto cos_theta = dot(rec.normal, unit_vector(scattered.direction()));
        return cos_theta < 0 ? 0 : cos_theta/pi;
    }

private:
    shared_ptr<texture> tex;
};

class metal : public material
{
public:
    metal(const color &albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const ray &r_in, const hit_record &rec, scatter_record& srec)
        const override
    {
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        reflected = unit_vector(reflected) + (fuzz * random_unit_vector());

        srec.attenuation = albedo;
        srec.pdf_ptr = nullptr;
        srec.skip_pdf = true;
        srec.skip_pdf_ray = ray(rec.p, reflected, r_in.time());

        return true;
    }

private:
    color albedo;
    double fuzz;
};

class dielectric : public material
{
public:
    dielectric(double refraction_index) : refraction_index(refraction_index) {}

    bool scatter(const ray &r_in, const hit_record &rec, scatter_record& srec)
        const override
    {
        srec.attenuation = color(1.0, 1.0, 1.0);
        srec.pdf_ptr = nullptr;
        srec.skip_pdf = true;
        double ri = rec.front_face ? (1.0 / refraction_index) : refraction_index;

        vec3 unit_direction = unit_vector(r_in.direction());
        double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = ri * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, ri) > random_double())
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, ri);

        srec.skip_pdf_ray = ray(rec.p, direction, r_in.time());
        return true;
    }

private:
    // Refractive index in vacuum or air, or the ratio of the material's refractive index over
    // the refractive index of the enclosing media
    double refraction_index;

    static double reflectance(double cosine, double refraction_index)
    {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1 - refraction_index) / (1 + refraction_index);
        r0 = r0 * r0;
        return r0 + (1 - r0) * std::pow((1 - cosine), 5);
    }
};

class iridescent : public material 
{
public:
    iridescent(std::shared_ptr<material> base, double strength)
        : base(base), strength(strength) {}

    virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override
    {
        if (!base->scatter(r_in, rec, srec)) return false;
        
        color base_atten = srec.attenuation;
        
        vec3 view_dir = unit_vector(-r_in.direction());
        double cos_theta = dot(view_dir, rec.normal);

        color iri = iridescent_color(cos_theta);
        srec.attenuation = (1.0 - strength) * base_atten + strength * iri;
        return true;
    }
    

private:
    std::shared_ptr<material> base;
    double strength;

    color iridescent_color(double cos_theta) const
    {
        double x = 1 - cos_theta;

        double k = 6.0;      // number of bands
        double phase = k*x;  // number of bands proportional to angle

        double fr = 1.0;
        double fg = 1.3;
        double fb = 1.7;

        double R = 0.5 * (1.0 + cos(fr * phase));
        double G = 0.5 * (1.0 + cos(fg * phase));
        double B = 0.5 * (1.0 + cos(fb * phase));

        return color(R,G,B);
    }
};

class diffuse_light : public material
{
public:
    diffuse_light(shared_ptr<texture> tex) : tex(tex) {}
    diffuse_light(const color& emit) : tex(make_shared<solid_color>(emit)) {};

    color emitted(const ray& r_in, const hit_record& rec, double u, double v, const point3& p)
    const override {
        if (!rec.front_face)
            return color(0,0,0);
        return tex->value(u, v, p);
    }

private:
    shared_ptr<texture> tex;
};

class isotropic : public material
{
public:
    isotropic(const color& albedo) : tex(make_shared<solid_color>(albedo)) {}
    isotropic(shared_ptr<texture> tex) : tex(tex) {}

    bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) const override
    {
        srec.attenuation = tex->value(rec.u, rec.v, rec.p);
        srec.pdf_ptr = make_shared<sphere_pdf>();
        srec.skip_pdf = false;
        return true;
    }

    double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered)
    const override {
        return 1 / (4 * pi);
    }

private:
    shared_ptr<texture> tex;
};

class glossy : public material
{
public:
    glossy(const color& albedo, double roughness, double metallic = 0.0)
        : albedo(albedo), roughness(roughness), metallic(metallic)
    {
        alpha = roughness * roughness;
    }

    bool scatter(const ray& r_in, const hit_record& rec, scatter_record& srec) 
        const override
    {
        srec.pdf_ptr = make_shared<ggx_pdf>(rec.normal, r_in.direction(), alpha, alpha);
        srec.skip_pdf = false;
        srec.attenuation = color(1.0,1.0,1.0);
        return true;
    }

    double scattering_pdf(const ray& r_in, const hit_record& rec, const ray& scattered) const override
    {
        auto pdf = make_shared<ggx_pdf>(rec.normal, r_in.direction(), alpha, alpha);
        return pdf->value(scattered.direction());
    }

    bool use_light_sampling() const override
    {
        // Glossy materials use only BRDF importance sampling, not light sampling
        return false;
    }

    color eval_brdf(const ray& r_in, const hit_record& rec, const ray& scattered) const override
    {
        vec3 wi = unit_vector(-r_in.direction());  // Incoming direction (pointing away from surface)
        vec3 wo = unit_vector(scattered.direction());  // Outgoing direction
        vec3 n = rec.normal;
        
        return eval_brdf_impl(wi, wo, n);
    }

private:
    color albedo;
    double roughness;
    double alpha;  // roughness^2
    double metallic;  // 0 = dielectric, 1 = metal

    static double reflectance(double cosine, double refraction_index)
    {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1 - refraction_index) / (1 + refraction_index);
        r0 = r0 * r0;
        return r0 + (1 - r0) * std::pow((1 - cosine), 5);
    }

    // Fresnel-Schlick approximation
    color fresnel_schlick(double cos_theta, const color& F0) const
    {
        return F0 + (color(1.0, 1.0, 1.0) - F0) * std::pow(1.0 - cos_theta, 5.0);
    }

    // Evaluate full Cook-Torrance BRDF (internal implementation)
    color eval_brdf_impl(const vec3& wi, const vec3& wo, const vec3& n) const
    {
        onb uvw(n);
        vec3 wi_local = uvw.local(wi);
        vec3 wo_local = uvw.local(wo);

        // Check if directions are above surface
        if (wo_local.z() <= 0.0 || wi_local.z() <= 0.0) {
            return color(0, 0, 0);
        }

        // Compute half-vector
        vec3 h_local = unit_vector(wi_local + wo_local);
        if (h_local.z() <= 0.0) {
            return color(0, 0, 0);
        }

        double wi_dot_h = dot(wi_local, h_local);
        if (wi_dot_h <= 0.0) {
            return color(0, 0, 0);
        }

        // Evaluate D term
        double D = ggx_pdf::ggx_D(h_local, alpha, alpha);

        // Evaluate G term (full geometry term: G1(wi) * G1(wo))
        double G = ggx_pdf::smith_G1(wi_local, alpha, alpha) * 
                   ggx_pdf::smith_G1(wo_local, alpha, alpha);

        // Compute F0 (base reflectance)
        color F0 = color(0.04, 0.04, 0.04);  // Dielectric
        if (metallic > 0.0) {
            // For metals, interpolate between dielectric and metal based on metallic parameter
            F0 = (1.0 - metallic) * F0 + metallic * albedo;
        }

        // Evaluate Fresnel term
        color F = fresnel_schlick(wi_dot_h, F0);
        double wi_dot_n = wi_local.z();
        double wo_dot_n = wo_local.z();

        // Prevent division by very small numbers to avoid emissive pixels
        const double min_dot = 1e-6;
        wi_dot_n = std::max(wi_dot_n, min_dot);
        wo_dot_n = std::max(wo_dot_n, min_dot);

        color specular = (D * G * F) / (4.0 * wi_dot_n * wo_dot_n);
    
        color result;
        if (metallic < 1.0) {
            color kd = (color(1.0, 1.0, 1.0) - F) * (1.0 - metallic);
            color diffuse = kd * albedo / pi;
            result = diffuse + specular;
        } else {
            result = specular;
        }

        return result;
    }
};

#endif