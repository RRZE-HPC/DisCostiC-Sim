#pragma once

constexpr auto ompThreshold = 8 * 1024; // number of work elements required to enable OpenMP parallelization

constexpr double regularization = 1.6e-3; // alpha**2 in the original formulation -> name clash with CG alpha


inline void initGradientsAndRHS(size_t nx, size_t ny, double dt,
                                const double *__restrict__ img0, const double *__restrict__ img1,
                                double *__restrict__ ixix, double *__restrict__ ixiy, double *__restrict__ iyiy,
                                double *__restrict__ uRHS, double *__restrict__ vRHS) {

    auto gridWidth = 1. / (nx - 2.);

#pragma omp parallel for schedule (static) collapse(2) if((nx - 2) * (ny - 2) >= ompThreshold)
    for (size_t j = 1; j < ny - 1; ++j) {
        for (size_t i = 1; i < nx - 1; ++i) {
            auto ix = (img0[(j + 0) * nx + (i + 1)] - img0[j * nx + i]) / gridWidth;// + (img1[(j + 0) * nx + (i + 1)] - img1[j * nx + i]) / gridWidth;
            auto iy = (img0[(j + 1) * nx + (i + 0)] - img0[j * nx + i]) / gridWidth;// + (img1[(j + 1) * nx + (i + 0)] - img1[j * nx + i]) / gridWidth;
            auto it = (img0[j * nx + i] - img1[j * nx + i]) / dt;
            ixix[j * nx + i] = ix * ix;
            ixiy[j * nx + i] = ix * iy;
            iyiy[j * nx + i] = iy * iy;
            uRHS[j * nx + i] = -ix * it;
            vRHS[j * nx + i] = -iy * it;
        }
    }
}


inline void applyBC(size_t nx, size_t ny, double *__restrict__ u, double *__restrict__ v) {
    auto scale = -1.;

    // east
    for (size_t j = 1; j < ny - 1; ++j) {
        auto i = nx - 1;
        u[j * nx + i] = scale * u[(j + 0) * nx + (i - 1)];
        v[j * nx + i] = scale * v[(j + 0) * nx + (i - 1)];
    }

    // west
    for (size_t j = 1; j < ny - 1; ++j) {
        auto i = 0;
        u[j * nx + i] = scale * u[(j + 0) * nx + (i + 1)];
        v[j * nx + i] = scale * v[(j + 0) * nx + (i + 1)];
    }

    // north
    for (size_t i = 1; i < nx - 1; ++i) {
        auto j = ny - 1;
        u[j * nx + i] = scale * u[(j - 1) * nx + (i + 0)];
        v[j * nx + i] = scale * v[(j - 1) * nx + (i + 0)];
    }

    // south
    for (size_t i = 1; i < nx - 1; ++i) {
        auto j = 0;
        u[j * nx + i] = scale * u[(j + 1) * nx + (i + 0)];
        v[j * nx + i] = scale * v[(j + 1) * nx + (i + 0)];
    }
}


inline void smooth(size_t nx, size_t ny, double omega,
                   const double *__restrict__ u, const double *__restrict__ v,
                   double *__restrict__ uNew, double *__restrict__ vNew,
                   const double *__restrict__ ixix, const double *__restrict__ ixiy, const double *__restrict__ iyiy,
                   const double *__restrict__ uRHS, const double *__restrict__ vRHS) {

    auto gridWidthSqInv = ((nx - 2.) * (nx - 2.));

#pragma omp parallel for schedule (static) collapse(2) if((nx - 2) * (ny - 2) >= ompThreshold)
    for (size_t j = 1; j < ny - 1; ++j) {
        for (size_t i = 1; i < nx - 1; ++i) {
            uNew[j * nx + i] = (1. - omega) * u[j * nx + i]
                               + omega *
                                 (uRHS[j * nx + i] - ixiy[j * nx + i] * v[j * nx + i]
                                  + regularization * gridWidthSqInv * (u[j * nx + (i - 1)] + u[j * nx + (i + 1)] + u[(j - 1) * nx + i] + u[(j + 1) * nx + i]))
                                 / (ixix[j * nx + i] + regularization * 4. * gridWidthSqInv);

            vNew[j * nx + i] = (1. - omega) * v[j * nx + i]
                               + omega *
                                 (vRHS[j * nx + i] - ixiy[j * nx + i] * u[j * nx + i]
                                  + regularization * gridWidthSqInv * (v[j * nx + (i - 1)] + v[j * nx + (i + 1)] + v[(j - 1) * nx + i] + v[(j + 1) * nx + i]))
                                 / (iyiy[j * nx + i] + regularization * 4 * gridWidthSqInv);
        }
    }
}


inline void updateRes(size_t nx, size_t ny,
                      const double *__restrict__ u, const double *__restrict__ v,
                      double *__restrict__ uRes, double *__restrict__ vRes,
                      const double *__restrict__ ixix, const double *__restrict__ ixiy, const double *__restrict__ iyiy,
                      const double *__restrict__ uRHS, const double *__restrict__ vRHS) {

    auto gridWidthSqInv = ((nx - 2.) * (nx - 2.));

#pragma omp parallel for schedule (static) collapse(2) if((nx - 2) * (ny - 2) >= ompThreshold)
    for (size_t j = 1; j < ny - 1; ++j) {
        for (size_t i = 1; i < nx - 1; ++i) {
            uRes[j * nx + i] = uRHS[j * nx + i] - (
                    ixix[j * nx + i] * u[j * nx + i]
                    + ixiy[j * nx + i] * v[j * nx + i]
                    + regularization * gridWidthSqInv *
                      (4 * u[j * nx + i] - (u[j * nx + i - 1] + u[j * nx + i + 1] + u[(j - 1) * nx + i] + u[(j + 1) * nx + i])));

            vRes[j * nx + i] = vRHS[j * nx + i] - (
                    ixiy[j * nx + i] * u[j * nx + i]
                    + iyiy[j * nx + i] * v[j * nx + i]
                    + regularization * gridWidthSqInv *
                      (4 * v[j * nx + i] - (v[j * nx + i - 1] + v[j * nx + i + 1] + v[(j - 1) * nx + i] + v[(j + 1) * nx + i])));
        }
    }
}


inline double resNorm(size_t nx, size_t ny,
                      const double *__restrict__ u, const double *__restrict__ v,
                      const double *__restrict__ ixix, const double *__restrict__ ixiy, const double *__restrict__ iyiy,
                      const double *__restrict__ uRHS, const double *__restrict__ vRHS) {

    auto gridWidthSqInv = ((nx - 2.) * (nx - 2.));

    auto norm = 0.;

#pragma omp parallel for schedule (static) collapse(2) reduction( + : norm) if((nx - 2) * (ny - 2) >= ompThreshold)
    for (size_t j = 1; j < ny - 1; ++j) {
        for (size_t i = 1; i < nx - 1; ++i) {
            auto uRes = uRHS[j * nx + i] - (
                    ixix[j * nx + i] * u[j * nx + i]
                    + ixiy[j * nx + i] * v[j * nx + i]
                    + regularization * gridWidthSqInv *
                      (4 * u[j * nx + i] - (u[j * nx + i - 1] + u[j * nx + i + 1] + u[(j - 1) * nx + i] + u[(j + 1) * nx + i])));

            auto vRes = vRHS[j * nx + i] - (
                    ixiy[j * nx + i] * u[j * nx + i]
                    + iyiy[j * nx + i] * v[j * nx + i]
                    + regularization * gridWidthSqInv *
                      (4 * v[j * nx + i] - (v[j * nx + i - 1] + v[j * nx + i + 1] + v[(j - 1) * nx + i] + v[(j + 1) * nx + i])));

            norm += uRes * uRes + vRes * vRes;
        }
    }

    return std::sqrt(norm);
}


inline void updateCoarserRhs(size_t nxCoarser, size_t nyCoarser, size_t nx, size_t ny,
                             const double *__restrict__ resU, const double *__restrict__ resV,
                             double *__restrict__ rhsUCoarser, double *__restrict__ rhsVCoarser) {

    auto scale = 1.;

#pragma omp parallel for schedule (static) collapse(2) if((nxCoarser - 2) * (nyCoarser - 2) >= ompThreshold)
    for (size_t j = 1; j < nyCoarser - 1; ++j) {
        for (size_t i = 1; i < nxCoarser - 1; ++i) {
            rhsUCoarser[j * nxCoarser + i] = scale * 0.25 * (
                    resU[((j - 1) * 2 + 1 + 0) * nx + ((i - 1) * 2 + 1 + 0)]
                    + resU[((j - 1) * 2 + 1 + 1) * nx + ((i - 1) * 2 + 1 + 0)]
                    + resU[((j - 1) * 2 + 1 + 0) * nx + ((i - 1) * 2 + 1 + 1)]
                    + resU[((j - 1) * 2 + 1 + 1) * nx + ((i - 1) * 2 + 1 + 1)]);

            rhsVCoarser[j * nxCoarser + i] = scale * 0.25 * (
                    resV[((j - 1) * 2 + 1 + 0) * nx + ((i - 1) * 2 + 1 + 0)]
                    + resV[((j - 1) * 2 + 1 + 1) * nx + ((i - 1) * 2 + 1 + 0)]
                    + resV[((j - 1) * 2 + 1 + 0) * nx + ((i - 1) * 2 + 1 + 1)]
                    + resV[((j - 1) * 2 + 1 + 1) * nx + ((i - 1) * 2 + 1 + 1)]);
        }
    }
}


inline void coarsenOperator(size_t nxCoarser, size_t nyCoarser, size_t nx, size_t ny,
                            const double *__restrict__ ixix, const double *__restrict__ ixiy, const double *__restrict__ iyiy,
                            double *__restrict__ ixixCoarser, double *__restrict__ ixiyCoarser, double *__restrict__ iyiyCoarser) {

    auto scale = 1.;

#pragma omp parallel for schedule (static) collapse(2) if((nxCoarser - 2) * (nyCoarser - 2) >= ompThreshold)
    for (size_t j = 1; j < nyCoarser - 1; ++j) {
        for (size_t i = 1; i < nxCoarser - 1; ++i) {
            ixixCoarser[j * nxCoarser + i] = scale * 0.25 * (
                    ixix[((j - 1) * 2 + 1 + 0) * nx + ((i - 1) * 2 + 1 + 0)]
                    + ixix[((j - 1) * 2 + 1 + 1) * nx + ((i - 1) * 2 + 1 + 0)]
                    + ixix[((j - 1) * 2 + 1 + 0) * nx + ((i - 1) * 2 + 1 + 1)]
                    + ixix[((j - 1) * 2 + 1 + 1) * nx + ((i - 1) * 2 + 1 + 1)]);

            ixiyCoarser[j * nxCoarser + i] = scale * 0.25 * (
                    ixiy[((j - 1) * 2 + 1 + 0) * nx + ((i - 1) * 2 + 1 + 0)]
                    + ixiy[((j - 1) * 2 + 1 + 1) * nx + ((i - 1) * 2 + 1 + 0)]
                    + ixiy[((j - 1) * 2 + 1 + 0) * nx + ((i - 1) * 2 + 1 + 1)]
                    + ixiy[((j - 1) * 2 + 1 + 1) * nx + ((i - 1) * 2 + 1 + 1)]);

            iyiyCoarser[j * nxCoarser + i] = scale * 0.25 * (
                    iyiy[((j - 1) * 2 + 1 + 0) * nx + ((i - 1) * 2 + 1 + 0)]
                    + iyiy[((j - 1) * 2 + 1 + 1) * nx + ((i - 1) * 2 + 1 + 0)]
                    + iyiy[((j - 1) * 2 + 1 + 0) * nx + ((i - 1) * 2 + 1 + 1)]
                    + iyiy[((j - 1) * 2 + 1 + 1) * nx + ((i - 1) * 2 + 1 + 1)]);
        }
    }
}


inline void correction(size_t nx, size_t ny, size_t nxCoarser, size_t nyCoarser,
                       const double *__restrict__ uCoarser, const double *__restrict__ vCoarser,
                       double *__restrict__ u, double *__restrict__ v) {

    auto scale = 1.;

#pragma omp parallel for schedule (static) collapse(2) if((nx - 2) * (ny - 2) >= ompThreshold)
    for (size_t j = 1; j < ny - 1; ++j) {
        for (size_t i = 1; i < nx - 1; ++i) {
            u[j * nx + i] += scale * uCoarser[((j - 1) / 2 + 1) * nxCoarser + ((i - 1) / 2 + 1)];

            v[j * nx + i] += scale * vCoarser[((j - 1) / 2 + 1) * nxCoarser + ((i - 1) / 2 + 1)];
        }
    }
}


/*inline*/ void multigrid(size_t level, double omega,
                          double *__restrict__ *__restrict__ rhsU, double *__restrict__ *__restrict__ rhsV,
                          double *__restrict__ *__restrict__ solU, double *__restrict__ *__restrict__ solV,
                          double *__restrict__ *__restrict__ solNewU, double *__restrict__ *__restrict__ solNewV,
                          double *__restrict__ *__restrict__ resU, double *__restrict__ *__restrict__ resV,
                          double *__restrict__ *__restrict__ ixix, double *__restrict__ *__restrict__ ixiy, double **__restrict__ iyiy) {
    auto nx = (1u << level) + 2;
    auto ny = (1u << level) + 2;

    if (0 == level) {
        smooth(nx, ny, omega, solU[level], solV[level], solNewU[level], solNewV[level], ixix[level], ixiy[level], iyiy[level], rhsU[level], rhsV[level]);
        std::swap(solU[level], solNewU[level]);
        std::swap(solV[level], solNewV[level]);
        applyBC(nx, ny, solU[level], solV[level]);

        return;
    }

    auto nxCoarser = (1u << (level - 1)) + 2;
    auto nyCoarser = (1u << (level - 1)) + 2;

    // pre-smoothing
    for (auto it = 0; it < 5; ++it) {
        smooth(nx, ny, omega, solU[level], solV[level], solNewU[level], solNewV[level], ixix[level], ixiy[level], iyiy[level], rhsU[level], rhsV[level]);
        std::swap(solU[level], solNewU[level]);
        std::swap(solV[level], solNewV[level]);
        applyBC(nx, ny, solU[level], solV[level]);
    }

    // update residual
    updateRes(nx, ny, solU[level], solV[level], resU[level], resV[level], ixix[level], ixiy[level], iyiy[level], rhsU[level], rhsV[level]);

    // update coarser rhs
    updateCoarserRhs(nxCoarser, nyCoarser, nx, ny, resU[level], resV[level], rhsU[level - 1], rhsV[level - 1]);

    // set coarser solution to zero
    memset(solU[level - 1], 0, nxCoarser * nyCoarser * sizeof(double));
    memset(solV[level - 1], 0, nxCoarser * nyCoarser * sizeof(double));

    // compute coarse grid
    multigrid(level - 1, omega, rhsU, rhsV, solU, solV, solNewU, solNewV, resU, resV, ixix, ixiy, iyiy);

    // apply correction
    correction(nx, ny, nxCoarser, nyCoarser, solU[level - 1], solV[level - 1], solU[level], solV[level]);

    // post-smoothing
    for (auto it = 0; it < 5; ++it) {
        smooth(nx, ny, omega, solU[level], solV[level], solNewU[level], solNewV[level], ixix[level], ixiy[level], iyiy[level], rhsU[level], rhsV[level]);
        std::swap(solU[level], solNewU[level]);
        std::swap(solV[level], solNewV[level]);
        applyBC(nx, ny, solU[level], solV[level]);
    }
}
