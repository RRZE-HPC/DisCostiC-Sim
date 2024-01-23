for (int level = 1; level < numberOfMgLevels; ++level) {
GenerateCoarseProblem(*curLevelMatrix);
void GenerateCoarseProblem
global_int_t nxf = Af.geom->nx;
global_int_t nyf = Af.geom->ny;
global_int_t nzf = Af.geom->nz;
local_int_t nxc, nyc, nzc;
assert(nxf%2==0); assert(nyf%2==0); assert(nzf%2==0);
nxc = nxf/2; nyc = nyf/2; nzc = nzf/2;
local_int_t * f2cOperator = new local_int_t[Af.localNumberOfRows];
local_int_t localNumberOfRows = nxc*nyc*nzc;
assert(localNumberOfRows>0);
for (local_int_t i=0; i< localNumberOfRows; ++i) {
f2cOperator[i] = 0;
}
for (local_int_t izc=0; izc<nzc; ++izc) {
local_int_t izf = 2*izc;
for (local_int_t iyc=0; iyc<nyc; ++iyc) {
local_int_t iyf = 2*iyc;
for (local_int_t ixc=0; ixc<nxc; ++ixc) {
local_int_t ixf = 2*ixc;
local_int_t currentCoarseRow = izc*nxc*nyc+iyc*nxc+ixc;
local_int_t currentFineRow = izf*nxf*nyf+iyf*nxf+ixf;
f2cOperator[currentCoarseRow] = currentFineRow;
}
}
}
Geometry * geomc = new Geometry;
local_int_t zlc = 0;
local_int_t zuc = 0;
int pz = Af.geom->pz;
if (pz>0) {
zlc = Af.geom->partz_nz[0]/2;
zuc = Af.geom->partz_nz[1]/2;
}
GenerateGeometry(Af.geom->size, Af.geom->rank, Af.geom->numThreads, Af.geom->pz, zlc, zuc, nxc, nyc, nzc, Af.geom->npx, Af.geom->npy, Af.geom->npz, geomc);
SparseMatrix * Ac = new SparseMatrix;
InitializeSparseMatrix(*Ac, geomc);
GenerateProblem(*Ac, 0, 0, 0);
SetupHalo(*Ac);
Vector *rc = new Vector;
Vector *xc = new Vector;
Vector * Axf = new Vector;
InitializeVector(*rc, Ac->localNumberOfRows);
InitializeVector(*xc, Ac->localNumberOfColumns);
InitializeVector(*Axf, Af.localNumberOfColumns);
Af.Ac = Ac;
MGData * mgData = new MGData;
InitializeMGData(f2cOperator, rc, xc, Axf, *mgData);
Af.mgData = mgData;
return;
}
void GenerateCoarseProblem
global_int_t nxf = Af.geom->nx;
global_int_t nyf = Af.geom->ny;
global_int_t nzf = Af.geom->nz;
local_int_t nxc, nyc, nzc;
assert(nxf%2==0); assert(nyf%2==0); assert(nzf%2==0);
nxc = nxf/2; nyc = nyf/2; nzc = nzf/2;
local_int_t * f2cOperator = new local_int_t[Af.localNumberOfRows];
local_int_t localNumberOfRows = nxc*nyc*nzc;
assert(localNumberOfRows>0);
for (local_int_t i=0; i< localNumberOfRows; ++i) {
f2cOperator[i] = 0;
}
for (local_int_t izc=0; izc<nzc; ++izc) {
local_int_t izf = 2*izc;
for (local_int_t iyc=0; iyc<nyc; ++iyc) {
local_int_t iyf = 2*iyc;
for (local_int_t ixc=0; ixc<nxc; ++ixc) {
local_int_t ixf = 2*ixc;
local_int_t currentCoarseRow = izc*nxc*nyc+iyc*nxc+ixc;
local_int_t currentFineRow = izf*nxf*nyf+iyf*nxf+ixf;
f2cOperator[currentCoarseRow] = currentFineRow;
}
}
}
Geometry * geomc = new Geometry;
local_int_t zlc = 0;
local_int_t zuc = 0;
int pz = Af.geom->pz;
if (pz>0) {
zlc = Af.geom->partz_nz[0]/2;
zuc = Af.geom->partz_nz[1]/2;
}
GenerateGeometry(Af.geom->size, Af.geom->rank, Af.geom->numThreads, Af.geom->pz, zlc, zuc, nxc, nyc, nzc, Af.geom->npx, Af.geom->npy, Af.geom->npz, geomc);
SparseMatrix * Ac = new SparseMatrix;
InitializeSparseMatrix(*Ac, geomc);
GenerateProblem(*Ac, 0, 0, 0);
SetupHalo(*Ac);
Vector *rc = new Vector;
Vector *xc = new Vector;
Vector * Axf = new Vector;
InitializeVector(*rc, Ac->localNumberOfRows);
InitializeVector(*xc, Ac->localNumberOfColumns);
InitializeVector(*Axf, Af.localNumberOfColumns);
Af.Ac = Ac;
MGData * mgData = new MGData;
InitializeMGData(f2cOperator, rc, xc, Axf, *mgData);
Af.mgData = mgData;
return;
}
curLevelMatrix = curLevelMatrix->Ac;
}
for (int level = 0; level < numberOfMgLevels; ++level) {
CheckProblem(*curLevelMatrix, curb, curx, curxexact);
using std::endl;
void CheckProblem
global_int_t nx = A.geom->nx;
global_int_t ny = A.geom->ny;
global_int_t nz = A.geom->nz;
global_int_t gnx = A.geom->gnx;
global_int_t gny = A.geom->gny;
global_int_t gnz = A.geom->gnz;
global_int_t gix0 = A.geom->gix0;
global_int_t giy0 = A.geom->giy0;
global_int_t giz0 = A.geom->giz0;
local_int_t localNumberOfRows = nx*ny*nz;
global_int_t totalNumberOfRows = gnx*gny*gnz;
double * bv = 0;
double * xv = 0;
double * xexactv = 0;
if (b!=0) bv = b->values;
if (x!=0) xv = x->values;
if (xexact!=0) xexactv = xexact->values;
local_int_t localNumberOfNonzeros = 0;
for (local_int_t iz=0; iz<nz; iz++) {
global_int_t giz = giz0+iz;
for (local_int_t iy=0; iy<ny; iy++) {
global_int_t giy = giy0+iy;
for (local_int_t ix=0; ix<nx; ix++) {
global_int_t gix = gix0+ix;
local_int_t currentLocalRow = iz*nx*ny+iy*nx+ix;
global_int_t currentGlobalRow = giz*gnx*gny+giy*gnx+gix;
assert(A.localToGlobalMap[currentLocalRow] == currentGlobalRow);
HPCG_fout << " rank, globalRow, localRow = " << A.geom->rank << " " << currentGlobalRow << " " << A.globalToLocalMap.find(currentGlobalRow)->second << endl;
char numberOfNonzerosInRow = 0;
double * currentValuePointer = A.matrixValues[currentLocalRow];
global_int_t * currentIndexPointerG = A.mtxIndG[currentLocalRow];
for (int sz=-1; sz<=1; sz++) {
if (giz+sz>-1 && giz+sz<gnz) {
for (int sy=-1; sy<=1; sy++) {
if (giy+sy>-1 && giy+sy<gny) {
for (int sx=-1; sx<=1; sx++) {
if (gix+sx>-1 && gix+sx<gnx) {
global_int_t curcol = currentGlobalRow+sz*gnx*gny+sy*gnx+sx;
if (curcol==currentGlobalRow) {
assert(A.matrixDiagonal[currentLocalRow] == currentValuePointer);
assert(*currentValuePointer++ == 26.0);
} else {
assert(*currentValuePointer++ == -1.0);
}
assert(*currentIndexPointerG++ == curcol);
numberOfNonzerosInRow++;
}
}
}
}
}
}
assert(A.nonzerosInRow[currentLocalRow] == numberOfNonzerosInRow);
localNumberOfNonzeros += numberOfNonzerosInRow;
if (b!=0)      assert(bv[currentLocalRow] == 26.0 - ((double) (numberOfNonzerosInRow-1)));
if (x!=0)      assert(xv[currentLocalRow] == 0.0);
if (xexact!=0) assert(xexactv[currentLocalRow] == 1.0);
}
}
}
HPCG_fout     << "Process " << A.geom->rank << " of " << A.geom->size <<" has " << localNumberOfRows    << " rows."     << endl << "Process " << A.geom->rank << " of " << A.geom->size <<" has " << localNumberOfNonzeros<< " nonzeros." <<endl;
global_int_t totalNumberOfNonzeros = 0;
MPI_Allreduce(&localNumberOfNonzeros, &totalNumberOfNonzeros, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
long long lnnz = localNumberOfNonzeros, gnnz = 0;
MPI_Allreduce(&lnnz, &gnnz, 1, MPI_LONG_LONG_INT, MPI_SUM, MPI_COMM_WORLD);
totalNumberOfNonzeros = gnnz;
totalNumberOfNonzeros = localNumberOfNonzeros;
assert(A.totalNumberOfRows == totalNumberOfRows);
assert(A.totalNumberOfNonzeros == totalNumberOfNonzeros);
assert(A.localNumberOfRows == localNumberOfRows);
assert(A.localNumberOfNonzeros == localNumberOfNonzeros);
return;
}
using std::endl;
void CheckProblem
global_int_t nx = A.geom->nx;
global_int_t ny = A.geom->ny;
global_int_t nz = A.geom->nz;
global_int_t gnx = A.geom->gnx;
global_int_t gny = A.geom->gny;
global_int_t gnz = A.geom->gnz;
global_int_t gix0 = A.geom->gix0;
global_int_t giy0 = A.geom->giy0;
global_int_t giz0 = A.geom->giz0;
local_int_t localNumberOfRows = nx*ny*nz;
global_int_t totalNumberOfRows = gnx*gny*gnz;
double * bv = 0;
double * xv = 0;
double * xexactv = 0;
if (b!=0) bv = b->values;
if (x!=0) xv = x->values;
if (xexact!=0) xexactv = xexact->values;
local_int_t localNumberOfNonzeros = 0;
for (local_int_t iz=0; iz<nz; iz++) {
global_int_t giz = giz0+iz;
for (local_int_t iy=0; iy<ny; iy++) {
global_int_t giy = giy0+iy;
for (local_int_t ix=0; ix<nx; ix++) {
global_int_t gix = gix0+ix;
local_int_t currentLocalRow = iz*nx*ny+iy*nx+ix;
global_int_t currentGlobalRow = giz*gnx*gny+giy*gnx+gix;
assert(A.localToGlobalMap[currentLocalRow] == currentGlobalRow);
HPCG_fout << " rank, globalRow, localRow = " << A.geom->rank << " " << currentGlobalRow << " " << A.globalToLocalMap.find(currentGlobalRow)->second << endl;
char numberOfNonzerosInRow = 0;
double * currentValuePointer = A.matrixValues[currentLocalRow];
global_int_t * currentIndexPointerG = A.mtxIndG[currentLocalRow];
for (int sz=-1; sz<=1; sz++) {
if (giz+sz>-1 && giz+sz<gnz) {
for (int sy=-1; sy<=1; sy++) {
if (giy+sy>-1 && giy+sy<gny) {
for (int sx=-1; sx<=1; sx++) {
if (gix+sx>-1 && gix+sx<gnx) {
global_int_t curcol = currentGlobalRow+sz*gnx*gny+sy*gnx+sx;
if (curcol==currentGlobalRow) {
assert(A.matrixDiagonal[currentLocalRow] == currentValuePointer);
assert(*currentValuePointer++ == 26.0);
} else {
assert(*currentValuePointer++ == -1.0);
}
assert(*currentIndexPointerG++ == curcol);
numberOfNonzerosInRow++;
}
}
}
}
}
}
assert(A.nonzerosInRow[currentLocalRow] == numberOfNonzerosInRow);
localNumberOfNonzeros += numberOfNonzerosInRow;
if (b!=0)      assert(bv[currentLocalRow] == 26.0 - ((double) (numberOfNonzerosInRow-1)));
if (x!=0)      assert(xv[currentLocalRow] == 0.0);
if (xexact!=0) assert(xexactv[currentLocalRow] == 1.0);
}
}
}
HPCG_fout     << "Process " << A.geom->rank << " of " << A.geom->size <<" has " << localNumberOfRows    << " rows."     << endl << "Process " << A.geom->rank << " of " << A.geom->size <<" has " << localNumberOfNonzeros<< " nonzeros." <<endl;
global_int_t totalNumberOfNonzeros = 0;
MPI_Allreduce(&localNumberOfNonzeros, &totalNumberOfNonzeros, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
long long lnnz = localNumberOfNonzeros, gnnz = 0;
MPI_Allreduce(&lnnz, &gnnz, 1, MPI_LONG_LONG_INT, MPI_SUM, MPI_COMM_WORLD);
totalNumberOfNonzeros = gnnz;
totalNumberOfNonzeros = localNumberOfNonzeros;
assert(A.totalNumberOfRows == totalNumberOfRows);
assert(A.totalNumberOfNonzeros == totalNumberOfNonzeros);
assert(A.localNumberOfRows == localNumberOfRows);
assert(A.localNumberOfNonzeros == localNumberOfNonzeros);
return;
}
curLevelMatrix = curLevelMatrix->Ac;
curb = 0;
curx = 0;
curxexact = 0;
}
for (int i = 0; i < numberOfCalls; ++i) {
ierr = ComputeSPMV_ref(A, x_overlap, b_computed);
int ComputeSPMV_ref
assert(x.localLength>=A.localNumberOfColumns);
assert(y.localLength>=A.localNumberOfRows);
ExchangeHalo(A,x);
const double * const xv = x.values;
double * const yv = y.values;
const local_int_t nrow = A.localNumberOfRows;
for (local_int_t i=0; i< nrow; i++)  {
double sum = 0.0;
const double * const cur_vals = A.matrixValues[i];
const local_int_t * const cur_inds = A.mtxIndL[i];
const int cur_nnz = A.nonzerosInRow[i];
for (int j=0; j< cur_nnz; j++) sum += cur_vals[j]*xv[cur_inds[j]];
yv[i] = sum;
}
return 0;
}
int ComputeSPMV_ref
assert(x.localLength>=A.localNumberOfColumns);
assert(y.localLength>=A.localNumberOfRows);
ExchangeHalo(A,x);
const double * const xv = x.values;
double * const yv = y.values;
const local_int_t nrow = A.localNumberOfRows;
for (local_int_t i=0; i< nrow; i++)  {
double sum = 0.0;
const double * const cur_vals = A.matrixValues[i];
const local_int_t * const cur_inds = A.mtxIndL[i];
const int cur_nnz = A.nonzerosInRow[i];
for (int j=0; j< cur_nnz; j++) sum += cur_vals[j]*xv[cur_inds[j]];
yv[i] = sum;
}
return 0;
}
ierr = ComputeMG_ref(A, b_computed, x_overlap);
int ComputeMG_ref
assert(x.localLength==A.localNumberOfColumns);
ZeroVector(x);
int ierr = 0;
if (A.mgData!=0) {
int numberOfPresmootherSteps = A.mgData->numberOfPresmootherSteps;
for (int i=0; i< numberOfPresmootherSteps; ++i) ierr += ComputeSYMGS_ref(A, r, x);
if (ierr!=0) return ierr;
ierr = ComputeSPMV_ref(A, x, *A.mgData->Axf); if (ierr!=0) return ierr;
ierr = ComputeRestriction_ref(A, r);  if (ierr!=0) return ierr;
ierr = ComputeMG_ref(*A.Ac,*A.mgData->rc, *A.mgData->xc);  if (ierr!=0) return ierr;
ierr = ComputeProlongation_ref(A, x);  if (ierr!=0) return ierr;
int numberOfPostsmootherSteps = A.mgData->numberOfPostsmootherSteps;
for (int i=0; i< numberOfPostsmootherSteps; ++i) ierr += ComputeSYMGS_ref(A, r, x);
if (ierr!=0) return ierr;
}
else {
ierr = ComputeSYMGS_ref(A, r, x);
if (ierr!=0) return ierr;
}
return 0;
}
int ComputeMG_ref
assert(x.localLength==A.localNumberOfColumns);
ZeroVector(x);
int ierr = 0;
if (A.mgData!=0) {
int numberOfPresmootherSteps = A.mgData->numberOfPresmootherSteps;
for (int i=0; i< numberOfPresmootherSteps; ++i) ierr += ComputeSYMGS_ref(A, r, x);
if (ierr!=0) return ierr;
ierr = ComputeSPMV_ref(A, x, *A.mgData->Axf); if (ierr!=0) return ierr;
ierr = ComputeRestriction_ref(A, r);  if (ierr!=0) return ierr;
ierr = ComputeMG_ref(*A.Ac,*A.mgData->rc, *A.mgData->xc);  if (ierr!=0) return ierr;
ierr = ComputeProlongation_ref(A, x);  if (ierr!=0) return ierr;
int numberOfPostsmootherSteps = A.mgData->numberOfPostsmootherSteps;
for (int i=0; i< numberOfPostsmootherSteps; ++i) ierr += ComputeSYMGS_ref(A, r, x);
if (ierr!=0) return ierr;
}
else {
ierr = ComputeSYMGS_ref(A, r, x);
if (ierr!=0) return ierr;
}
return 0;
}
}
for (int i = 0; i < numberOfCalls; ++i) {
ZeroVector(x);
ierr = CG_ref(A, data, b, x, refMaxIters, tolerance, niters, normr, normr0, &ref_times[0], true);
int CG_ref
double t_begin = mytimer();
normr = 0.0;
double rtz = 0.0, oldrtz = 0.0, alpha = 0.0, beta = 0.0, pAp = 0.0;
double t0 = 0.0, t1 = 0.0, t2 = 0.0, t3 = 0.0, t4 = 0.0, t5 = 0.0;
local_int_t nrow = A.localNumberOfRows;
Vector & r = data.r;
Vector & z = data.z;
Vector & p = data.p;
Vector & Ap = data.Ap;
int print_freq = 1;
if (print_freq>50) print_freq=50;
if (print_freq<1)  print_freq=1;
CopyVector(x, p);
TICK(); ComputeSPMV_ref(A, p, Ap);  TOCK(t3);
TICK(); ComputeWAXPBY_ref(nrow, 1.0, b, -1.0, Ap, r); TOCK(t2);
TICK(); ComputeDotProduct_ref(nrow, r, r, normr, t4);  TOCK(t1);
normr = sqrt(normr);
normr0 = normr;
for (int k=1; k<=max_iter && normr/normr0 > tolerance; k++ ) {
TICK();
if (doPreconditioning) ComputeMG_ref(A, r, z);
else ComputeWAXPBY_ref(nrow, 1.0, r, 0.0, r, z);
TOCK(t5);
if (k == 1) {
CopyVector(z, p); TOCK(t2);
TICK(); ComputeDotProduct_ref(nrow, r, z, rtz, t4); TOCK(t1);
} else {
oldrtz = rtz;
TICK(); ComputeDotProduct_ref(nrow, r, z, rtz, t4); TOCK(t1);
beta = rtz/oldrtz;
TICK(); ComputeWAXPBY_ref(nrow, 1.0, z, beta, p, p);  TOCK(t2);
}
TICK(); ComputeSPMV_ref(A, p, Ap); TOCK(t3);
TICK(); ComputeDotProduct_ref(nrow, p, Ap, pAp, t4); TOCK(t1);
alpha = rtz/pAp;
TICK(); ComputeWAXPBY_ref(nrow, 1.0, x, alpha, p, x);
ComputeWAXPBY_ref(nrow, 1.0, r, -alpha, Ap, r);  TOCK(t2);
TICK(); ComputeDotProduct_ref(nrow, r, r, normr, t4); TOCK(t1);
normr = sqrt(normr);
if (A.geom->rank==0 && (k%print_freq == 0 || k == max_iter)) HPCG_fout << "Iteration = "<< k << "   Scaled Residual = "<< normr/normr0 << std::endl;
niters = k;
}
times[1] += t1;
times[2] += t2;
times[3] += t3;
times[4] += t4;
times[5] += t5;
times[0] += mytimer() - t_begin;
return 0;
}
int CG_ref
double t_begin = mytimer();
normr = 0.0;
double rtz = 0.0, oldrtz = 0.0, alpha = 0.0, beta = 0.0, pAp = 0.0;
double t0 = 0.0, t1 = 0.0, t2 = 0.0, t3 = 0.0, t4 = 0.0, t5 = 0.0;
local_int_t nrow = A.localNumberOfRows;
Vector & r = data.r;
Vector & z = data.z;
Vector & p = data.p;
Vector & Ap = data.Ap;
int print_freq = 1;
if (print_freq>50) print_freq=50;
if (print_freq<1)  print_freq=1;
CopyVector(x, p);
TICK(); ComputeSPMV_ref(A, p, Ap);  TOCK(t3);
TICK(); ComputeWAXPBY_ref(nrow, 1.0, b, -1.0, Ap, r); TOCK(t2);
TICK(); ComputeDotProduct_ref(nrow, r, r, normr, t4);  TOCK(t1);
normr = sqrt(normr);
normr0 = normr;
for (int k=1; k<=max_iter && normr/normr0 > tolerance; k++ ) {
TICK();
if (doPreconditioning) ComputeMG_ref(A, r, z);
else ComputeWAXPBY_ref(nrow, 1.0, r, 0.0, r, z);
TOCK(t5);
if (k == 1) {
CopyVector(z, p); TOCK(t2);
TICK(); ComputeDotProduct_ref(nrow, r, z, rtz, t4); TOCK(t1);
} else {
oldrtz = rtz;
TICK(); ComputeDotProduct_ref(nrow, r, z, rtz, t4); TOCK(t1);
beta = rtz/oldrtz;
TICK(); ComputeWAXPBY_ref(nrow, 1.0, z, beta, p, p);  TOCK(t2);
}
TICK(); ComputeSPMV_ref(A, p, Ap); TOCK(t3);
TICK(); ComputeDotProduct_ref(nrow, p, Ap, pAp, t4); TOCK(t1);
alpha = rtz/pAp;
TICK(); ComputeWAXPBY_ref(nrow, 1.0, x, alpha, p, x);
ComputeWAXPBY_ref(nrow, 1.0, r, -alpha, Ap, r);  TOCK(t2);
TICK(); ComputeDotProduct_ref(nrow, r, r, normr, t4); TOCK(t1);
normr = sqrt(normr);
if (A.geom->rank==0 && (k%print_freq == 0 || k == max_iter)) HPCG_fout << "Iteration = "<< k << "   Scaled Residual = "<< normr/normr0 << std::endl;
niters = k;
}
times[1] += t1;
times[2] += t2;
times[3] += t3;
times[4] += t4;
times[5] += t5;
times[0] += mytimer() - t_begin;
return 0;
}
if (ierr) ++err_count;
totalNiters_ref += niters;
}
for (int i = 0; i < numberOfCalls; ++i) {
ZeroVector(x);
double last_cummulative_time = opt_times[0];
ierr = CG(A, data, b, x, optMaxIters, refTolerance, niters, normr, normr0, &opt_times[0], true);
int CG
double t_begin = mytimer();
normr = 0.0;
double rtz = 0.0, oldrtz = 0.0, alpha = 0.0, beta = 0.0, pAp = 0.0;
double t0 = 0.0, t1 = 0.0, t2 = 0.0, t3 = 0.0, t4 = 0.0, t5 = 0.0;
local_int_t nrow = A.localNumberOfRows;
Vector & r = data.r;
Vector & z = data.z;
Vector & p = data.p;
Vector & Ap = data.Ap;
int print_freq = 1;
if (print_freq>50) print_freq=50;
if (print_freq<1)  print_freq=1;
CopyVector(x, p);
TICK(); ComputeSPMV(A, p, Ap); TOCK(t3);
TICK(); ComputeWAXPBY(nrow, 1.0, b, -1.0, Ap, r, A.isWaxpbyOptimized);  TOCK(t2);
TICK(); ComputeDotProduct(nrow, r, r, normr, t4, A.isDotProductOptimized); TOCK(t1);
normr = sqrt(normr);
normr0 = normr;
for (int k=1; k<=max_iter && normr/normr0 > tolerance; k++ ) {
TICK();
if (doPreconditioning) ComputeMG(A, r, z);
else CopyVector (r, z);
TOCK(t5);
if (k == 1) {
TICK(); ComputeWAXPBY(nrow, 1.0, z, 0.0, z, p, A.isWaxpbyOptimized); TOCK(t2);
TICK(); ComputeDotProduct (nrow, r, z, rtz, t4, A.isDotProductOptimized); TOCK(t1);
} else {
oldrtz = rtz;
TICK(); ComputeDotProduct (nrow, r, z, rtz, t4, A.isDotProductOptimized); TOCK(t1);
beta = rtz/oldrtz;
TICK(); ComputeWAXPBY (nrow, 1.0, z, beta, p, p, A.isWaxpbyOptimized);  TOCK(t2);
}
TICK(); ComputeSPMV(A, p, Ap); TOCK(t3);
TICK(); ComputeDotProduct(nrow, p, Ap, pAp, t4, A.isDotProductOptimized); TOCK(t1);
alpha = rtz/pAp;
TICK(); ComputeWAXPBY(nrow, 1.0, x, alpha, p, x, A.isWaxpbyOptimized);
ComputeWAXPBY(nrow, 1.0, r, -alpha, Ap, r, A.isWaxpbyOptimized);  TOCK(t2);
TICK(); ComputeDotProduct(nrow, r, r, normr, t4, A.isDotProductOptimized); TOCK(t1);
normr = sqrt(normr);
if (A.geom->rank==0 && (k%print_freq == 0 || k == max_iter)) HPCG_fout << "Iteration = "<< k << "   Scaled Residual = "<< normr/normr0 << std::endl;
niters = k;
}
times[1] += t1;
times[2] += t2;
times[3] += t3;
times[4] += t4;
times[5] += t5;
times[0] += mytimer() - t_begin;
return 0;
}
int CG
double t_begin = mytimer();
normr = 0.0;
double rtz = 0.0, oldrtz = 0.0, alpha = 0.0, beta = 0.0, pAp = 0.0;
double t0 = 0.0, t1 = 0.0, t2 = 0.0, t3 = 0.0, t4 = 0.0, t5 = 0.0;
local_int_t nrow = A.localNumberOfRows;
Vector & r = data.r;
Vector & z = data.z;
Vector & p = data.p;
Vector & Ap = data.Ap;
int print_freq = 1;
if (print_freq>50) print_freq=50;
if (print_freq<1)  print_freq=1;
CopyVector(x, p);
TICK(); ComputeSPMV(A, p, Ap); TOCK(t3);
TICK(); ComputeWAXPBY(nrow, 1.0, b, -1.0, Ap, r, A.isWaxpbyOptimized);  TOCK(t2);
TICK(); ComputeDotProduct(nrow, r, r, normr, t4, A.isDotProductOptimized); TOCK(t1);
normr = sqrt(normr);
normr0 = normr;
for (int k=1; k<=max_iter && normr/normr0 > tolerance; k++ ) {
TICK();
if (doPreconditioning) ComputeMG(A, r, z);
else CopyVector (r, z);
TOCK(t5);
if (k == 1) {
TICK(); ComputeWAXPBY(nrow, 1.0, z, 0.0, z, p, A.isWaxpbyOptimized); TOCK(t2);
TICK(); ComputeDotProduct (nrow, r, z, rtz, t4, A.isDotProductOptimized); TOCK(t1);
} else {
oldrtz = rtz;
TICK(); ComputeDotProduct (nrow, r, z, rtz, t4, A.isDotProductOptimized); TOCK(t1);
beta = rtz/oldrtz;
TICK(); ComputeWAXPBY (nrow, 1.0, z, beta, p, p, A.isWaxpbyOptimized);  TOCK(t2);
}
TICK(); ComputeSPMV(A, p, Ap); TOCK(t3);
TICK(); ComputeDotProduct(nrow, p, Ap, pAp, t4, A.isDotProductOptimized); TOCK(t1);
alpha = rtz/pAp;
TICK(); ComputeWAXPBY(nrow, 1.0, x, alpha, p, x, A.isWaxpbyOptimized);
ComputeWAXPBY(nrow, 1.0, r, -alpha, Ap, r, A.isWaxpbyOptimized);  TOCK(t2);
TICK(); ComputeDotProduct(nrow, r, r, normr, t4, A.isDotProductOptimized); TOCK(t1);
normr = sqrt(normr);
if (A.geom->rank==0 && (k%print_freq == 0 || k == max_iter)) HPCG_fout << "Iteration = "<< k << "   Scaled Residual = "<< normr/normr0 << std::endl;
niters = k;
}
times[1] += t1;
times[2] += t2;
times[3] += t3;
times[4] += t4;
times[5] += t5;
times[0] += mytimer() - t_begin;
return 0;
}
if (ierr) ++err_count;
if (normr / normr0 > refTolerance) ++tolerance_failures;
if (niters > optNiters) optNiters = niters;
double current_time = opt_times[0] - last_cummulative_time;
if (current_time > opt_worst_time) opt_worst_time = current_time;
}
for (int i = 0; i < numberOfCgSets; ++i) {
ZeroVector(x);
ierr = CG(A, data, b, x, optMaxIters, optTolerance, niters, normr, normr0, &times[0], true);
int CG
double t_begin = mytimer();
normr = 0.0;
double rtz = 0.0, oldrtz = 0.0, alpha = 0.0, beta = 0.0, pAp = 0.0;
double t0 = 0.0, t1 = 0.0, t2 = 0.0, t3 = 0.0, t4 = 0.0, t5 = 0.0;
local_int_t nrow = A.localNumberOfRows;
Vector & r = data.r;
Vector & z = data.z;
Vector & p = data.p;
Vector & Ap = data.Ap;
int print_freq = 1;
if (print_freq>50) print_freq=50;
if (print_freq<1)  print_freq=1;
CopyVector(x, p);
TICK(); ComputeSPMV(A, p, Ap); TOCK(t3);
TICK(); ComputeWAXPBY(nrow, 1.0, b, -1.0, Ap, r, A.isWaxpbyOptimized);  TOCK(t2);
TICK(); ComputeDotProduct(nrow, r, r, normr, t4, A.isDotProductOptimized); TOCK(t1);
normr = sqrt(normr);
normr0 = normr;
for (int k=1; k<=max_iter && normr/normr0 > tolerance; k++ ) {
TICK();
if (doPreconditioning) ComputeMG(A, r, z);
else CopyVector (r, z);
TOCK(t5);
if (k == 1) {
TICK(); ComputeWAXPBY(nrow, 1.0, z, 0.0, z, p, A.isWaxpbyOptimized); TOCK(t2);
TICK(); ComputeDotProduct (nrow, r, z, rtz, t4, A.isDotProductOptimized); TOCK(t1);
} else {
oldrtz = rtz;
TICK(); ComputeDotProduct (nrow, r, z, rtz, t4, A.isDotProductOptimized); TOCK(t1);
beta = rtz/oldrtz;
TICK(); ComputeWAXPBY (nrow, 1.0, z, beta, p, p, A.isWaxpbyOptimized);  TOCK(t2);
}
TICK(); ComputeSPMV(A, p, Ap); TOCK(t3);
TICK(); ComputeDotProduct(nrow, p, Ap, pAp, t4, A.isDotProductOptimized); TOCK(t1);
alpha = rtz/pAp;
TICK(); ComputeWAXPBY(nrow, 1.0, x, alpha, p, x, A.isWaxpbyOptimized);
ComputeWAXPBY(nrow, 1.0, r, -alpha, Ap, r, A.isWaxpbyOptimized);  TOCK(t2);
TICK(); ComputeDotProduct(nrow, r, r, normr, t4, A.isDotProductOptimized); TOCK(t1);
normr = sqrt(normr);
if (A.geom->rank==0 && (k%print_freq == 0 || k == max_iter)) HPCG_fout << "Iteration = "<< k << "   Scaled Residual = "<< normr/normr0 << std::endl;
niters = k;
}
times[1] += t1;
times[2] += t2;
times[3] += t3;
times[4] += t4;
times[5] += t5;
times[0] += mytimer() - t_begin;
return 0;
}
int CG
double t_begin = mytimer();
normr = 0.0;
double rtz = 0.0, oldrtz = 0.0, alpha = 0.0, beta = 0.0, pAp = 0.0;
double t0 = 0.0, t1 = 0.0, t2 = 0.0, t3 = 0.0, t4 = 0.0, t5 = 0.0;
local_int_t nrow = A.localNumberOfRows;
Vector & r = data.r;
Vector & z = data.z;
Vector & p = data.p;
Vector & Ap = data.Ap;
int print_freq = 1;
if (print_freq>50) print_freq=50;
if (print_freq<1)  print_freq=1;
CopyVector(x, p);
TICK(); ComputeSPMV(A, p, Ap); TOCK(t3);
TICK(); ComputeWAXPBY(nrow, 1.0, b, -1.0, Ap, r, A.isWaxpbyOptimized);  TOCK(t2);
TICK(); ComputeDotProduct(nrow, r, r, normr, t4, A.isDotProductOptimized); TOCK(t1);
normr = sqrt(normr);
normr0 = normr;
for (int k=1; k<=max_iter && normr/normr0 > tolerance; k++ ) {
TICK();
if (doPreconditioning) ComputeMG(A, r, z);
else CopyVector (r, z);
TOCK(t5);
if (k == 1) {
TICK(); ComputeWAXPBY(nrow, 1.0, z, 0.0, z, p, A.isWaxpbyOptimized); TOCK(t2);
TICK(); ComputeDotProduct (nrow, r, z, rtz, t4, A.isDotProductOptimized); TOCK(t1);
} else {
oldrtz = rtz;
TICK(); ComputeDotProduct (nrow, r, z, rtz, t4, A.isDotProductOptimized); TOCK(t1);
beta = rtz/oldrtz;
TICK(); ComputeWAXPBY (nrow, 1.0, z, beta, p, p, A.isWaxpbyOptimized);  TOCK(t2);
}
TICK(); ComputeSPMV(A, p, Ap); TOCK(t3);
TICK(); ComputeDotProduct(nrow, p, Ap, pAp, t4, A.isDotProductOptimized); TOCK(t1);
alpha = rtz/pAp;
TICK(); ComputeWAXPBY(nrow, 1.0, x, alpha, p, x, A.isWaxpbyOptimized);
ComputeWAXPBY(nrow, 1.0, r, -alpha, Ap, r, A.isWaxpbyOptimized);  TOCK(t2);
TICK(); ComputeDotProduct(nrow, r, r, normr, t4, A.isDotProductOptimized); TOCK(t1);
normr = sqrt(normr);
if (A.geom->rank==0 && (k%print_freq == 0 || k == max_iter)) HPCG_fout << "Iteration = "<< k << "   Scaled Residual = "<< normr/normr0 << std::endl;
niters = k;
}
times[1] += t1;
times[2] += t2;
times[3] += t3;
times[4] += t4;
times[5] += t5;
times[0] += mytimer() - t_begin;
return 0;
}
testnorms_data.values[i] = normr / normr0;
}

