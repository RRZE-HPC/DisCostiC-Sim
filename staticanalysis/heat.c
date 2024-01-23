/* =======================================================================================
 *
 *  Authors:   Markus Wittmann (mw), markus.wittmann@fau.de
 *  Copyright (c) 2022 NHR@FAU, University Erlangen-Nuremberg
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 * =======================================================================================
 */

/*

  heat: a simple Jacobi 2D stencil sweep with vertical ghost cell exchange

  usage: heat [global_dim_y] [global_dim_x] [iterations]

  by default global_dim_y = 100, global_dim_x = 100, iterations = 1000

  - (global_)dim_x: width of domain/grid
  - (global_)dim_y: height of domain/grid
  - decompose domain only **vertically**, see example below
  - data layout
    - one chunk of memory holds ghost cells and inner cells of the grid
    - row major layout
    - cells start bottom left, go to top right
    - pointer to ghost_cells_top/ghost_cells_bottom point to start of ghost
      cells
      - pointer to ghost cells is NULL if they are not available, e.g. for
        rank 0 and rank size-1
    - pointer to inner_cells points to start of inner cells
    - ghost cells are at the bottom/top of the grid

    - sample data layout:
      I: inner cells
      T: top ghost cells
      B: bottom ghost cells

      ghost_cells_top    -> T T T T
                            I I I I
                            . . . .
      inner_cells        -> I I I I
      ghost_cells_bottom -> B B B B

  - domain decomposed over three MPI ranks:
    rank
    2     I I I I
          .......
          I I I I
          B B B B

    1     T T T T
          I I I I
          .......
          I I I I
          B B B B

    0     T T T T
          I I I I
          . . . .
          I I I I
*/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <mpi.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const double V_DEFAULT = 0.1;
const double V_TOP = 1.0;
const double V_RIGHT = 2.0;
const double V_BOTTOM = 3.0;
const double V_LEFT = 4.0;
const double V_MAX = 4.0;

struct grid_t
{
    double *data;               /* pointer to start of data */
    double *inner_cells;        /* pointer to inner cells of process' local part of the domain */
    double *ghost_cells_bottom; /* pointer to bottom ghost cells, if available on this process */
    double *ghost_cells_top;    /* pointer to top ghost cells, if available on this process */
};

struct domain_t
{
    int x;                     /* process local x location in domain */
    int y;                     /* process local y location in domain */
    int dim_x;                 /* process local x dimension */
    int dim_y;                 /* process local y dimension */
    int global_dim_x;          /* global x dimension of domain */
    int global_dim_y;          /* global y dimension of domain */
    struct grid_t grids[2];    /* two grids */
    int comm_rank;             /* process' rank in MPI_COMM_WORLD */
    int comm_size;             /* size of MPI_COMM_WORLD */
    int iterations_performed;  /* no. of iterations performed on grids */
    int iterations_to_perform; /* no. of iterations to perform */
    int active_grid;           /* index of the grid which currently acts as source grid */
};

static int
max_int(int a, int b)
{
    return a <= b ? a : b;
}

/**
 * @brief Collective routine that writs the domain as a PGM file.
 *
 * Simple implementation for writing the domain as PGM file.
 * No MPI comminucation or IO is involved.
 *
 * @param domain    The domain to dump.
 */
static void
dump_domain(struct domain_t *domain)
{
    int rank = domain->comm_rank;
    int dim_x = domain->dim_x;
    int dim_y = domain->dim_y;
    double *cells = domain->grids[domain->active_grid].inner_cells;

    char *file_name = NULL;
    int err = asprintf(&file_name, "domain-%08d.pgm", domain->iterations_performed);
    if (err == -1)
    {
        fprintf(stderr, "[%d] ERROR: asprintf failed.\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (rank == 0)
    {
        FILE *f = fopen(file_name, "wb");

        if (f == NULL)
        {
            fprintf(stderr, "[%d] ERROR: open file for writing failed.\n", rank);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        fprintf(f, "P5\n%d %d\n255\n", domain->global_dim_x, domain->global_dim_y);

        fclose(f);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    for (int r = domain->comm_size - 1; r >= 0; --r)
    {

        if (r == rank)
        {
            FILE *f = fopen(file_name, "ab");

            if (f == NULL)
            {
                fprintf(stderr, "[%d] ERROR: open file for writing failed.\n", rank);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }

            for (int y = dim_y - 1; y >= 0; --y)
            {
                for (int x = 0; x < dim_x; ++x)
                {
                    unsigned char gray = (unsigned char)(255.0 * (cells[y * dim_x + x] / V_MAX));
                    fwrite(&gray, sizeof(gray), 1, f);
                }
            }

            fclose(f);
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }

    free(file_name);
}

/**
 * @brief Initialize grid @grid_idx in domain.
 * @param domain        The domain that contains the grids.
 * @param grid_idx      The grid in @p domain to initialize.
 */
static void
init_grid_data(struct domain_t *domain, int grid_idx)
{
    int dim_x = domain->dim_x;
    int dim_y = domain->dim_y;
    struct grid_t *grid = &domain->grids[grid_idx];
    double *cells = grid->inner_cells;

    for (int i = 0; i < dim_x * dim_y; ++i)
    {
        cells[i] = V_DEFAULT;
    }

    for (int i = 0; i < dim_y; ++i)
    {
        cells[dim_x * i] = V_LEFT;
        cells[dim_x * (i + 1) - 1] = V_RIGHT;
    }

    if (!grid->ghost_cells_top)
    {
        for (int i = 0; i < dim_x; ++i)
        {
            cells[dim_x * (dim_y - 1) + i] = V_TOP;
        }
    }

    if (!grid->ghost_cells_bottom)
    {
        for (int i = 0; i < dim_x; ++i)
        {
            cells[i] = V_BOTTOM;
        }
    }
}

/**
 * @brief Allocate and initialize domain and grids. Aborts on error.
 * @param argc
 * @param argv
 * @param rank  The rank the current MPI process in MPI_COMM_WORLD.
 * @param size  The size of MPI_COMM_WORLD.
 * @return an initialized domain or stopps execution on error.
 */
struct domain_t *
init(int argc, char *argv[], int rank, int size)
{
    int global_dim_x = 100;
    int global_dim_y = 100;
    int iterations_to_perform = 1000;

    if (argc > 1)
    {
        global_dim_y = atoi(argv[1]);
    }

    if (argc > 2)
    {
        global_dim_x = atoi(argv[2]);
    }

    if (argc > 3)
    {
        iterations_to_perform = atoi(argv[3]);
    }

    int dim_x = global_dim_x;
    int dim_y = global_dim_y / size;

    int x = 0;
    int y = rank * dim_y;

    if (global_dim_y % size != 0)
    {
        y += max_int(rank, global_dim_y % size);
        if (rank < global_dim_y % size)
        {
            ++dim_y;
        }
    }

    int have_ghost_cells_top = rank + 1 == size ? 0 : 1;
    int have_ghost_cells_bottom = rank == 0 ? 0 : 1;
    int n_cells = dim_x * (dim_y + have_ghost_cells_bottom + have_ghost_cells_top);

    int initialization_failed = 0;

    struct domain_t *domain = NULL;
    if (!initialization_failed)
    {
        domain = malloc(sizeof(struct domain_t));
        if (domain == NULL)
        {
            fprintf(stderr, "[%d] malloc failed: %d - %s\n",
                    rank, errno, strerror(errno));
            initialization_failed = 1;
        }
    }

    double *data[2] = {NULL, NULL};

    if (!initialization_failed)
    {
        data[0] = malloc(sizeof(double) * n_cells);
        data[1] = malloc(sizeof(double) * n_cells);
        if (data[0] == NULL || data[1] == NULL)
        {
            fprintf(stderr, "[%d] malloc failed: %d - %s\n",
                    rank, errno, strerror(errno));
            initialization_failed = 1;
        }
    }

    else
    {
        int failed_initializations = 0;
        MPI_Allreduce(&initialization_failed, &failed_initializations,
                      1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        if (failed_initializations)
        {
            if (rank == 0)
            {
                fprintf(stderr, "ERROR: initialization failed in %d processes. Stopping.\n",
                        failed_initializations);
            }
            free(data[0]);
            free(data[1]);
            free(domain);
            MPI_Finalize();
            exit(EXIT_FAILURE);
        }
    }

    domain->comm_rank = rank;
    domain->comm_size = size;

    domain->x = x;
    domain->y = y;
    domain->dim_x = dim_x;
    domain->dim_y = dim_y;
    domain->global_dim_x = global_dim_x;
    domain->global_dim_y = global_dim_y;
    domain->active_grid = 0;
    domain->iterations_performed = 0;
    domain->iterations_to_perform = iterations_to_perform;

    for (int i = 0; i < 2; ++i)
    {
        domain->grids[i].data = data[i];
        domain->grids[i].inner_cells = data[i] + (have_ghost_cells_bottom * dim_x);
        domain->grids[i].ghost_cells_bottom = have_ghost_cells_bottom ? data[i] : NULL;
        domain->grids[i].ghost_cells_top = have_ghost_cells_top ? data[i] + (dim_y + have_ghost_cells_bottom) * dim_x : NULL;
    }

    for (int i = 0; i < 2; ++i)
    {
        init_grid_data(domain, i);
    }

    if (rank == 0)
    {
        printf("[%3d] global domain %d x %d, performing %d iterations\n",
               domain->comm_rank, domain->global_dim_y,
               domain->global_dim_x, domain->iterations_to_perform);
    }

    printf("[%3d] at (%d,%d) local domain %d x %d, with ghost cells: %d x %d  ghost cells at bottom: %d at top: %d\n",
           rank,
           x, y,
           dim_y, dim_x,
           dim_y + have_ghost_cells_bottom + have_ghost_cells_top, dim_x,
           have_ghost_cells_bottom, have_ghost_cells_top);

    return domain;
}

/**
 * @brief Deinitialize and free resources associated with @p domain.
 * @param domain        The domain to free.
 */
static void
deinit(struct domain_t *domain)
{
    if (domain)
    {
        for (int i = 0; i < 2; ++i)
        {
            if (domain->grids[i].data)
            {
                free(domain->grids[i].data);
            }
        }
        free(domain);
    }
}

/**
 * @brief Exchange ghost cells.
 * @param domain        The domain to use.
 * @param grid_idx      The grid in @p domain to use.
 */
static void
exchange(struct domain_t *domain, int grid_idx)
{
    int dim_x = domain->dim_x;
    int dim_y = domain->dim_y;
    struct grid_t *grid = &domain->grids[grid_idx];
    double *src = grid->inner_cells;

    MPI_Request requests[4] = {MPI_REQUEST_NULL, MPI_REQUEST_NULL, MPI_REQUEST_NULL, MPI_REQUEST_NULL};

    /* exchange ghost cells with top neighbor */
    if (domain->comm_rank + 1 < domain->comm_size)
    {
        int top = domain->comm_rank + 1;
        MPI_Isend(&src[(dim_y - 1) * dim_x], dim_x, MPI_DOUBLE, top, 1, MPI_COMM_WORLD, &requests[0]);
        MPI_Irecv(grid->ghost_cells_top, dim_x, MPI_DOUBLE, top, 2, MPI_COMM_WORLD, &requests[1]);
    }

    /* exchange ghost cells with bottom neighbor */
    if (domain->comm_rank > 0)
    {
        int bottom = domain->comm_rank - 1;
        MPI_Isend(&src[0], dim_x, MPI_DOUBLE, bottom, 2, MPI_COMM_WORLD, &requests[2]);
        MPI_Irecv(grid->ghost_cells_bottom, dim_x, MPI_DOUBLE, bottom, 1, MPI_COMM_WORLD, &requests[3]);
    }

    MPI_Waitall(4, requests, MPI_STATUSES_IGNORE);

    return;
}

/**
 * @brief Perform one sweep over the domain.
 * @param domain        The domain to use.
 * @param src_grid_idx  Source grid index in @p domain.
 * @param dst_grid_idx  Destination grid index in @p domain.
 */
static void
relax(struct domain_t *domain, int src_grid_idx, int dst_grid_idx)
{
    int dim_x = domain->dim_x;
    int dim_y = domain->dim_y;
    struct grid_t *grid = &domain->grids[src_grid_idx];
    double *src = domain->grids[src_grid_idx].inner_cells;
    double *dst = domain->grids[dst_grid_idx].inner_cells;

    int start_x = 1, end_x = dim_x - 1;
    int start_y = 1, end_y = dim_y - 1;

    if (grid->ghost_cells_bottom)
    {
        start_y = 0;
    }

    if (grid->ghost_cells_top)
    {
        end_y = dim_y;
    }

    for (int y = start_y; y < end_y; ++y)
    {
        for (int x = start_x; x < end_x; ++x)
        {
            dst[y * dim_x + x] = 0.25 * (src[y * dim_x + x - 1] + src[y * dim_x + x + 1] + src[(y - 1) * dim_x + x] + src[(y + 1) * dim_x + x]);
        }
    }
}

/**
 * @brief Perform a fixed amount of iterations over given domain.
 *
 * @param domain    The domain to iterate over.
 *
 * A single iteration consists of a ghost cell exchange and a relaxation step.
 * The active_grid and iterations_performed member of @p domain will be updated
 * upon return.
 */
static double
iterate(struct domain_t *domain)
{
    int n_iterations = domain->iterations_to_perform;
    int src_grid_idx = domain->active_grid;
    int dst_grid_idx = (src_grid_idx + 1) % 2;

    double t = MPI_Wtime();

    for (int iter = 0; iter < n_iterations; ++iter)
    {
        exchange(domain, src_grid_idx);
        relax(domain, src_grid_idx, dst_grid_idx);

        int tmp = src_grid_idx;
        src_grid_idx = dst_grid_idx;
        dst_grid_idx = tmp;

        ++domain->iterations_performed;
    }

    t = MPI_Wtime() - t;

    domain->active_grid = src_grid_idx;

    return t;
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank = -1;
    int size = -1;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    struct domain_t *domain = init(argc, argv, rank, size);

    // dump_domain(domain);

    double runtime = iterate(domain);

    // dump_domain(domain);

    deinit(domain);

    if (0 == rank)
        printf("[%3d] Runtime: %.2f sec\n", rank, runtime);

    MPI_Finalize();

    return EXIT_SUCCESS;
}
