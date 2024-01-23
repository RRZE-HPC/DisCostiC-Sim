void kernel(double *dst, double *src, const int dim_x, const int dim_y)
{
    for (int y = 0; y < dim_y; y++)
    for (int x = 0; x < dim_x; x++)
    dst[x + (y * dim_x)] = 0.25 * (((src[(x - 1) + (y * dim_x)] + src[(x + 1) + (y * dim_x)]) + src[x + ((y - 1) * dim_x)]) + src[x + ((y + 1) * dim_x)]);


}

