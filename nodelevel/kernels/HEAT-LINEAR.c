double src[M*N];
double dst[M*N];

int start_x = 1, end_x = M - 1;
int start_y = 1, end_y = N - 1;


for (int y = start_y; y < end_y; ++y)
    for (int x = start_x; x < end_x; ++x)
        dst[y * M + x] = 0.25 * (src[y * M + x - 1] + src[y * M + x + 1] + src[(y - 1) * M + x] + src[(y + 1) * M + x]);
        
    