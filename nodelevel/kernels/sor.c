double r1;
double res;
double dx;
double dy;
double dx2;
double dy2;
double idx2;
double idy2;
double omega;
double factor;
double src[imax][imax];
double rhs[imax][imax];
for (int j = 1; j < jmaxLocal + 1; j++)
{
    for (int i = 1; i < imax + 1; i++)
    {
        r1 = rhs[j][i] - ((src[j][i - 1] - 2.0 * src[j][i] + src[j][i + 1]) * idx2 + (src[j - 1][i] - 2.0 * src[j][i] + src[j + 1][i]) * idy2);
        src[j][i] = src[j][i] - (factor * r1);
        res = res + (r1 * r1);
    }
}