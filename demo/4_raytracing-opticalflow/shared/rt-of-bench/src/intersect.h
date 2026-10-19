#pragma once

#include "ray-tracing-util.h"


bool intersect(const Ray &ray, double &minDist, int &iHit) {
    /**
     * For
     *      p   = 3D intersection point
     *      s.c = 3D sphere center
     *      s.r = 1D sphere radius
     *      r.o = 3D ray origin
     *      r.d = 3D ray direction
     *      t   = 1D distance along the ray direction from r.o to p
     * Intersection point p has to be on the ray, e.g.
     *      p = r.o + t * r.d
     * Intersection point p has to be on the surface of the sphere, i.e.
     *      ( p - s.c ) • ( p - s.c ) = s.r**2
     * Combining both equations and applying some reformulation yields
     *      ( r.o + t * r.d - s.c ) • ( r.o + t * r.d - s.c ) = s.r**2
     *      ( r.o.x - s.c.x + t * r.d.x )**2 + ( r.o.y - s.c.y + t * r.d.y )**2 + ( r.o.z - s.c.z + t * r.d.z )**2 = s.r**2
     *      ( r.o.x - s.c.x )**2 + t * 2 * ( r.o.x - s.c.x ) * r.d.x + t**2 * r.d.x**2
     *          + ( r.o.y - s.c.y )**2 + t * 2 * ( r.o.y - s.c.y ) * r.d.y + t**2 * r.d.y**2
     *          + ( r.o.z - s.c.z )**2 + t * 2 * ( r.o.z - s.c.z ) * r.d.z + t**2 * r.d.z**2
     *          = s.r**2
     * Now we can obtain t by solving the quadratic equation of the form
     *      a * t**2 + b * t + c = 0
     * with
     *      a = r.d.x**2 + r.d.y**2 + r.d.z**2
     *        = 1 since ray directions are normalized
     *      b = 2 * ( r.o.x - s.c.x ) * r.d.x + 2 * ( r.o.y - s.c.y ) * r.d.y + 2 * ( r.o.z - s.c.z ) * r.d.z
     *      c = ( r.o.x - s.c.x )**2 + ( r.o.y - s.c.y )**2 + ( r.o.z - s.c.z )**2 - s.r**2
     * and its solutions
     *      ( -b +- sqrt ( b**2 - 4 * a * c ) ) / ( 2 * a )
     * if the following condition is satisfied (otherwise there is no intersection)
     *      ( b**2 - 4 * a * c ) >= 0
    */
    constexpr auto minRayDist = 1e-6;

    bool hit = false;

    for (auto i = 0; i < numSpheres; ++i) {
        constexpr auto a = 1.;
        auto b = 2 * dot(ray.origin - spheres[i].pos, ray.direction);
        auto c = dot(ray.origin - spheres[i].pos, ray.origin - spheres[i].pos) - spheres[i].rSq;
        double discr = b * b - 4 * a * c;

        if (discr >= 0.)
            for (auto tHit: {(-b + std::sqrt(discr)) / (2. * a), (-b - std::sqrt(discr)) / (2. * a)})
                if (tHit >= minRayDist && tHit < minDist) {
                    hit = true;
                    iHit = i;
                    minDist = tHit;
                }
    }

    return hit;
}
