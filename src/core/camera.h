#ifndef CAMERA_H
#define CAMERA_H

#include <algorithm>
#include <atomic>
#include <vector>
#include <cstdio>
#include <ctime>

#include "hittable.h"
#include "pdf.h"
#include "material.h"

class camera
{
public:
    double ar = 1.0; // Ratio of image width over height
    int width = 100; // Rendered image width in pixel count
    int samples_per_pixel = 10;
    int max_depth = 10;
    color  background;

    bool vignette = false;

    double vfov = 90;
    point3 lookfrom = point3(0, 0, 0);
    point3 lookat = point3(0, 0, -1);
    vec3 vup = vec3(0, 1, 0);

    double defocus_angle = 0; // Variation angle of rays through each pixel
    double focus_dist = 10;   // Distance from camera lookfrom point to plane of perfect focus

    void render(const hittable &world, const hittable& lights)
    {
        initialize();

        std::vector<std::vector<color>> pixel_colors(height, std::vector<color>(width));

        const int num_threads = std::thread::hardware_concurrency();
        std::vector<std::thread> threads;
        std::atomic<int> scanlines_completed{0};

        auto center_x = width / 2;
        auto center_y = height / 2;

        // Launch threads with chunk-based work distribution for better cache locality
        const int chunk_size = (height + num_threads - 1) / num_threads;
        for (int t = 0; t < num_threads; ++t)
        {
            threads.emplace_back([this, &world, &lights, &pixel_colors, &scanlines_completed, chunk_size, num_threads, t, center_x, center_y]()
                                 {
                // Each thread processes a chunk of consecutive rows
                int start_row = t * chunk_size;
                int end_row = std::min(start_row + chunk_size, height);
                for (int j = start_row; j < end_row; j++)
                {
                    for (int i = 0; i < width; i++)
                    {
                        color pixel_color(0, 0, 0);
                        for (int s_i = 0; s_i < sqrt_spp; s_i++)
                        {
                            for (int s_j = 0; s_j < sqrt_spp; s_j++)
                            {
                                ray r = get_ray(i, j, s_i, s_j);
                                pixel_color += ray_color(r, max_depth, world, lights);
                            }
                        }
                        pixel_colors[j][i] = pixel_color;
                    }
                    
                    // Update progress (atomic operation)
                    int completed = ++scanlines_completed;
                    std::clog << "\rScanlines remaining: " << (height - completed) << ' ' << std::flush;
                } });
        }

        // Wait for all threads to complete
        for (auto &thread : threads)
        {
            thread.join();
        }

        // Write PPM header
        std::cout << "P3\n"
                  << width << ' ' << height << "\n255\n";

        // Write pixel data in order
        for (int j = 0; j < height; j++)
        {
            for (int i = 0; i < width; i++)
            {
                write_color(std::cout, pixel_samples_scale * pixel_colors[j][i]);
            }
        }

        std::clog << "\rDone.                 \n";
    }

private:
    int height;                 // Rendered image height
    double pixel_samples_scale; // Color scale factor for a sum of pixel samples
    int sqrt_spp;
    double inv_sqrt_spp;
    point3 center;              // Camera center
    point3 pixel00;             // Location of pixel 0, 0
    vec3 delta_u;               // Offset to pixel to the right
    vec3 delta_v;               // Offset to pixel below
    vec3 u, v, w;               // Camera frame basis vectors
    vec3 defocus_disk_u;        // Defocus disk horizontal radius
    vec3 defocus_disk_v;        // Defocus disk vertical radius

    void initialize()
    {
        height = int(width / ar);
        height = (height < 1) ? 1 : height;

        sqrt_spp = std::sqrt(samples_per_pixel);
        pixel_samples_scale = 1.0 / (sqrt_spp * sqrt_spp);
        inv_sqrt_spp = 1.0 / sqrt_spp;
        
        center = lookfrom;

        // Determine viewport dimensions.
        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta / 2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * (double(width) / height);

        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        auto viewport_u = viewport_width * u;
        auto viewport_v = viewport_height * -v;

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        delta_u = viewport_u / width;
        delta_v = viewport_v / height;

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left =
            center - (focus_dist * w) - viewport_u / 2 - viewport_v / 2;
        pixel00 = viewport_upper_left + 0.5 * (delta_u + delta_v);

        // Calculate the camera defocus disk basis vectors.
        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    ray get_ray(int i, int j, int s_i, int s_j) const
    {
        auto offset = sample_square_stratified(s_i, s_j);
        auto pixel_sample = pixel00 + ((i + offset.x()) * delta_u) + ((j + offset.y()) * delta_v);

        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;
        auto ray_time = random_double();

        return ray(ray_origin, ray_direction, ray_time);
    }
   
    vec3 sample_square_stratified(int s_i, int s_j) const
    {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        auto px = ((s_i + random_double()) * inv_sqrt_spp) - 0.5;
        auto py = ((s_j + random_double()) * inv_sqrt_spp) - 0.5;
        return vec3(px, py, 0);
    }

    vec3 sample_square() const
    {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    point3 defocus_disk_sample() const
    {
        // Returns a random point in the camera defocus disk.
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    color ray_color(const ray &r, int depth, const hittable &world, const hittable& lights) const
    {
        // If we've exceeded the ray bounce limit, no more light is gathered.
        if (depth <= 0)
            return color(0, 0, 0);

        hit_record rec;
        if (world.hit(r, interval(0.001, infinity), rec))
        {
            ray scattered;
            color attenuation;
            double pdf_value;
            scatter_record srec;
            color color_from_emission = rec.mat->emitted(r, rec, rec.u, rec.v, rec.p);

            
            
            if (rec.mat->scatter(r, rec, srec))
            {
                if (srec.skip_pdf) 
                {
                    return srec.attenuation * ray_color(srec.skip_pdf_ray, depth-1, world, lights);
                }
                
                if (rec.mat->use_light_sampling())
                {
                    // Use mixture PDF (light + material) for diffuse materials
                auto light_ptr = make_shared<hittable_pdf>(lights, rec.p);
                mixture_pdf p(light_ptr, srec.pdf_ptr);

                scattered = ray(rec.p, p.generate(), r.time());
                pdf_value = p.value(scattered.direction());
                }
                else
                {
                    // Use only material PDF for glossy materials (BRDF importance sampling)
                    scattered = ray(rec.p, srec.pdf_ptr->generate(), r.time());
                    pdf_value = srec.pdf_ptr->value(scattered.direction());
                }
                    
                color brdf_value = rec.mat->eval_brdf(r, rec, scattered);
                color sample_color = ray_color(scattered, depth-1, world, lights);
                color color_from_scatter = (brdf_value * sample_color) / pdf_value;

                // Use ratio-preserving clamp to maintain color when clamping
                const double max_radiance = 0.6;
                double max_component = std::max({color_from_scatter.x(), color_from_scatter.y(), color_from_scatter.z()});
                if (max_component > max_radiance) {
                    double scale = max_radiance / max_component;
                    color_from_scatter = color_from_scatter * scale;
                }

                // Russian Roulette: probabilistically terminate rays based on BRDF value
                // This allows rays to terminate early when they contribute little light
                double max_brdf = std::max({brdf_value.x(), brdf_value.y(), brdf_value.z()});

                // After a few bounces, use Russian Roulette
                if (depth > (max_depth - 3))
                {
                    // Always continue for first few bounces
                    return color_from_emission + color_from_scatter;
                }
                else if (random_double() < max_brdf)
                {
                    // Continue with probability proportional to BRDF value
                    return (color_from_emission + color_from_scatter) / max_brdf;
                }
                else
                {
                    // Terminate early
                    return color(0, 0, 0);
                }
            }
            else if(!rec.mat->scatter(r, rec, srec))
                return color_from_emission;

            return color(0, 0, 0);
        }
        else if(!world.hit(r, interval(0.001, infinity), rec))
            return background;

        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
    }
};

#endif