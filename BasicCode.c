#include <time.h>
#include <stdio.h>
#include <stdlib.h>

void randomizeMatrix(int **matrix, int row, int collumn) {
	for (int i = 0; i < row; ++i) {
	        for (int j = 0; j < collumn; ++j) {
        	        matrix[i][j] = rand()%100;
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

void freeMatrix(int **matrix, int row) {
	for (int i = 0; i < row; ++i) {
		free(matrix[i]);
	}
	free(matrix);
}

void main() {
int M = 100, N = 100, L = 100;
srand(time(NULL));
struct timespec begin;
clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &begin);

int **mMN, **mNL, **C;
mMN = mallocMatrix(mMN, M, N);
mNL = mallocMatrix(mNL, N, L);
C = mallocMatrix(C, M, L);

randomizeMatrix(mMN, M, N);
randomizeMatrix(mNL, N, L);

for (int i = 0; i < M; ++i) {
	for (int j = 0; j < L; ++j) {
		C[i][j] = 0;
		for (int k = 0; k < N; ++k) {
			C[i][j] = C[i][j] + mMN[i][k]*mNL[k][j];
			}
		}
	}

freeMatrix(mMN, M);
freeMatrix(mNL, N);
freeMatrix(C, M);

struct timespec end;
clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

double time_spent = (end.tv_sec - begin.tv_sec) + (end.tv_nsec - begin.tv_nsec)/1000000000.0;
printf("Time spent (CPU): %lf\n", time_spent);
}