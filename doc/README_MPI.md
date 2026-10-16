Part of the [main README](../README.md#further-reading).

<a name="mpi-parallelized-implementation"></a>
## 🧐 MPI-parallelized DisCostiC implementation

DisCostiC is itself an MPI program. Launch it with `number_of_processes + 1` ranks: one controller rank (`Q_0`) and one simulator rank (`Q_i`) for each simulated application rank (`P_i`). `MPI_Comm_split` puts the controller on its own side of the communicator. The remaining ranks advance the discrete-event schedule and exchange serialized operation metadata with blocking or non-blocking MPI. The controller merges trace output and prints verbose logs.

Simulator process count follows the simulated rank count. Very large runs may need distributed event scheduling; that limit is listed under [Planned features](../README.md#library-limitations).

The underlying principle of parallel simulation is that each operation's entire data is transmitted via blocking or non-blocking MPI routines to the processes it communicates with.

| Terms                  | Description | 
| --------------------- | ------------- | 
| `simulated processes P_i`    | processes from the application point of view | 
| `simulator processes Q_i`    | processes from the parallel simulation framework point of view | 
| `master simulator process Q_0`  | master process from the parallel simulation framework point of view | 


**Initialization**:thought_balloon:
In the MPI implementation, only the master process of the default communicator makes any print calls for debug and verbose output reporting, and all other processes of the new communicator initialize root operations only once in each run. 

**MPI-parallelized**

```cpp
int ierr, process_Rank, size_Of_Cluster, sucesss, N_process_Rank, N_size_Of_Cluster;
MPI_Comm newcomm;
ierr = MPI_Init(NULL, NULL);
ierr = MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);

MPI_Comm_split(MPI_COMM_WORLD, (process_Rank) < 1 ? 1 : 2, 1, &newcomm);
ierr = MPI_Comm_rank(newcomm, &N_process_Rank);
ierr = MPI_Comm_size(newcomm, &N_size_Of_Cluster);
ierr = MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);
ierr = MPI_Comm_size(MPI_COMM_WORLD, &size_Of_Cluster);
std::queue<int> test;
MPI_Status status;
MPI_Request request;
    
DisCostiC_Timetype starttimeProcess;
DisCostiC::DisCostiC_OP operation, operation_og;		
int iter = 0;

iter = ((rankLocalIt->getNumOps()) / 3);

if (rank == N_process_Rank)
{
 operation_og = copy(op, operation_og);
 operation = op;
 starttimeProcess = operation.starttime;
}
```

**Serialized**

```cpp
op.numOpsInQueue = numOpsInQueue++;
queue.emplace(op);
```


**Start of simulation**:thought_balloon:
Do loop over the number of iterations and the number of steps per iteration. Processes connected to the new communicator, or "newcomm," are only used in the main simulation.

**MPI-parallelized**

```cpp
rank = N_process_Rank;
bool cycle = true;
int iter_num = 1;
do{ //!< over the number of iterations
  do{ //!< over the number of steps per iteration
```

**Serialized**

```cpp
do{
  queue.pop();
```

Within each ccNUMA domain, runtime corrections are made by monitoring the number of processes that are concurrently engaged in each computation and communication step. 

**SEND operation**:thought_balloon:
The currently active process sends the operation object or array to the process listed as "operation.target," which is specific to the communication pattern simulation program.
 
**MPI-parallelized**

```cpp
operation.type = DisCostiC::Operation_t::RECV;
operation.starttime = operation.time;

int temp = oSuccessor[operation.rank][operation.node];
                        
double arr[13];                   
*serialize(operation, temp, arr);
if (operation.mode == DisCostiC::Mode_t::NONBLOCKING){
  MPI_Isend(&arr, 13, MPI_DOUBLE, operation.target, operation.tag, newcomm, &request);
}else{
  MPI_Send(&arr, 13, MPI_DOUBLE, operation.target, operation.tag, newcomm);
}

finishedRankList.push_back(operation.rank);
operation.type = DisCostiC::Operation_t::RECV;
cycle = true;
```

**Serialized**

```cpp
std::swap(operation.rank,operation.target);
queue.emplace(operation);
```


**RECV operation**:thought_balloon:
The currently running process receives the operation object or array from the process listed as "operation.target".
 
**MPI-parallelized**

```cpp
double brr[13];
if (operation.mode == DisCostiC::Mode_t::NONBLOCKING){
  MPI_Irecv(&brr, 13, MPI_DOUBLE, MPI_ANY_SOURCE, operation.tag, newcomm, &request);
}else{
  MPI_Irecv(&brr, 13, MPI_DOUBLE, MPI_ANY_SOURCE, operation.tag, newcomm, &request);
}
*deserialize(operation, temp, brr);                   
oSuccessor[operation.rank][operation.node] = brr[12];
cycle=true;
operation.numOpsInQueue = numOpsInQueue++;
finishedRankList.push_back(operation.rank);

operation.type = DisCostiC::Operation_t::MSG;
```

**Serialized**

```cpp
finishedRankList.push_back(operation.rank);
DisCostiC::DisCostiC_queueOP matchedOP;
if(M.listmatch(operation, &UMQ[operation.rank], &matchedOP))

else { 
 DisCostiC::DisCostiC_queueOP nOp;
 nOp.bufSize = operation.bufSize;
 nOp.src = operation.target;
 nOp.tag = operation.tag;
 nOp.label = operation.label;
 recvQueue[operation.rank].push_back(nOp);
}
```


**MSG operation**:thought_balloon:
If MSG is not found, the cycle will continue (cycle = true) until it is found.
 
**MPI-parallelized**

```cpp
if ((earliestFinish = operation.time) <= ...){
 ...
 operation.numOpsInQueue = numOpsInQueue++;
 ...
 iter_num++;		
 cycle = false;		        		
}else{
 ...
 cycle = true;
}
```

**Serialized**

```cpp
if ((earliestFinish = operation.time) <= ...){
 ...
 DisCostiC::DisCostiC_queueOP matchedOP;
 if(M.listmatch(operation, &recvQueue[operation.rank], &matchedOP)) { 
 ...
 }else { 
 #ifdef USE_VERBOSE 
 std::cout << "[MSG ==> NOT FOUND] in receive queue - add to unexpected queue"<< std::endl;
 #endif
 DisCostiC::DisCostiC_queueOP nOp;
 nOp.bufSize = operation.bufSize;
 nOp.src = operation.target;
 nOp.tag = operation.tag;
 nOp.label = operation.label;
 nOp.starttime = operation.time; // when it was started
 UMQ[operation.rank].push_back(nOp);
 }
}else{
 ...
 queue.emplace(operation);
}
```


**Adding new sorted operations in queue in order**:thought_balloon:

The parallel path does not add a separate snippet in this step.

**Serialized**

```cpp
newOps=false;

newOps = true;
op.numOpsInQueue=numOpsInQueue++;

queue.emplace(op);
```


**End of while statement**:thought_balloon:
The end of the do loops over the number of iterations and the number of steps per iteration.
The necessary information gathered from all processes of the new communicator is communicated to the default communicator's master process.
 
**MPI-parallelized**

```cpp
while (cycle);
 cycle = true;
 operation_og.time = oSuccessor[operation.rank][operation.node];
 operation = copy(operation_og, operation);
 } while (iter_num <= iter);
 double arr[13];
 double brr[13];
 *serialize(operation, oSuccessor[rank][operation.node], arr);
 *finalize(oSuccessor[rank][operation.node], numRanks, numOpsInQueue, rank, operation.node, brr);
 MPI_Send(&brr, 13, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
 }
 else if (process_Rank == 0)
 {
 bool cycle = true;
 double brr[13];
 while (test.size() < numRanks)
 {
   MPI_Recv(&brr, 13, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
   if (status.MPI_TAG == 1)
   {
     test.emplace(status.MPI_SOURCE);
     oSuccessor[brr[3]][brr[4]] = brr[0];
     numOpsInQueue = brr[2];
   }
 }
}
ierr = MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);
MPI_Barrier(MPI_COMM_WORLD);
```

**Serialized**

```cpp
while(!queue.empty() || newOps)
```


