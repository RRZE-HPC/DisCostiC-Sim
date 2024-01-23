double s, dst[dim_y][dim_x],src[dim_y][dim_x];

 for(int y=0; y<dim_y; y++)
	 for(int x=0; x<dim_x; x++)
	 dst[y * dim_x + x] = s * (src[y * dim_x + x - 1] + src[y * dim_x + x + 1] + src[(y - 1) * dim_x + x] + src[(y + 1) * dim_x + x]);
