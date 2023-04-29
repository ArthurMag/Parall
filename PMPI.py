import numpy as np 
from mpi4py import MPI
import time

start_time = time.time()
comm = MPI.COMM_WORLD
size = comm.Get_size()
rank = comm.Get_rank()

M = 200
N = 200
L = 200
sendbufNM = None
sendbufNL = None

if rank == 0:
    sendbufNM = np.random.randint(0, 100, size=(N,M), dtype='i')
    sendbufNL = np.random.randint(0, 100, size=(N,L), dtype='i')

recvbufA = np.empty([N//size,M], dtype='i')
recvbufB = np.empty([N//size,L], dtype='i')

comm.Barrier()
comm.Scatter(sendbufNM, recvbufA, root = 0)
comm.Scatter(sendbufNL, recvbufB, root = 0)

LocalC = np.zeros((M,L))
GlobalC = np.zeros((M,L))

for i in range(M):
    for j in range(L):
        for k in range(N//size):
            LocalC[i][j] = LocalC[i][j] + recvbufA[k][i]*recvbufB[k][j]


comm.Reduce(LocalC, GlobalC, op=MPI.SUM, root=0)

if rank == 0:
    print("Elapsed time:", time.time() - start_time)
