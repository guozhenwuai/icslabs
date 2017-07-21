/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
	/* Declare variables. */
	int i,j,k;  //variables for loop
	int var1,var2,var3,var4,var5,var6,var7,var8;   //variables for saving data

	/* Take different optimization strategy for each case. */
	/* case for matrice with M=N=32. */
	if(M==32&&N==32){
		for(j=0;j<M;j+=8){
			for(i=0;i<N;i++){

				/* load data */
				var1 = A[i][j];
				var2 = A[i][j+1];
				var3 = A[i][j+2];
				var4 = A[i][j+3];
				var5 = A[i][j+4];
				var6 = A[i][j+5];
				var7 = A[i][j+6];
				var8 = A[i][j+7];

				/* write data */
				B[j][i] = var1;
				B[j+1][i] = var2;
				B[j+2][i] = var3;
				B[j+3][i] = var4;
				B[j+4][i] = var5;
				B[j+5][i] = var6;
				B[j+6][i] = var7;
				B[j+7][i] = var8;

			}
		}
		return;
	}
	/* case for matrice with M=N=64. */
	else if(M==64&&N==64){
		for(j=0;j<M;j+=8){
			for(i=0;i<N;i+=8){
				for(k=0;k<4;k++){
					var1 = A[i+k][j];
					var2 = A[i+k][j+1];
					var3 = A[i+k][j+2];
					var4 = A[i+k][j+3];
					var5 = A[i+k][j+4];
					var6 = A[i+k][j+5];
					var7 = A[i+k][j+6];
					var8 = A[i+k][j+7];

					B[j][i+k] = var1;
					B[j+1][i+k] = var2;
					B[j+2][i+k] = var3;
					B[j+3][i+k] = var4;

					B[j][i+k+4] = var8;
					B[j+1][i+k+4] = var7;
					B[j+2][i+k+4] = var6;
					B[j+3][i+k+4] = var5;
				}
				for(k=0;k<4;k++){
					if(i==j){
						var1 = A[i+k+4][j];
						var2 = A[i+k+4][j+1];
						var3 = A[i+k+4][j+2];
						var4 = A[i+k+4][j+3];
						var5 = A[i+k+4][j+4];
						var6 = A[i+k+4][j+5];
						var7 = A[i+k+4][j+6];
						var8 = A[i+k+4][j+7];

						B[j+4][i+k] = var4;
						B[j+5][i+k] = var3;
						B[j+6][i+k] = var2;
						B[j+7][i+k] = var1;

						B[j+4][i+k+4] = var5;
						B[j+5][i+k+4] = var6;
						B[j+6][i+k+4] = var7;
						B[j+7][i+k+4] = var8;
					}
					else{
						var1 = B[j+k][i+4];
						var2 = B[j+k][i+5];
						var3 = B[j+k][i+6];
						var4 = B[j+k][i+7];

						var5 = A[i+4][j+7-k];
						var6 = A[i+5][j+7-k];
						var7 = A[i+6][j+7-k];
						var8 = A[i+7][j+7-k];

						B[j+k][i+4] = A[i+4][j+k];
						B[j+k][i+5] = A[i+5][j+k];
						B[j+k][i+6] = A[i+6][j+k];
						B[j+k][i+7] = A[i+7][j+k];

						B[j+7-k][i] = var1;
						B[j+7-k][i+1] = var2;
						B[j+7-k][i+2] = var3;
						B[j+7-k][i+3] = var4;
						B[j+7-k][i+4] = var5;
						B[j+7-k][i+5] = var6;
						B[j+7-k][i+6] = var7;
						B[j+7-k][i+7] = var8;
					}

				}
				if(i==j){
					for(k=0;k<4;k++){
						var1 = B[j+k][i+4];
						var2 = B[j+k][i+5];
						var3 = B[j+k][i+6];
						var4 = B[j+k][i+7];
						var5 = B[j+7-k][i];
						var6 = B[j+7-k][i+1];
						var7 = B[j+7-k][i+2];
						var8 = B[j+7-k][i+3];

						B[j+k][i+4] = var5;
						B[j+k][i+5] = var6;
						B[j+k][i+6] = var7;
						B[j+k][i+7] = var8;
						B[j+7-k][i] = var1;
						B[j+7-k][i+1] = var2;
						B[j+7-k][i+2] = var3;
						B[j+7-k][i+3] = var4;
					}
				}
			}
		}
		return;
	}
	/* case for matrice with M=61 and N=67. */
	else if(M==61&&N==67){
		for(j=0;j<56;j+=8){
			for(i=64;i<67;++i){
				var1 = A[i][j];
				var2 = A[i][j+1];
				var3 = A[i][j+2];
				var4 = A[i][j+3];
				var5 = A[i][j+4];
				var6 = A[i][j+5];
				var7 = A[i][j+6];
				var8 = A[i][j+7];

				B[j][i] = var1;
				B[j+1][i] = var2;
				B[j+2][i] = var3;
				B[j+3][i] = var4;
				B[j+4][i] = var5;
				B[j+5][i] = var6;
				B[j+6][i] = var7;
				B[j+7][i] = var8;
			}
		}
		for(i=0;i<64;i+=8){
			for(j=0;j<M;j++){
				var1 = A[i][j];
				var2 = A[i+1][j];
				var3 = A[i+2][j];
				var4 = A[i+3][j];
				var5 = A[i+4][j];
				var6 = A[i+5][j];
				var7 = A[i+6][j];
				var8 = A[i+7][j];

				B[j][i] = var1;
				B[j][i+1] = var2;
				B[j][i+2] = var3;
				B[j][i+3] = var4;
				B[j][i+4] = var5;
				B[j][i+5] = var6;
				B[j][i+6] = var7;
				B[j][i+7] = var8;

			}
		}
		for(j=56;j<61;j++){
			var1 = A[64][j];
			var2 = A[65][j];
			var3 = A[66][j];

			B[j][64] = var1;
			B[j][65] = var2;
			B[j][66] = var3;
		}
		return;

	}
	/* case not included in the evaluation*/
	/* take the simple baseline transpose function with no optimazition for the cache. */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            var1 = A[i][j];
            B[j][i] = var1;
        }
    }    

}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

