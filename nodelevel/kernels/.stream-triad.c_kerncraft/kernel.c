void kernel(double *a, double *b, double *c, double *s_, const int N)
{
  static double s;
  s = *s_;
    for (int i = 0; i < N; ++i)
  {
    a[i] = b[i] + (s * c[i]);
  }

  *s_ = s;
}

