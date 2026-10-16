#pragma once

#include <cstring>
#include <fstream>


void mapImageToDouble(size_t nxOF, size_t nyOF, size_t nxRT, const Color *__restrict__ imgSrc, double *__restrict__ imgDest, size_t supersampling) {
#pragma omp parallel for schedule (static) collapse(2)
    for (size_t j = 0; j < nyOF; ++j)
        for (size_t i = 0; i < nxOF; ++i) {
            double acc = 0;

            for (size_t jOff = 0; jOff < supersampling; ++jOff) {
                for (size_t iOff = 0; iOff < supersampling; ++iOff) {
#ifdef USE_COLOR
                    acc += 1. / 3. * (imgSrc[(supersampling * j + jOff) * nxRT + supersampling * i + iOff].x
                                      + imgSrc[(supersampling * j + jOff) * nxRT + supersampling * i + iOff].y
                                      + imgSrc[(supersampling * j + jOff) * nxRT + supersampling * i + iOff].z);
#else
                    acc += imgSrc[(supersampling * j + jOff) * nxRT + supersampling * i + iOff];
#endif
                }
            }

            imgDest[j * nxOF + i] = acc / (supersampling * supersampling);
        }
}


void mapImageToColor(size_t nxOF, size_t nyOF, size_t nxRT, const Color *__restrict__ imgSrc, Color *__restrict__ imgDest, size_t supersampling) {
#pragma omp parallel for schedule (static) collapse(2)
    for (size_t j = 0; j < nyOF; ++j) {
        for (size_t i = 0; i < nxOF; ++i) {
            Color acc{0};

            for (size_t jOff = 0; jOff < supersampling; ++jOff)
                for (size_t iOff = 0; iOff < supersampling; ++iOff)
                    acc += imgSrc[(supersampling * j + jOff) * nxRT + supersampling * i + iOff];

            imgDest[j * nxOF + i] = acc / (supersampling * supersampling);
        }
    }
}


void printImage(size_t nx, size_t ny, const Color *__restrict__ img, const std::string &filename) {
    std::ofstream outStream(filename, std::iostream::binary);
    for (size_t j = 1; j < ny - 1; ++j) {
        for (size_t i = 1; i < nx - 1; ++i) {
#ifdef USE_COLOR
            float c;
            c = (float) img[j * nx + i].x;
            outStream.write(reinterpret_cast<const char *>(&c), sizeof(float));
            c = (float) img[j * nx + i].y;
            outStream.write(reinterpret_cast<const char *>(&c), sizeof(float));
            c = (float) img[j * nx + i].z;
            outStream.write(reinterpret_cast<const char *>(&c), sizeof(float));
#else
            for (auto dim = 0; dim < 3; ++dim) {
                auto c = (float) img[j * nx + i];
                outStream.write(reinterpret_cast<const char *>(&c), sizeof(float));
            }
#endif
        }
    }

    outStream.close();
}


void printImage(size_t nx, size_t ny, const double *__restrict__ imgU, const double *__restrict__ imgV, const std::string &filename) {
    std::ofstream outStream(filename, std::iostream::binary);
    for (size_t j = 1; j < ny - 1; ++j) {
        for (size_t i = 1; i < nx - 1; ++i) {
            float r, g, b;
            // version 1, u and v on separate color channels, 0 on the remaining
            //r = (float) imgU[j * nx + i];
            //g = (float) imgV[j * nx + i];
            //b = (float) 0;

            // version 2, positive and negative components of v on separate channels, u on remaining
            //r = (float) std::max(0., imgV[j * nx + i]);
            //g = (float) std::max(0., -imgV[j * nx + i]);
            //b = (float) (imgU[j * nx + i]);

            // version 3, u and v on separate channels, magnitude on remaining
            r = (float) (imgU[j * nx + i]);
            g = (float) (imgV[j * nx + i]);
            b = (float) std::sqrt(imgU[j * nx + i] * imgU[j * nx + i] + imgV[j * nx + i] * imgV[j * nx + i]);

            outStream.write(reinterpret_cast<const char *>(&r), sizeof(float));
            outStream.write(reinterpret_cast<const char *>(&g), sizeof(float));
            outStream.write(reinterpret_cast<const char *>(&b), sizeof(float));
        }
    }

    outStream.close();
}
