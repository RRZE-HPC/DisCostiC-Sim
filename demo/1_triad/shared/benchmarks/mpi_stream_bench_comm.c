/*
 *    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
 *
 * Purpose: Measured STREAM Triad plus closed-ring MPI exchange benchmark for Stages 1–3 validation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mpi.h>

#define COMM_BUFSIZE_DEFAULT 20000  /* doubles — must match STREAM_FILE.hpp default */
#define STREAM_SCALAR 55.0
#define ALIGN_BYTES 64

static void *xaligned_alloc(size_t alignment, size_t size)
{
    void *ptr = NULL;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return NULL;
    }
    return ptr;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    if (argc < 3 || argc > 4) {
        if (rank == 0) {
            fprintf(stderr,
                    "Usage: %s <N_total> <ntimes> [comm_buf_doubles]\n"
                    "  ntimes must equal config.cfg number_of_timesteps\n"
                    "  comm_buf_doubles defaults to %d\n",
                    argv[0], COMM_BUFSIZE_DEFAULT);
        }
        MPI_Finalize();
        return 1;
    }

    long n_total = atol(argv[1]);
    int ntimes = atoi(argv[2]);
    int comm_bufsize = (argc == 4) ? atoi(argv[3]) : COMM_BUFSIZE_DEFAULT;
    const double scalar = STREAM_SCALAR;

    if (comm_bufsize <= 0) {
        if (rank == 0) {
            fprintf(stderr, "comm_buf_doubles must be positive (got %d)\n", comm_bufsize);
        }
        MPI_Finalize();
        return 1;
    }

    long n = n_total / nprocs;

    if (rank == 0) {
        printf("N_total=%ld  nprocs=%d  N_per_rank=%ld  NTIMES=%d  scalar=%.1f  comm_buf=%d doubles\n",
               n_total, nprocs, n, ntimes, scalar, comm_bufsize);
    }

    size_t nbytes = (size_t)n * sizeof(double);
    double *a = (double *)xaligned_alloc(ALIGN_BYTES, nbytes);
    double *b = (double *)xaligned_alloc(ALIGN_BYTES, nbytes);
    double *c = (double *)xaligned_alloc(ALIGN_BYTES, nbytes);
    double *sbuf_top = (double *)xaligned_alloc(ALIGN_BYTES, (size_t)comm_bufsize * sizeof(double));
    double *rbuf_top = (double *)xaligned_alloc(ALIGN_BYTES, (size_t)comm_bufsize * sizeof(double));
    double *sbuf_bottom = (double *)xaligned_alloc(ALIGN_BYTES, (size_t)comm_bufsize * sizeof(double));
    double *rbuf_bottom = (double *)xaligned_alloc(ALIGN_BYTES, (size_t)comm_bufsize * sizeof(double));

    if (!a || !b || !c || !sbuf_top || !rbuf_top || !sbuf_bottom || !rbuf_bottom) {
        fprintf(stderr, "posix_memalign failed on rank %d: %s\n", rank, strerror(errno));
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    for (long i = 0; i < n; i++) {
        a[i] = 1.0;
        b[i] = 2.0;
        c[i] = 0.5;
    }
    for (int i = 0; i < comm_bufsize; i++) {
        sbuf_top[i] = sbuf_bottom[i] = (double)rank;
    }

    /* Warm-up (outside timed region) */
    for (long i = 0; i < n; i++) {
        a[i] = b[i] + scalar * c[i];
    }
    __asm__ volatile("" ::: "memory");

    int top = (rank == nprocs - 1) ? 0 : rank + 1;
    int bottom = (rank == 0) ? nprocs - 1 : rank - 1;

    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = MPI_Wtime();

    for (int r = 0; r < ntimes; r++) {
        for (long i = 0; i < n; i++) {
            a[i] = b[i] + scalar * c[i];
        }
        __asm__ volatile("" ::: "memory");
        double *tmp = a;
        a = b;
        b = tmp;

        MPI_Request reqs[4] = {MPI_REQUEST_NULL, MPI_REQUEST_NULL,
                               MPI_REQUEST_NULL, MPI_REQUEST_NULL};
        MPI_Isend(sbuf_top, comm_bufsize, MPI_DOUBLE, top, 1, MPI_COMM_WORLD, &reqs[0]);
        MPI_Irecv(rbuf_top, comm_bufsize, MPI_DOUBLE, top, 2, MPI_COMM_WORLD, &reqs[1]);
        MPI_Isend(sbuf_bottom, comm_bufsize, MPI_DOUBLE, bottom, 2, MPI_COMM_WORLD, &reqs[2]);
        MPI_Irecv(rbuf_bottom, comm_bufsize, MPI_DOUBLE, bottom, 1, MPI_COMM_WORLD, &reqs[3]);
        MPI_Waitall(4, reqs, MPI_STATUSES_IGNORE);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double elapsed = MPI_Wtime() - t0;

    double local_sum = 0.0;
    for (long i = 0; i < n; i++) {
        local_sum += a[i];
    }

    double global_sum, max_elapsed;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&elapsed, &max_elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double bw_gb_s = (double)n_total * 4.0 * sizeof(double) * ntimes
                         / max_elapsed / 1e9;
        double gflops = 2.0 * (double)n_total * ntimes / max_elapsed / 1e9;
        printf("wall_seconds=%.6f  bw_gb_s=%.3f  gflops=%.6f  check=%.6e\n",
               max_elapsed, bw_gb_s, gflops, global_sum);
    }

    free(a);
    free(b);
    free(c);
    free(sbuf_top);
    free(rbuf_top);
    free(sbuf_bottom);
    free(rbuf_bottom);
    MPI_Finalize();
    return 0;
}
