#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

int M = 1000, N = 1000, L = 1000, p = 10;
int **mMN, **mNL, **C;

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
};

void do_count(int i_st, int i_ed, int j_st, int j_ed) {
	for (int i = i_st; i < i_ed; ++i) {
		for (int j = j_st; j < j_ed; ++j) {
			C[i][j] = 0;
			for (int k = 0; k < N; ++k) {
				C[i][j] = C[i][j] + mMN[i][k]*mNL[k][j];
			}
		}
	}		
}

/*void* do_thread(void* arg) {
	struct param *my_param = (struct param*)arg;
	int LocalSum = 0;
	for (int i = 0; i < M; ++i) {
		for (int j = 0; j < L; ++j) {
			for (int k = *(*my_param).start; k < *(*my_param).end; ++k) {
				LocalSum = LocalSum + mMN[i][k]*mNL[k][j];
				}
			C[i][j] = C[i][j] + LocalSum;
			LocalSum = 0;
			}
		}		
	free(arg);
}*/

int get_something(int diff, int thread_num, int bin) {
	if (thread_num < diff) {
		return(thread_num + bin);
	}
	else {
		return(diff);
	}
}

int get_start(int long_side, int thread_num, int thrds) {
	int diff = long_side % thrds;
	int start = thread_num * (long_side / thrds);
	start = start + get_something(diff, thread_num, 0);
	return start;
}

int get_end(int long_side, int thread_num, int thrds) {
	int diff = long_side % thrds;
	int end = (thread_num + 1) * (long_side / thrds);
	end = end + get_something(diff, thread_num, 1);
	return end;
}

void main() {
	
	srand(time(NULL));
	double s_timer = omp_get_wtime();	
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
	int threads = 0;
	omp_set_num_threads(p);
	#pragma omp parallel 
	{
		int ID = omp_get_thread_num();
		int nthrds = omp_get_num_threads();
		if (ID == 0) { threads = nthrds; }
		int start, end;
		if (M > L) {
			start = get_start(M, ID, nthrds);	
			end = get_end(M, ID, nthrds);
			do_count(start, end, 0, L);	
		}
		else {
			start = get_start(L, ID, nthrds);	
			end = get_end(L, ID, nthrds);
			do_count(0, M, start, end);	
		}	
	}

	int Summ = 0;
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < L; j++) {
			Summ = Summ + C[i][j];
		}
	}
	printf("Summ: %d\n", Summ); //Sum check if needed

	mMN = freeMatrix(mMN, M);
	mNL = freeMatrix(mNL, N);
	C = freeMatrix(C, M);

	struct timespec end;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
	double e_timer = omp_get_wtime();	

	double time_spent = (end.tv_sec - begin.tv_sec) + (end.tv_nsec - begin.tv_nsec)/1000000000.0;
	printf("Time spent (CPU): %lf\n", time_spent);
	double omp_time = e_timer - s_timer;
	printf("Time spent (OMP): %lf\n", omp_time);
	printf("Actual threads amount created: %d\n", threads);
}
