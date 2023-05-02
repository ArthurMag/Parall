#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


void randomizeMatrix(int **matrix, int row, int collumn) {
	for (int i = 0; i < row; ++i) {
		for (int j = 0; j < collumn; ++j) {
			matrix[i][j] = /*rand()%100*/1; // = 1 for sum check
		}
	}
}

int** mallocMatrix(int **matrix, int row, int collumn) {
	matrix = malloc(sizeof(int*) * row);
	for (int i = 0; i < row; ++i) {
		matrix[i] = malloc(sizeof(int) * collumn);
	}
	return matrix;
}

int** freeMatrix(int **matrix, int row) {
	for (int i = 0; i < row; ++i) {
		free(matrix[i]);
	}
	free(matrix);
	return matrix;
}

struct param {
	int* start;
	int* end;
	int*** matrixA;
	int*** matrixB;
	int* M;
	int* L;
};

void do_count(int M, int L, int start, int end, int** A, int** B, int** locC) {
	int LocalSum = 0;
	for (int i = 0; i < M; ++i) {
		for (int j = 0; j < L; ++j) {
			for (int k = start; k < end; ++k) {
				LocalSum = LocalSum + A[i][k] * B[k][j];
			}
			locC[i][j] = locC[i][j] + LocalSum;
			LocalSum = 0;
		}
	}		
}

void* do_thread(void* arg) {
	struct param *my_param = (struct param*)arg;
	int **LocalC;
	LocalC = mallocMatrix(LocalC, *(*my_param).M, *(*my_param).L);
	do_count(*(*my_param).M, *(*my_param).L, *(*my_param).start, *(*my_param).end, *(*my_param).matrixA, *(*my_param).matrixB, LocalC);
	free(arg);
	return (void***) LocalC;
}

int get_something(int diff, int thread_num, int bin) {
	if (thread_num < diff) {
		return(thread_num + bin);
	}
	else {
		return(diff);
	}
}

int get_start(int N, int thread_num, int p) {
	int diff = N % p;
	int start = thread_num * (N / p);
	start = start + get_something(diff, thread_num, 0);
	return start;
}

int get_end(int N, int thread_num, int p) {
	int diff = N % p;
	int end = (thread_num + 1) * (N / p);
	end = end + get_something(diff, thread_num, 1);
	return end;
}

void main() {

	int **mMN, **mNL, **C;
	int M = 1000, N = 1000, L = 1000, p = 10;

	srand(time(NULL));
	struct timespec begin;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &begin);
	mMN = mallocMatrix(mMN, M, N);
	mNL = mallocMatrix(mNL, N, L);
	C = mallocMatrix(C, M, L);

	randomizeMatrix(mMN, M, N);
	randomizeMatrix(mNL, N, L);

	for (int i = 0; i < M; ++i) {
		for (int j = 0; j < L; ++j) {
	        C[i][j] = 0;
		}
	}

	pthread_t th[p];
	struct param *arg[p];
	int startA[p], endA[p];
	for (int i = 0; i < p; i++) {
		arg[i] = malloc(sizeof(struct param));
		startA[i] = 0;
		startA[i] = get_start(N, i, p);
		(*arg[i]).start = &startA[i];
		endA[i] = 0;
		endA[i] = get_end(N, i, p);
		(*arg[i]).end = &endA[i];
		(*arg[i]).matrixA = &mMN;
		(*arg[i]).matrixB = &mNL;
		(*arg[i]).M = &M;
		(*arg[i]).L = &L;
		pthread_create(&th[i], NULL, &do_thread, (void*) arg[i]);
	}

	for (int k = 0; k < p; k++) {
		int** LclC;
		pthread_join(th[k], (void **) &LclC);
		for (int i = 0; i < M; i++) {
			for (int j = 0; j < L; j++) {
				C[i][j] = C[i][j] + LclC[i][j];
			}
		}
		freeMatrix(LclC, M);
	}

	int Summ = 0;
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < L; j++) {
			Summ = Summ + C[i][j];
		}
	}
	printf("Summ: %d\n", Summ); //sum check if needed

	mMN = freeMatrix(mMN, M);
	mNL = freeMatrix(mNL, N);
	C = freeMatrix(C, M);

	struct timespec end;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	double time_spent = (end.tv_sec - begin.tv_sec) + (end.tv_nsec - begin.tv_nsec)/1000000000.0;
	printf("Time spent (CPU): %lf\n", time_spent);
}
