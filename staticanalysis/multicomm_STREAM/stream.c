for(std::size_t i = 0; i < nDims; i++ ) {
mpiErrorCheck(MPI_Cart_shift( comm_cart, i,  ShiftStep, &srcRank[0][i], &dstRank[0][i] ));
if (nDirs ==2) mpiErrorCheck(MPI_Cart_shift( comm_cart, i, negShiftStep, &srcRank[1][i], &dstRank[1][i] ));
}
for (std::size_t i = gridrank * tilesize; i < gridrank * tilesize + tilesize; i++){
x = (i+0.5)*stepLength;
sum = sum + 4.0 / ( 1.0 + x * x);
}
for ( std::size_t j = 0; j < nDirs; j++ ) {
mpiErrorCheck(MPI_Sendrecv( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], j, &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], j, comm_cart, MPI_STATUS_IGNORE ));
}
for ( std::size_t j = 0; j < nDirs; j++ ) {
mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], 0, comm_cart, &req[i*4+(0+j*2)] ));
mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], 0, comm_cart, &req[i*4+(1+j*2)] ));
}
for ( std::size_t j = 0; j < nDirs; j++ ) {
mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], 0, comm_cart, &req[0] ));
mpiErrorCheck(MPI_Recv ( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], 0, comm_cart, MPI_STATUS_IGNORE ));
mpiErrorCheck(MPI_Barrier( comm_cart ));
}
for ( std::size_t timeStep = 0 ; timeStep <= nTimeStep ; timeStep++ ) {
LIKWID_MARKER_START("Case:Computation");
for (std::size_t i=0 ; i < gridNTasks ; ++i){
if (gridrank == i) maint1 = high_resolution_clock::now();
}
for (std::size_t timing_repetitions = 0; timing_repetitions < nCppTimeRep; ++timing_repetitions) {
if( gridrank == 0 && timeStep == 0) std::cout << "\nComputation: compute-bound pi solver" <<std::endl;
sum = 0.0;
std::size_t tilesize = nSteps / gridNTasks;
for (std::size_t i = gridrank * tilesize; i < gridrank * tilesize + tilesize; i++){
x = (i+0.5)*stepLength;
sum = sum + 4.0 / ( 1.0 + x * x);
}
if (sum < 0) std::cout << "sum" << sum <<std::endl;
if( gridrank == 0 && timeStep == 0) std::cout << "taking idle period values from configuration setting" << std::endl;
sum = 0.0;
if ((timeStep==IdlePeriodTimeStep) && (gridrank % gridNTasks == IdlePeriodRank)) {
for ( std::size_t i = 0; i < extraIt; i++ ) {
x = 0.5*stepLength;
sum = sum + 4.0 / ( 1.0 + x * x);
}
if (sum < 0) std::cout << "sum" << sum <<std::endl;
}
if (std::fstream{filename}) {
if( gridrank == 0 && timeStep == 0) std::cout << "taking fine-grained delay values from binary file" << std::endl;
sum = 0.0;
const realT stepLength1 = 1./44245260;
for (std::size_t i=0 ; i < gridNTasks ; ++i){
if ((timeStep>=extraItTimeStep) && (timeStep<extraItTimeStep+bufsize) && (gridrank == i)) {
for ( std::size_t j = 0; j < workIterations[timeStep-extraItTimeStep]; j++ ) {
x = 0.5*stepLength1;
sum = sum + 4.0 / ( 1.0 + x * x);
}
if (sum < 0) std::cout << "sum" << sum <<std::endl;
}
}
}
if( gridrank == 0 && timeStep == 0) std::cout << "\nComputation: memory-bound triad benchmark" <<std::endl;
for (std::size_t i=0; i<arrayElements; i++) A[i] = B[i] + cos(C[i]/D[i]);
if (A[arrayElements/2] < 0) std::cout << "A" << A <<std::endl;
if( gridrank == 0 && timeStep == 0) std::cout << "taking idle period values from configuration setting" << std::endl;
if ((timeStep==IdlePeriodTimeStep) && (gridrank % gridNTasks == IdlePeriodRank)) {
for (std::size_t j=0; j<extraTriadIt; j++)	{
for (std::size_t i=0; i<arrayElements; i++) A[i] = B[i]+scalar*C[i];
if (A[arrayElements/2] < 0) std::cout << "A" << A <<std::endl;
}
}
if (std::fstream{filename}) {
if( gridrank == 0 && timeStep == 0) std::cout << "taking fine-grained delay values from binary file" << std::endl;
realT sum, x;
const realT stepLength = 1./44245260;
sum = 0.0;
std::size_t extraItTimeStep =extraItTimeStep1;// +j;
for (std::size_t i=0 ; i < gridNTasks ; ++i){
if ((timeStep>=extraItTimeStep) && (timeStep<extraItTimeStep+bufsize) && (gridrank == i)) {
for ( std::size_t j = 0; j < workIterations[timeStep-extraItTimeStep]; j++ ) {
x = 0.5*stepLength;
sum = sum + 4.0 / ( 1.0 + x * x);
}
if (sum < 0) std::cout << "sum" << sum <<std::endl;
}
}
}
LIKWID_MARKER_STOP("Case:Computation");
}
for (std::size_t i=0 ; i < gridNTasks ; ++i){
if (gridrank == i) {
maint2 = high_resolution_clock::now();
timeT  CompDuration = duration_cast<nanoseconds>(maint2 - maint1).count();
CompDuration /= static_cast<double>(nCppTimeRep);
aggregate_time1 = aggregate_time3 + CompDuration;
}
}
TR.add_loclop(gridrank, aggregate_time3/1000000000, aggregate_time1/1000000000, 0);
if (timeStep!=collectiveOPTimeStep){
if (timeStep!=collectiveOPTimeStep){
LIKWID_MARKER_START("Case:PairwiseSendrecv");
for (std::size_t i=0 ; i < gridNTasks ; ++i){
if (gridrank == i) maint3 = high_resolution_clock::now();
}
for (std::size_t timing_repetitions = 0; timing_repetitions < nCppTimeRep; ++timing_repetitions) {
if( gridrank == 0 && timeStep == 0) std::cout << "\nCommunication Pattern: Pairwise blocking send and receive operations" <<std::endl;
for ( std::size_t i = 0; i < nDims; i++ ) {
for ( std::size_t j = 0; j < nDirs; j++ ){
mpiErrorCheck(MPI_Sendrecv( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], j, &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], j, comm_cart, MPI_STATUS_IGNORE ));
}
}
if( gridrank == 0 && timeStep == 0) std::cout << "\nCommunication Pattern: Pairwise non-blocking send and receive operations" <<std::endl;
for ( std::size_t i = 0; i < nDims; i++ ) {
for ( std::size_t j = 0; j < nDirs; j++ ) {
mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], 0, comm_cart, &req[i*4+(0+j*2)] ));
mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], 0, comm_cart, &req[i*4+(1+j*2)] ));
}
}
mpiErrorCheck(MPI_Waitall( 2*nDirs* nDims, &req[0], &stt[0] ));
if( gridrank == 0 && timeStep == 0) std::cout << "\nCommunication Pattern: Pairwise non-blocking send and blocking receive operations" <<std::endl;
for ( std::size_t i = 0; i < nDims; i++ ) {
for ( std::size_t j = 0; j < nDirs; j++ ) {
mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], 0, comm_cart, &req[0] ));
mpiErrorCheck(MPI_Recv ( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], 0, comm_cart, MPI_STATUS_IGNORE ));
mpiErrorCheck(MPI_Barrier( comm_cart ));
}
}
LIKWID_MARKER_STOP("Case:PairwiseSendrecv");
}
for (std::size_t i=0 ; i < gridNTasks ; ++i){
if (gridrank == i){
maint4 = high_resolution_clock::now();
timeT  SendrecvDuration = duration_cast<nanoseconds>(maint4 - maint3).count();
SendrecvDuration /= static_cast<double>(nCppTimeRep);
aggregate_time2 += SendrecvDuration;
timeT  CompSendrecvDuration = duration_cast<nanoseconds>(maint4 - maint1).count();
CompSendrecvDuration /= static_cast<double>(nCppTimeRep);
aggregate_time3 += CompSendrecvDuration;
}
}
}
realT result=0.0;
if (timeStep==collectiveOPTimeStep){
if( gridrank == 0) std::cout << "\nCommunication Pattern for time step iteration " << timeStep << ": Collective blocking send and receive operation\n" << std::endl;
LIKWID_MARKER_START("Case:BCollectiveSendRecv");
for (std::size_t timing_repetitions = 0; timing_repetitions < nCppTimeRep; ++timing_repetitions) {
if(gridrank != 0){
mpiErrorCheck(MPI_Send(&sum, 1, MPI_REAL_TYPE, 0, 1, MPI_COMM_WORLD));
}else{
MPI_Status status;
result=sum;
for(int i=1;i<gridNTasks;++i){
mpiErrorCheck(MPI_Recv(&sum, 1, MPI_REAL_TYPE, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD,&status));
result+=sum;
}
}
LIKWID_MARKER_STOP("Case:BCollectiveSendRecv");
}
if( gridrank == 0 ) std::cout << "PI = " << result * stepLength << " for time step iteration " << timeStep <<" with error compare to the Pi in 'math.h': " << fabs((result * stepLength) - PiExact) << std::endl;
}
}
realT result=0.0;
if (timeStep==collectiveOPTimeStep){
if( gridrank == 0) std::cout << "\nCommunication Pattern for time step iteration " << timeStep << ": Collective blocking reduce operation\n" << std::endl;
LIKWID_MARKER_START("Case:BCollectiveReduce");
high_resolution_clock::time_point maint5 = high_resolution_clock::now();
for (std::size_t timing_repetitions = 0; timing_repetitions < nCppTimeRep; ++timing_repetitions) {
for ( std::size_t i = 0; i < nDims; i++ ) {
for ( std::size_t j = 0; j < nDirs; j++ ) {
mpiErrorCheck(MPI_Allgather(&sendBuffer[i][j][0], 1, MPI_REAL_TYPE, &recvBuffer[i][j][0], 1, MPI_REAL_TYPE, comm_cart));
}
}
LIKWID_MARKER_STOP("Case:BCollectiveReduce");
}
high_resolution_clock::time_point maint6 = high_resolution_clock::now();
duration<timeT> BReduceDuration = duration_cast<duration<timeT>>(maint6 - maint5);
BReduceDuration/= static_cast<double>(nCppTimeRep);
aggregate_time2 += BReduceDuration.count();
duration<timeT> CompBReduceDuration = duration_cast<duration<timeT>>(maint6 - maint1);
CompBReduceDuration /= static_cast<double>(nCppTimeRep);
aggregate_time3 += CompBReduceDuration.count();
if( gridrank == 0 ) std::cout << "PI = " << result * stepLength << " for time step iteration " << timeStep <<" with error compare to the Pi in 'math.h': " << fabs((result * stepLength) - PiExact) << std::endl;
}
}
if( gridrank == 0 ) {
std::cout << "\nComputation phase for " << aggregate_time1 * 1000 << " ms" << std::endl;
std::cout << "Pairwise communication phase time without computation for " << length << " doubles: " << aggregate_time2 * 1000 << " ms" << std::endl;
std::cout << "Total Pairwise communication phase time with computation for " << length << " doubles: " << aggregate_time3 * 1000 << " ms" <<    std::endl;
std::cout << "Collective communication phase time without computation for " << length << " doubles: " << aggregate_time4 * 1000 << " ms" <<    std::endl;
std::cout << "Total Collective communication phase time with computation for " << length << " doubles: " << aggregate_time5 * 1000 << " ms" <<    std::endl;
std::cout << "\nFor detailed time-rank traces, see 'timeRankViz.cvs' file \n " <<    std::endl;
std::cout << " totaltime "<< aggregate_time1 * 1000 << " " << aggregate_time2 * 1000  << " " << aggregate_time3 * 1000 << std::endl;
}
try {
for(std::size_t x = 0; x < nDims; ++x){
for(std::size_t y = 0; y < nDirs; ++y){
delete[] sendBuffer[x][y];
}
delete[] sendBuffer[x];
}
delete[] sendBuffer;
for(std::size_t x = 0; x < nDims; ++x){
for(std::size_t y = 0; y < nDirs; ++y){
delete[] recvBuffer[x][y];
}
delete[] recvBuffer[x];
}
delete[] recvBuffer;
free(A);
free(B);
free(C);
free(buf);
mpiErrorCheck(MPI_Comm_free( &comm_cart ));
}
catch 
std::cerr << "error on deallocating resouces: " << e.what() << std::endl;
return (EXIT_FAILURE);
}
}
for (std::size_t timing_repetitions = 0; timing_repetitions < nCppTimeRep; ++timing_repetitions) {
if( gridrank == 0 && timeStep == 0) std::cout << "\nComputation: compute-bound pi solver" <<std::endl;
sum = 0.0;
std::size_t tilesize = nSteps / gridNTasks;
for (std::size_t i = gridrank * tilesize; i < gridrank * tilesize + tilesize; i++){
x = (i+0.5)*stepLength;
sum = sum + 4.0 / ( 1.0 + x * x);
}
if (sum < 0) std::cout << "sum" << sum <<std::endl;
if( gridrank == 0 && timeStep == 0) std::cout << "taking idle period values from configuration setting" << std::endl;
sum = 0.0;
if ((timeStep==IdlePeriodTimeStep) && (gridrank % gridNTasks == IdlePeriodRank)) {
for ( std::size_t i = 0; i < extraIt; i++ ) {
x = 0.5*stepLength;
sum = sum + 4.0 / ( 1.0 + x * x);
}
if (sum < 0) std::cout << "sum" << sum <<std::endl;
}
if (std::fstream{filename}) {
if( gridrank == 0 && timeStep == 0) std::cout << "taking fine-grained delay values from binary file" << std::endl;
sum = 0.0;
const realT stepLength1 = 1./44245260;
for (std::size_t i=0 ; i < gridNTasks ; ++i){
if ((timeStep>=extraItTimeStep) && (timeStep<extraItTimeStep+bufsize) && (gridrank == i)) {
for ( std::size_t j = 0; j < workIterations[timeStep-extraItTimeStep]; j++ ) {
x = 0.5*stepLength1;
sum = sum + 4.0 / ( 1.0 + x * x);
}
if (sum < 0) std::cout << "sum" << sum <<std::endl;
}
}
}
if( gridrank == 0 && timeStep == 0) std::cout << "\nComputation: memory-bound triad benchmark" <<std::endl;
for (std::size_t i=0; i<arrayElements; i++) A[i] = B[i] + cos(C[i]/D[i]);
if (A[arrayElements/2] < 0) std::cout << "A" << A <<std::endl;
if( gridrank == 0 && timeStep == 0) std::cout << "taking idle period values from configuration setting" << std::endl;
if ((timeStep==IdlePeriodTimeStep) && (gridrank % gridNTasks == IdlePeriodRank)) {
for (std::size_t j=0; j<extraTriadIt; j++)	{
for (std::size_t i=0; i<arrayElements; i++) A[i] = B[i]+scalar*C[i];
if (A[arrayElements/2] < 0) std::cout << "A" << A <<std::endl;
}
}
if (std::fstream{filename}) {
if( gridrank == 0 && timeStep == 0) std::cout << "taking fine-grained delay values from binary file" << std::endl;
realT sum, x;
const realT stepLength = 1./44245260;
sum = 0.0;
std::size_t extraItTimeStep =extraItTimeStep1;// +j;
for (std::size_t i=0 ; i < gridNTasks ; ++i){
if ((timeStep>=extraItTimeStep) && (timeStep<extraItTimeStep+bufsize) && (gridrank == i)) {
for ( std::size_t j = 0; j < workIterations[timeStep-extraItTimeStep]; j++ ) {
x = 0.5*stepLength;
sum = sum + 4.0 / ( 1.0 + x * x);
}
if (sum < 0) std::cout << "sum" << sum <<std::endl;
}
}
}
LIKWID_MARKER_STOP("Case:Computation");
}
for (std::size_t i = gridrank * tilesize; i < gridrank * tilesize + tilesize; i++){
x = (i+0.5)*stepLength;
sum = sum + 4.0 / ( 1.0 + x * x);
}
for ( std::size_t i = 0; i < extraIt; i++ ) {
x = 0.5*stepLength;
sum = sum + 4.0 / ( 1.0 + x * x);
}
for ( std::size_t j = 0; j < workIterations[timeStep-extraItTimeStep]; j++ ) {
x = 0.5*stepLength1;
sum = sum + 4.0 / ( 1.0 + x * x);
}
for ( std::size_t j = 0; j < workIterations[timeStep-extraItTimeStep]; j++ ) {
x = 0.5*stepLength;
sum = sum + 4.0 / ( 1.0 + x * x);
}
for (std::size_t timing_repetitions = 0; timing_repetitions < nCppTimeRep; ++timing_repetitions) {
if( gridrank == 0 && timeStep == 0) std::cout << "\nCommunication Pattern: Pairwise blocking send and receive operations" <<std::endl;
for ( std::size_t i = 0; i < nDims; i++ ) {
for ( std::size_t j = 0; j < nDirs; j++ ){
mpiErrorCheck(MPI_Sendrecv( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], j, &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], j, comm_cart, MPI_STATUS_IGNORE ));
}
}
if( gridrank == 0 && timeStep == 0) std::cout << "\nCommunication Pattern: Pairwise non-blocking send and receive operations" <<std::endl;
for ( std::size_t i = 0; i < nDims; i++ ) {
for ( std::size_t j = 0; j < nDirs; j++ ) {
mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], 0, comm_cart, &req[i*4+(0+j*2)] ));
mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], 0, comm_cart, &req[i*4+(1+j*2)] ));
}
}
mpiErrorCheck(MPI_Waitall( 2*nDirs* nDims, &req[0], &stt[0] ));
if( gridrank == 0 && timeStep == 0) std::cout << "\nCommunication Pattern: Pairwise non-blocking send and blocking receive operations" <<std::endl;
for ( std::size_t i = 0; i < nDims; i++ ) {
for ( std::size_t j = 0; j < nDirs; j++ ) {
mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], 0, comm_cart, &req[0] ));
mpiErrorCheck(MPI_Recv ( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], 0, comm_cart, MPI_STATUS_IGNORE ));
mpiErrorCheck(MPI_Barrier( comm_cart ));
}
}
LIKWID_MARKER_STOP("Case:PairwiseSendrecv");
}
for ( std::size_t j = 0; j < nDirs; j++ ){
mpiErrorCheck(MPI_Sendrecv( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], j, &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], j, comm_cart, MPI_STATUS_IGNORE ));
}
for ( std::size_t j = 0; j < nDirs; j++ ) {
mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], 0, comm_cart, &req[i*4+(0+j*2)] ));
mpiErrorCheck(MPI_Irecv( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], 0, comm_cart, &req[i*4+(1+j*2)] ));
}
for ( std::size_t j = 0; j < nDirs; j++ ) {
mpiErrorCheck(MPI_Isend( &sendBuffer[i][j][0], length, MPI_REAL_TYPE, dstRank[j][i], 0, comm_cart, &req[0] ));
mpiErrorCheck(MPI_Recv ( &recvBuffer[i][j][0], length, MPI_REAL_TYPE, srcRank[j][i], 0, comm_cart, MPI_STATUS_IGNORE ));
mpiErrorCheck(MPI_Barrier( comm_cart ));
}
for(int i=1;i<gridNTasks;++i){
mpiErrorCheck(MPI_Recv(&sum, 1, MPI_REAL_TYPE, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD,&status));
result+=sum;
}
for (std::size_t timing_repetitions = 0; timing_repetitions < nCppTimeRep; ++timing_repetitions) {
for ( std::size_t i = 0; i < nDims; i++ ) {
for ( std::size_t j = 0; j < nDirs; j++ ) {
mpiErrorCheck(MPI_Allgather(&sendBuffer[i][j][0], 1, MPI_REAL_TYPE, &recvBuffer[i][j][0], 1, MPI_REAL_TYPE, comm_cart));
}
}
LIKWID_MARKER_STOP("Case:BCollectiveReduce");
}
for ( std::size_t j = 0; j < nDirs; j++ ) {
mpiErrorCheck(MPI_Allgather(&sendBuffer[i][j][0], 1, MPI_REAL_TYPE, &recvBuffer[i][j][0], 1, MPI_REAL_TYPE, comm_cart));
}

