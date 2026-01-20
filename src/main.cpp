#include "./core/rtweekend.h"

#include "./core/bvh.h"
#include "./core/camera.h"
#include "./core/constant_medium.h"
#include "./core/hittable.h"
#include "./core/hittable_list.h"
#include "./core/material.h"
#include "./core/obj_parser.h"
#include "./core/quad.h"
#include "./core/sphere.h"
#include "./core/sdf_group.h"
#include "./core/sdsphere.h"
#include "./core/texture.h"

#include <algorithm>
#include <vector>
#include <iostream>
#include <iomanip>
#include <cmath>

// void bouncing_spheres()
// {
//     hittable_list world;

//     auto checker = make_shared<checker_texture>(0.32, color(.2, .3, .1), color(.9, .9, .9));
//     world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(checker)));

//     for (int a = -11; a < 11; a++)
//     {
//         for (int b = -11; b < 11; b++)
//         {
//             auto choose_mat = random_double();
//             point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

//             if ((center - point3(4, 0.2, 0)).length() > 0.9)
//             {
//                 shared_ptr<material> sphere_material;

//                 if (choose_mat < 0.8)
//                 {
//                     // diffuse
//                     auto albedo = color::random() * color::random();
//                     sphere_material = make_shared<lambertian>(albedo);
//                     auto center2 = center + vec3(0, random_double(0, .5), 0);
//                     world.add(make_shared<sphere>(center, center2, 0.2, sphere_material));
//                 }
//                 else if (choose_mat < 0.95)
//                 {
//                     // metal
//                     auto albedo = color::random(0.5, 1);
//                     auto fuzz = random_double(0, 0.5);
//                     sphere_material = make_shared<metal>(albedo, fuzz);
//                     world.add(make_shared<sphere>(center, 0.2, sphere_material));
//                 }
//                 else
//                 {
//                     // glass
//                     sphere_material = make_shared<dielectric>(1.5);
//                     world.add(make_shared<sphere>(center, 0.2, sphere_material));
//                 }
//             }
//         }
//     }

//     auto material1 = make_shared<dielectric>(1.5);
//     world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

//     auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
//     world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

//     auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
//     world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

//     world = hittable_list(make_shared<bvh_node>(world));

//     camera cam;

//     cam.ar = 16.0 / 9.0;
//     cam.width = 400;
//     cam.samples_per_pixel = 100;
//     cam.max_depth = 50;
//     cam.background = color(0.70, 0.80, 1.00);

//     cam.vfov = 20;
//     cam.lookfrom = point3(13, 2, 3);
//     cam.lookat = point3(0, 0, 0);
//     cam.vup = vec3(0, 1, 0);

//     cam.defocus_angle = 0.6;
//     cam.focus_dist = 10.0;

//     cam.render(world);
// }

// void checkered_spheres()
// {
//     hittable_list world;

//     auto checker = make_shared<checker_texture>(0.32, color(.2, .3, .1), color(.9, .9, .9));

//     world.add(make_shared<sphere>(point3(0, -10, 0), 10, make_shared<lambertian>(checker)));
//     world.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

//     camera cam;

//     cam.ar = 16.0 / 9.0;
//     cam.width = 400;
//     cam.samples_per_pixel = 100;
//     cam.max_depth = 50;
//     cam.background = color(0.70, 0.80, 1.00);

//     cam.vfov = 20;
//     cam.lookfrom = point3(13, 2, 3);
//     cam.lookat = point3(0, 0, 0);
//     cam.vup = vec3(0, 1, 0);

//     cam.defocus_angle = 0;

//     cam.render(world);
// }

// void earth()
// {
//     auto earth_texture = make_shared<image_texture>("earthmap.jpg");
//     auto earth_surface = make_shared<lambertian>(earth_texture);
//     auto globe = make_shared<sphere>(point3(0, 0, 0), 2, earth_surface);

//     camera cam;

//     cam.ar = 16.0 / 9.0;
//     cam.width = 400;
//     cam.samples_per_pixel = 100;
//     cam.max_depth = 50;
//     cam.background = color(0.70, 0.80, 1.00);

//     cam.vfov = 20;
//     cam.lookfrom = point3(0, 0, 12);
//     cam.lookat = point3(0, 0, 0);
//     cam.vup = vec3(0, 1, 0);

//     cam.defocus_angle = 0;

//     cam.render(hittable_list(globe));
// }

void bubble()
{   
    auto glass = make_shared<dielectric>(1.5);
    auto iridescent_glass = make_shared<iridescent>(glass, 0.6);
    auto ball = make_shared<sphere>(point3(0, 0, 0), 2, iridescent_glass);

    camera cam;

    cam.ar = 16.0 / 9.0;
    cam.width = 600;
    cam.samples_per_pixel = 100;
    cam.max_depth = 50;
    cam.background = color(0.47, 0.57, 0.74);

    cam.vfov = 20;
    cam.lookfrom = point3(0, 0, 12);
    cam.lookat = point3(0, 0, 0);
    cam.vup = vec3(0, 1, 0);

    cam.defocus_angle = 0;

    hittable_list lights;
    cam.render(hittable_list(ball), lights);
}

void simple_scene()
{
    hittable_list world;

    // Floor - large sphere acting as ground
    auto floor_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, floor_material));

    // Light source - emissive sphere
    auto light_material = make_shared<diffuse_light>(color(15, 15, 13));
    auto light_sphere = make_shared<sphere>(point3(-2, 4, 5), 1, light_material);
    world.add(light_sphere);

    // Diffuse sphere - will be illuminated by the light
    auto diffuse_material = make_shared<lambertian>(color(0.8, 0.3, 0.3));
    world.add(make_shared<sphere>(point3(0, 1.5, 0), 1.5, diffuse_material));

    // Light list for importance sampling
    hittable_list lights;
    lights.add(light_sphere);

    camera cam;

    cam.ar = 16.0 / 9.0;
    cam.width = 600;
    cam.samples_per_pixel = 10;
    cam.max_depth = 50;
    cam.background = color(0, 0, 0);

    cam.vfov = 45;
    cam.lookfrom = point3(5, 3, 7);
    cam.lookat = point3(0, 1, 0);
    cam.vup = vec3(0, 1, 0);

    cam.defocus_angle = 0;

    cam.render(world, lights);
}

// void perlin_spheres()
// {
//     hittable_list world;

//     // Use debug_gradient_texture to visualize gradient vectors at corners
//     // Use debug_lattice_texture to see noise with grid boundaries
//     // Use noise_texture to see the actual noise
//     auto pertext = make_shared<noise_texture>(4);
//     // auto pertext = make_shared<debug_lattice_texture>(2);
//     // auto pertext = make_shared<noise_texture>(2);

//     world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
//     world.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

//     camera cam;

//     cam.ar = 16.0 / 9.0;
//     cam.width = 400;
//     cam.samples_per_pixel = 100;
//     cam.max_depth = 50;
//     cam.background = color(0.70, 0.80, 1.00);

//     cam.vfov = 20;
//     cam.lookfrom = point3(13, 2, 3);
//     cam.lookat = point3(0, 0, 0);
//     cam.vup = vec3(0, 1, 0);

//     cam.defocus_angle = 0;

//     cam.render(world);
// }

// void simple_light()
// {
//     hittable_list world;

//     auto pertext = make_shared<noise_texture>(4);
//     world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertext)));
//     world.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(pertext)));

//     auto difflight = make_shared<diffuse_light>(color(7,7,7));
//     world.add(make_shared<sphere>(point3(7,10,-3), 5, difflight));

//     camera cam;

//     cam.ar      = 16.0 / 9.0;
//     cam.width       = 400;
//     cam.samples_per_pixel = 100;
//     cam.max_depth         = 50;
//     cam.background        = color(0,0,0);

//     cam.vfov     = 20;
//     cam.lookfrom = point3(26,3,6);
//     cam.lookat   = point3(0,2,0);
//     cam.vup      = vec3(0,1,0);

//     cam.defocus_angle = 0;

//     cam.render(world);
// }

// void quads() {
//     hittable_list world;

//     // Materials
//     auto left_red     = make_shared<lambertian>(color(1.0, 0.2, 0.2));
//     auto back_green   = make_shared<lambertian>(color(0.2, 1.0, 0.2));
//     auto right_blue   = make_shared<lambertian>(color(0.2, 0.2, 1.0));
//     auto upper_orange = make_shared<lambertian>(color(1.0, 0.5, 0.0));
//     auto lower_teal   = make_shared<lambertian>(color(0.2, 0.8, 0.8));

//     // Quads
//     world.add(make_shared<quad>(point3(-3,-2, 5), vec3(0, 0,-4), vec3(0, 4, 0), left_red));
//     world.add(make_shared<quad>(point3(-2,-2, 0), vec3(4, 0, 0), vec3(0, 4, 0), back_green));
//     world.add(make_shared<quad>(point3( 3,-2, 1), vec3(0, 0, 4), vec3(0, 4, 0), right_blue));
//     world.add(make_shared<quad>(point3(-2, 3, 1), vec3(4, 0, 0), vec3(0, 0, 4), upper_orange));
//     world.add(make_shared<quad>(point3(-2,-3, 5), vec3(4, 0, 0), vec3(0, 0,-4), lower_teal));

//     camera cam;

//     cam.ar      = 1.0;
//     cam.width       = 400;
//     cam.samples_per_pixel = 100;
//     cam.max_depth         = 50;
//     cam.background        = color(0.70, 0.80, 1.00);

//     cam.vfov     = 80;
//     cam.lookfrom = point3(0,0,9);
//     cam.lookat   = point3(0,0,0);
//     cam.vup      = vec3(0,1,0);

//     cam.defocus_angle = 0;

//     cam.render(world);
// }

void cornell_box() {
    hittable_list world;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    // Cornell box sides
    world.add(make_shared<quad>(point3(555,0,0), vec3(0,0,555), vec3(0,555,0), green));
    world.add(make_shared<quad>(point3(0,0,555), vec3(0,0,-555), vec3(0,555,0), red));
    world.add(make_shared<quad>(point3(0,555,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,0,-555), white));
    world.add(make_shared<quad>(point3(555,0,555), vec3(-555,0,0), vec3(0,555,0), white));

    // Light
    world.add(make_shared<quad>(point3(213,554,227), vec3(130,0,0), vec3(0,0,105), light));

   // Box
    shared_ptr<hittable> box1 = box(point3(0,0,0), point3(165,330,165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));
    world.add(box1);

    // Glass Sphere
    auto glossy_sphere = make_shared<glossy>(color(0.8, 0.8, 0.8), 0.3, 1.0);
    world.add(make_shared<sphere>(point3(190,90,190), 90, glossy_sphere));

    // Light Sources
    auto empty_material = shared_ptr<material>();
    hittable_list lights;
    lights.add(make_shared<quad>(point3(343,554,332), vec3(-130,0,0), vec3(0,0,-105), empty_material));
    lights.add(make_shared<sphere>(point3(190, 90, 190), 90, empty_material));

    camera cam;

    cam.ar                = 1.0;
    cam.width             = 600;
    cam.samples_per_pixel = 1000;
    cam.max_depth         = 50;
    cam.background        = color(0,0,0);

    cam.vfov     = 40;
    cam.lookfrom = point3(278, 278, -800);
    cam.lookat   = point3(278, 278, 0);
    cam.vup      = vec3(0, 1, 0);

    cam.defocus_angle = 0;

    cam.render(world, lights);
}

// void cornell_smoke() {
//     hittable_list world;

//     auto red   = make_shared<lambertian>(color(.65, .05, .05));
//     auto white = make_shared<lambertian>(color(.73, .73, .73));
//     auto green = make_shared<lambertian>(color(.12, .45, .15));
//     auto light = make_shared<diffuse_light>(color(7, 7, 7));

//     world.add(make_shared<quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), green));
//     world.add(make_shared<quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), red));
//     world.add(make_shared<quad>(point3(113,554,127), vec3(330,0,0), vec3(0,0,305), light));
//     world.add(make_shared<quad>(point3(0,555,0), vec3(555,0,0), vec3(0,0,555), white));
//     world.add(make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), white));
//     world.add(make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), white));

//     shared_ptr<hittable> box1 = box(point3(0,0,0), point3(165,330,165), white);
//     box1 = make_shared<rotate_y>(box1, 15);
//     box1 = make_shared<translate>(box1, vec3(265,0,295));

//     shared_ptr<hittable> box2 = box(point3(0,0,0), point3(165,165,165), white);
//     box2 = make_shared<rotate_y>(box2, -18);
//     box2 = make_shared<translate>(box2, vec3(130,0,65));

//     world.add(make_shared<constant_medium>(box1, 0.01, color(0,0,0)));
//     world.add(make_shared<constant_medium>(box2, 0.01, color(1,1,1)));

//     camera cam;

//     cam.ar                = 1.0;
//     cam.width             = 600;
//     cam.samples_per_pixel = 200;
//     cam.max_depth         = 50;
//     cam.background        = color(0,0,0);

//     cam.vfov     = 40;
//     cam.lookfrom = point3(278, 278, -800);
//     cam.lookat   = point3(278, 278, 0);
//     cam.vup      = vec3(0,1,0);

//     cam.defocus_angle = 0;

//     cam.render(world);
// }

// void load_obj()
// {
//     obj_parser parser;
//     if (!parser.load("../src/models/CartoonTree.obj")) {
//         std::cerr << "Failed to load CartoonTree.obj\n";
//         return;
//     }
//     auto world = parser.parse(make_shared<lambertian>(color(0.15, 0.35, 0.20)));

//     std::cerr << "Loaded " << world.objects.size() << " triangles\n";

//     auto dirt_noise = make_shared<noise_texture>(1.0, color(0.4, 0.2, 0.1));
//     auto ground_material = make_shared<lambertian>(dirt_noise);
    
//     world.add(make_shared<quad>(
//         point3(-10, 0, -10),  // Corner point (bottom-left-back)
//         vec3(20, 0, 0),        // Extend 20 units in +x direction
//         vec3(0, 0, 20),        // Extend 20 units in +z direction
//         ground_material
//     ));

//     auto star = make_shared<sphere>(point3(0,6.5,0), .3, make_shared<diffuse_light>(color(7,7,7)));
//     world.add(star);

//     // Ornament materials - weird and varied!
//     auto red_ornament = make_shared<metal>(color(0.8, 0.1, 0.1), 0.1);
    
//     // Noise-based materials (cloud-like, marble-like)
//     auto cloud_white = make_shared<lambertian>(make_shared<noise_texture>(3.0, color(0.9, 0.9, 0.95)));
//     auto cloud_blue = make_shared<lambertian>(make_shared<noise_texture>(4.0, color(0.6, 0.8, 1.0)));
//     auto cloud_pink = make_shared<lambertian>(make_shared<noise_texture>(2.5, color(1.0, 0.7, 0.8)));
//     auto marble_red = make_shared<lambertian>(make_shared<noise_texture>(8.0, color(0.7, 0.2, 0.2)));
//     auto marble_green = make_shared<lambertian>(make_shared<noise_texture>(6.0, color(0.2, 0.5, 0.3)));
//     auto turbulent_purple = make_shared<lambertian>(make_shared<turb_noise_texture>(2.0, color(0.5, 0.3, 0.8)));
//     auto turbulent_orange = make_shared<lambertian>(make_shared<turb_noise_texture>(5.0, color(0.9, 0.5, 0.2)));
    
//     // Smooth metals for contrast
//     auto blue_ornament = make_shared<metal>(color(0.1, 0.2, 0.8), 0.1);
//     auto gold_ornament = make_shared<metal>(color(0.85, 0.65, 0.13), 0.05);
//     auto silver_ornament = make_shared<metal>(color(0.75, 0.75, 0.75), 0.2);
//     auto green_ornament = make_shared<metal>(color(0.1, 0.6, 0.2), 0.1);
//     auto bronze_ornament = make_shared<metal>(color(0.7, 0.5, 0.3), 0.3);
    
//     // Glass variations
//     auto blue_glass = make_shared<dielectric>(1.5);
//     auto green_glass = make_shared<dielectric>(1.5);
//     auto purple_glass = make_shared<dielectric>(1.5);
//     auto red_glass = make_shared<dielectric>(1.5);
//     auto iridescent_base = make_shared<dielectric>(1.5);
//     auto iridescent_ornament = make_shared<iridescent>(iridescent_base, 0.7);
    
//     // Checker pattern materials
//     auto checker_red_white = make_shared<lambertian>(make_shared<checker_texture>(0.15, color(0.8, 0.1, 0.1), color(0.95, 0.95, 0.95)));
//     auto checker_blue_yellow = make_shared<lambertian>(make_shared<checker_texture>(0.2, color(0.2, 0.4, 0.9), color(0.9, 0.8, 0.2)));
//     auto checker_green_gold = make_shared<lambertian>(make_shared<checker_texture>(0.18, color(0.2, 0.6, 0.2), color(0.85, 0.65, 0.13)));
    
//     double ornament_radius = 0.25;
    
//     // Bottom layer ornaments - evenly distributed across angles and heights
//     // Red ornament moved closer (smaller radius) to reduce space between tree and ornament
//     double angle1 = 0.4;     double y1 = 1.6; double r1 = 1.7;  // Right side, bottom
//     double angle2 = 0.6;     double y2 = 2.4; double r2 = 1.5;  // Right-middle
//     double angle3 = 1.2;     double y3 = 2.8; double r3 = 1.6;  // Middle-right
//     double angle4 = 1.8;    double y4 = 1.8; double r4 = 1.6;  // Middle (π/2) - fills bottom middle gap
//     double angle5 = 2.1;     double y5 = 2.6; double r5 = 1.4;  // Middle-left
//     double angle6 = 2.7;     double y6 = 3.0; double r6 = 1.5;  // Left side
    
//     world.add(make_shared<sphere>(point3(r1*cos(angle1), y1, r1*sin(angle1)), ornament_radius, red_ornament));
//     // Smokey white ornament using constant_medium
//     auto cloud_white_sphere = make_shared<sphere>(point3(r2*cos(angle2), y2, r2*sin(angle2)), ornament_radius, make_shared<lambertian>(color(1,1,1)));
//     world.add(make_shared<constant_medium>(cloud_white_sphere, 0.15, color(0.95, 0.98, 1.0)));
//     world.add(make_shared<sphere>(point3(r3*cos(angle3), y3, r3*sin(angle3)), ornament_radius, gold_ornament));
//     world.add(make_shared<sphere>(point3(r4*cos(angle4), y4, r4*sin(angle4)), ornament_radius, checker_red_white));
//     world.add(make_shared<sphere>(point3(r5*cos(angle5), y5, r5*sin(angle5)), ornament_radius, marble_green));
//     world.add(make_shared<sphere>(point3(r6*cos(angle6), y6, r6*sin(angle6)), ornament_radius, iridescent_ornament));
    
//     // Middle layer ornaments - evenly distributed
//     double angle7 = 0.3;     double y7 = 3.8; double r7 = 1.0;  // Right side
//     double angle8 = 1.57;    double y8 = 4.2; double r8 = 1.1;  // Middle - fills middle gap
//     double angle9 = 2.8;     double y9 = 3.9; double r9 = 1.0;  // Left side
    
//     world.add(make_shared<sphere>(point3(r7*cos(angle7), y7, r7*sin(angle7)), ornament_radius, cloud_blue));
//     world.add(make_shared<sphere>(point3(r8*cos(angle8), y8, r8*sin(angle8)), ornament_radius, silver_ornament));
//     world.add(make_shared<sphere>(point3(r9*cos(angle9), y9, r9*sin(angle9)), ornament_radius, marble_red));
    
//     // Top layer ornaments - distributed left and right to fill top right gap
//     double angle10 = 0.4;    double y10 = 5.1; double r10 = 0.8;  // Top right - fills gap
//     double angle11 = 2.9;    double y11 = 5.3; double r11 = 0.7;  // Top left
    
//     world.add(make_shared<sphere>(point3(r10*cos(angle10), y10, r10*sin(angle10)), ornament_radius, checker_blue_yellow));
//     world.add(make_shared<sphere>(point3(r11*cos(angle11), y11, r11*sin(angle11)), ornament_radius, turbulent_purple));
    
//     auto difflight = make_shared<diffuse_light>(color(7,7,7));
//     world.add(make_shared<sphere>(point3(6,10,3), 5, difflight));
    
//     camera cam;

//     cam.ar = 16.0 / 9.0;
//     cam.width = 900;
//     cam.samples_per_pixel = 300;
//     cam.max_depth = 50;
//     cam.background = color(0.70, 0.80, 1.00);

//     cam.vfov = 30;
//     cam.lookfrom = point3(0, 4, 15);
//     cam.lookat = point3(0, 2.5, 0);
//     cam.vup = vec3(0, 1, 0);

//     cam.defocus_angle = 0;

//     world = hittable_list(make_shared<bvh_node>(world));

//     cam.render(world);
// }

void estimate_pi()
{
    std::cout << std::fixed << std::setprecision(12);

    int in_circle = 0;
    int in_circle_stratified = 0;
    int N = 1000000;
    auto sqrt_N = std::sqrt(N);

    for(int i = 0; i < sqrt_N; i++)
    {
        for(int j = 0; j < sqrt_N; j++)
        {
            double x = random_double(-1, 1);
            double y = random_double(-1, 1);
    
            if (x*x + y*y < 1)
            {
                in_circle++;
            }

            x = 2 * ((i + random_double()) / sqrt_N) - 1;
            y = 2 * ((j + random_double()) / sqrt_N) - 1;

            if(x*x + y*y < 1)
            {
                in_circle_stratified++;
            }
        }
    }

    std::cout << "Regular    Estimate of Pi: " << (4. * in_circle) / N << std::endl;
    std::cout << "Stratified Estimate of Pi: " << (4. * in_circle_stratified) / N << std::endl;
}

void estimate_log_sin()
{
    int a = 0;
    int b = 2;
    int N = 1000000;
    auto sum = 0.0;

    for(int i = 0; i < N; i++)
    {
        auto x = random_double(a,b);
        sum += std::log(std::sin(x));
    }

    std::cout << std::fixed << std::setprecision(12);
    std::cout << "I = " << (b-a) * (sum / N) << std::endl; 
}

struct sample 
{
    double x;
    double p_x;
};

bool compare_by_x(const sample& a, const sample& b) 
{
    return a.x < b.x;
}

void estimate_log_sin_halfway_point()
{
    int a = 0;
    int b = 2;
    const unsigned int N = 10000;
    sample samples[N];
    auto sum = 0.0;

    for(int i = 0; i < N; i++)
    {
        auto x = random_double(0, 2*pi);
        auto sin_x = std::sin(x);
        auto p_x = exp(-x / (2*pi)) * sin_x * sin_x;
        sum += p_x;

        sample this_sample{x, p_x};
        samples[i] = this_sample;
    }

    std::sort(std::begin(samples), std::end(samples), compare_by_x);

    double half_sum = sum / 2.0;
    double halfway_point = 0.0;
    double accum = 0.0;

    for (unsigned int i = 0; i < N; i++)
    {
        accum += samples[i].p_x;
        if(accum >= half_sum)
        {
            halfway_point = samples[i].x;
            break;
        }
    }
    

    std::cout << std::fixed << std::setprecision(12);
    std::cout << "Average = " << sum / N << std::endl; 
    std::cout << "Area under curve = " << 2 * pi * sum / N << '\n';
    std::cout << "Halfway = " << halfway_point << std::endl;
}

double f(double r)
{
    double z = 1 - r;
    double cos_theta = z;
    return cos_theta*cos_theta*cos_theta;
}

double p()
{
    return 1.0 / (2.0 * pi);
}

void integrate_cos_cubed()
{
    int N = 1000000;
    auto sum = 0.0;

    for(int i = 0; i < N; i++)
    {
        auto r2 = random_double();
        sum += f(r2) / p();
    }

    std::cout << std::fixed << std::setprecision(12);
    std::cout << "PI/2 = " << pi / 2.0 << '\n';
    std::cout << "Estimate = " << sum / N << '\n';
}

int main()
{
    switch (15)
    {
    // case 1:
    //     bouncing_spheres();
    //     break;
    // case 2:
    //     checkered_spheres();
    //     break;
    // case 3:
    //     earth();
    //     break;
    case 4:
        bubble();
        break;
    // case 5:
    //     perlin_spheres();
    //     break;
    // case 6:
    //     simple_light();
    //     break;
    // case 7:
    //     quads();
    //     break;
    case 8:
        cornell_box();
        break;
    // case 9:
    //     cornell_smoke();
    //     break;
    // case 10:
    //     load_obj();
    //     break;
    case 11:
        estimate_pi();
        break;
    case 12:
        estimate_log_sin();
        break;
    case 13:
        estimate_log_sin_halfway_point();
        break;
    case 14:
        integrate_cos_cubed();
        break;
    case 15:
        simple_scene();
        break;
    }
}