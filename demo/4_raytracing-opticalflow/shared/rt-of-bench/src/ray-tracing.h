#pragma once

#include "ray-tracing-util.h"
#include "intersect.h"


Color trace(const Ray &ray, int numBounces) {
    // locate the closest intersection
    int iHit = 0;
    double rayDist = 1e6;
    if (!intersect(ray, rayDist, iHit))
        return Color{0};

    Sphere &sphere = spheres[iHit];

    auto hitPoint = ray.origin + rayDist * ray.direction;
    auto normal = hitPoint - sphere.pos;
    normal = normal.normalize();

    auto reflectionRay = Ray{hitPoint, ray.direction - 2 * dot(normal, ray.direction) * normal};

    // add lightning
    Color color(0);

    for (const auto &light: lights) {
        auto lightRay = Ray{hitPoint, (light.position - hitPoint).normalize()};

        // check if light is visible - first compare light direction with surface normal
        auto diffuseIntensity = dot(lightRay.direction, normal);
        if (diffuseIntensity < 0)
            continue;

        // then check if the light source is occluded
        auto lightRayDist = 1e6;
        if (intersect(lightRay, lightRayDist, iHit))
            continue;

        // add diffuse contribution
        color += diffuseIntensity * sphere.diffuseColor * light.color;

        // add specular contribution
        auto specularIntensity = dot(reflectionRay.direction, lightRay.direction);
        if (specularIntensity > 0)
            color += pow(specularIntensity, sphere.specularExp) * sphere.specularColor * light.color;
    }

    // add contribution from reflection ray
    if (numBounces < 8)
        color += sphere.reflectionColor * trace(reflectionRay, numBounces + 1);

    return color;
}


inline void createImage(size_t nx, size_t ny, double t, Color *__restrict__ img) {
#pragma omp parallel for schedule (static) collapse(2)
    for (size_t j = 0; j < ny; ++j) {
        for (size_t i = 0; i < nx; ++i) {
            // compute screen coordinates
            auto fov = 60;
            auto scale = tan(fov * 0.5 * M_PI / 180);
            auto imageAspectRatio = nx / (double) ny;

            auto origin = Vec3{0., 0., 4.};

            auto px = (2 * (i + 0.5) / nx - 1) * scale * imageAspectRatio;
            auto py = (1 - 2 * (j + 0.5) / ny) * scale;
            auto direction = Vec3{px, py, -1};

            // rotate camera
            double alpha = 0., beta = t * camSpeed * 2. * M_PI, gamma = 0 * -0.125 * M_PI;

            origin = origin.rotate(alpha, beta, gamma);
            direction = direction.rotate(alpha, beta, gamma);

            // create and trace ray for current pixel
            Ray ray{origin, direction.normalize()};
            Color color = trace(ray, 0);

            // clamp color to [0, 1] and store result
#ifdef USE_COLOR
            color.x = std::min(1., std::max(0., color.x));
            color.y = std::min(1., std::max(0., color.y));
            color.z = std::min(1., std::max(0., color.z));
#else
            color = std::min(1., std::max(0., color));
#endif
            img[j * nx + i] = color;
        }
    }
}
