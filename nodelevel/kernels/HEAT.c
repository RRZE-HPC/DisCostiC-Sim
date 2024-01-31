double dst[dim_y][dim_x],src[dim_y][dim_x];

 for(int y=0; y<dim_y; y++)
	 for(int x=0; x<dim_x; x++)
	 dst[y][x] = 0.25 * (src[y][x - 1] + src[y][x + 1] + src[y - 1][x] + src[y + 1][x]);
