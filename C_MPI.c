#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>

int M = 1000, N = 1000, L = 1000;

void randomizeMatrix(int **matrix, int row, int collumn) {
	for (int i = 0; i < row; ++i) {
		for (int j = 0; j < collumn; ++j) {
			matrix[i][j] = rand()%100; // = 1 for sum check
		}
	}
}

int** freeMatrix(int **matrix) {
	free(matrix[0]);
	free(matrix);
	return matrix;
}

int **alloc_2d_init(int rows, int cols) {
	int *data = (int *)malloc(rows*cols*sizeof(int));
	int **array= (int **)malloc(rows*sizeof(int*));
	for (int i=0; i<rows; i++)
		array[i] = &(data[cols*i]);
	
	return array;
}

void get_result(int **A, int **B, int **Res, int num_task) {
	for (int i = 0; i < M; ++i) {
		for (int j = 0; j < L; ++j) {
			Res[i][j] = 0;
			for (int k = 0; k < num_task; ++k) {
				Res[i][j] = Res[i][j] + A[k][i]*B[k][j];
			}
			//Res[0][0] = Res[0][0] + Res[i][j]; // Sum check
		}
	}
}


void main(int argc, char *argv[]) {

	MPI_Init(&argc, &argv);

	int num_tasks, task_id;
	MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &task_id);

	srand(time(NULL));
	struct timespec begin, end;
	if (task_id == 0) {
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &begin);
	}

	int **mMN, **mNL;
	int **Receive_dataA, **Receive_dataB;
	int **LocalC, **GlobalC;

	Receive_dataA = alloc_2d_init(N/num_tasks,M);
	Receive_dataB = alloc_2d_init(N/num_tasks,L);
	LocalC = alloc_2d_init(M,L); //local result for each task
	
	mMN = alloc_2d_init(N,M); //inverted matrix A, easy to send
	mNL = alloc_2d_init(N,L); //classic matrix B, easy to send
	GlobalC = alloc_2d_init(M,L); //final result for main task

	if (task_id == 0) {
		mMN = alloc_2d_init(N,M); //inverted matrix A, easy to send
		mNL = alloc_2d_init(N,L); //classic matrix B, easy to send
		GlobalC = alloc_2d_init(M,L); //final result for main task
		randomizeMatrix(mMN, N, M);
		randomizeMatrix(mNL, N, L);
	}

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Scatter(&(mMN[0][0]), M*(N/num_tasks), MPI_INT, &(Receive_dataA[0][0]), M*(N/num_tasks), MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Scatter(&(mNL[0][0]), L*(N/num_tasks), MPI_INT, &(Receive_dataB[0][0]), L*(N/num_tasks), MPI_INT, 0, MPI_COMM_WORLD);
	get_result(Receive_dataA, Receive_dataB, LocalC, N/num_tasks);

	MPI_Reduce(&(LocalC[0][0]), &(GlobalC[0][0]), M*L, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	freeMatrix(Receive_dataA);
	freeMatrix(Receive_dataB);
	freeMatrix(LocalC);

	/*if (task_id == 0) {	
		printf("GlobalC[0][0] from task %d = %d\n", task_id, GlobalC[0][0]); // Sum check = N+N*M*L if every number in MN NL arrays = 1
	}*/
	if (task_id == 0) {
		freeMatrix(mMN);
		freeMatrix(mNL);
		freeMatrix(GlobalC);
	}
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
	if (task_id == 0) {
		double time_spent = (end.tv_sec - begin.tv_sec) + (end.tv_nsec - begin.tv_nsec)/1000000000.0;
		printf("Time spent (CPU): %lf\n", time_spent);
	}
	MPI_Finalize();
}
