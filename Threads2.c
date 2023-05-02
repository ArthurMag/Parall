#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void randomizeMatrix(int **matrix, int row, int collumn) {
	for (int i = 0; i < row; ++i) {
	        for (int j = 0; j < collumn; ++j) {
        	        matrix[i][j] = rand()%100; // = 1 for sum check
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
	/*matrix = NULL;*/
	return matrix;
}

struct param {
        int* start;
        int* end;
		int*** matrixA;
		int*** matrixB;
		int*** matrixC;
		int* M;
		int* N;
		int* L;
};

void do_count(int i_st, int i_ed, int j_st, int j_ed, int** A, int** B, int** C, int N) {
	for (int i = i_st; i < i_ed; ++i) {
		for (int j = j_st; j < j_ed; ++j) {
			C[i][j] = 0;
			for (int k = 0; k < N; ++k) {
				C[i][j] = C[i][j] + A[i][k]*B[k][j];
			}
		}
	}		
}

void* do_thread(void* arg) {
	struct param *my_param = (struct param*)arg;
	int LocalSum = 0;
	if (*(*my_param).M > *(*my_param).L) {
		do_count(*(*my_param).start, *(*my_param).end, 0, *(*my_param).L, *(*my_param).matrixA, *(*my_param).matrixB, *(*my_param).matrixC, *(*my_param).N);
	}
	else {
		do_count(0, *(*my_param).M, *(*my_param).start, *(*my_param).end, *(*my_param).matrixA, *(*my_param).matrixB, *(*my_param).matrixC, *(*my_param).N);
	}
	free(arg);
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
	
	int M = 1000, N = 1000, L = 1000, p = 10;
	int **mMN, **mNL, **C;

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
		if (M > L) {
			startA[i] = get_start(M, i, p);
			endA[i] = get_end(M, i, p);
		}
		else {
			startA[i] = get_start(L, i, p);
			endA[i] = get_end(L, i, p);
		}
		(*arg[i]).matrixA = &mMN;
		(*arg[i]).matrixB = &mNL;
		(*arg[i]).matrixC = &C;
		(*arg[i]).M = &M;
		(*arg[i]).N = &N;
		(*arg[i]).L = &L;
		(*arg[i]).start = &startA[i];
		(*arg[i]).end = &endA[i];
		pthread_create(&th[i], NULL, &do_thread, (void*) arg[i]);
	}

	for (int i = 0; i < p; i++) {
		pthread_join(th[i], NULL);
	}
	struct timespec end;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	double time_spent = (end.tv_sec - begin.tv_sec) + (end.tv_nsec - begin.tv_nsec)/1000000000.0;
	printf("Time spent (CPU): %lf\n", time_spent);

	/*int Summ = 0;
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < L; j++) {
			Summ = Summ + C[i][j];
		}
	}
	printf("Summ: %d\n", Summ);*/ //Sum check if needed

	mMN = freeMatrix(mMN, M);
	mNL = freeMatrix(mNL, N);
	C = freeMatrix(C, M);
}