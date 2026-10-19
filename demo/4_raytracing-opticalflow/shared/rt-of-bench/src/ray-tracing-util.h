#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>

#include "vec3.h"


// speed parameters
constexpr auto camSpeed = 0.25; // revolutions per second //default 0.25
constexpr auto sphereSpeed = 1.; // domain heights per second //default 1.0

// boundary parameters for initial sphere placement
constexpr auto boundaryExtentXZ = 2.; // extent in each direction (positive & negative) in the x- and z-axes
constexpr auto boundaryExtentY = 2 * boundaryExtentXZ;

// color or black & wight
//#define USE_COLOR


struct Ray {
    Vec3 origin;
    Vec3 direction;
};


#ifdef USE_COLOR
using Color = Vec3;
#else
using Color = double;
#endif


struct Sphere {
    Vec3 initPos;   // initial position before moving
    Vec3 pos;       // current position
    double rSq;     // radius squared

    Color diffuseColor;
    Color specularColor;
    double specularExp;
    Color reflectionColor;
};


Sphere *spheres;
int numSpheres;


inline void initSpheres(double &offset, double &scalingFactor) {
    std::mt19937 randGen(0);// constant seed
    std::uniform_real_distribution<> posXZDistribution(-boundaryExtentXZ, boundaryExtentXZ);
    std::uniform_real_distribution<> posYDistribution(-boundaryExtentY, boundaryExtentY);
    std::uniform_real_distribution<> rDistribution(0.35, 0.7);
    for (auto s = 0; s < numSpheres; ++s) {
        auto pos = Vec3{posXZDistribution(randGen), posYDistribution(randGen), posXZDistribution(randGen)};
        auto r = rDistribution(randGen);

        auto invalidPos = false;

        // check for side walls
        if (std::sqrt(pos.x * pos.x + pos.z * pos.z) + r > boundaryExtentXZ * (offset + scalingFactor * sin((pos.y / boundaryExtentY + 0.5) * M_PI)))
            invalidPos = true;

        // check for collisions with other particles; also check periodic mirror particle positions
        for (auto sCmp = 0; sCmp < s && !invalidPos; ++sCmp)
            if (std::min({
                                 (spheres[sCmp].pos - pos).magnitude(),
                                 (spheres[sCmp].pos - pos + Vec3{0., 2 * boundaryExtentY, 0.}).magnitude(),
                                 (spheres[sCmp].pos - pos + Vec3{0., -2 * boundaryExtentY, 0.}).magnitude()
                         }) < 0.1 + 1. * (std::sqrt(spheres[sCmp].rSq) + r))
                invalidPos = true;

        if (invalidPos) {
            --s;
            continue;
        }

        // diffuse material w/o reflection
        //spheres[s] = {pos, pos, r * r, Color{1.}, Color{0.}, 32, Color{0.}};

        // highly reflective material
        spheres[s] = {pos, pos, r * r, Color{0.5}, Color{0.8}, 32, Color{0.8}};
    }
}

inline void updateSpherePositions(double t) {
    for (auto s = 0; s < numSpheres; ++s) {
        spheres[s].pos.y = spheres[s].initPos.y - sphereSpeed * t * 2 * boundaryExtentY;
        while (spheres[s].pos.y < -boundaryExtentY)
            spheres[s].pos.y += 2 * boundaryExtentY;
    }
}


struct Light {
    Vec3 position; // 3D position
    Color color;
};
//using Light = Vec3;

#ifdef USE_COLOR
Light lights[] = {
        {{-16., 0., -16.}, {1.,  0.5, 0.5}},
        {{16.,  0., -16.}, {0.5, 1.,  0.5}},
        {{0.,   0., 16.},  {0.5, 0.5, 1.}}
};
#else
Light lights[] = {
        {{-16., 0., -16.}, 2. / 3.},
        {{16.,  0., -16.}, 2. / 3.},
        {{0.,   0., 16.},  2. / 3.}
};
#endif

constexpr auto numLights = sizeof(lights) / sizeof(Light);
