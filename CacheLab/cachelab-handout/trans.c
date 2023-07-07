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

char solve32_desc[] = "Transpose32 submission";
void solve32(int M, int N, int A[N][M], int B[M][N])
{
    for(int i = 0; i < M; i += 8)
    {
        for(int j = 0; j < N; j += 8)
        {
            for(int k = 0; k < 8; k ++)
            {
                int a1 = A[k + i][j];
                int a2 = A[k + i][j + 1];
                int a3 = A[k + i][j + 2];
                int a4 = A[k + i][j + 3];
                int a5 = A[k + i][j + 4];
                int a6 = A[k + i][j + 5];
                int a7 = A[k + i][j + 6];
                int a8 = A[k + i][j + 7];
                B[j][k + i] = a1;
                B[j + 1][k + i] = a2;
                B[j + 2][k + i] = a3;
                B[j + 3][k + i] = a4;
                B[j + 4][k + i] = a5;
                B[j + 5][k + i] = a6;
                B[j + 6][k + i] = a7;
                B[j + 7][k + i] = a8;
            }
        }
    }
}

char solve64_desc[] = "Transpose64 submission";
void solve64(int M, int N, int A[N][M], int B[M][N])
{
    int a1, a2, a3, a4, a5, a6, a7, a8;
    for(int i = 0; i < M; i += 8)
    {
        for(int j = 0; j < N; j += 8)
        {
            //A的左上 -> B的左上
            //A的右上 -> B的右上
            for(int k = i; k < i + 4; k ++)//k枚举A中的行
            {
                a1 = A[k][j];
                a2 = A[k][j + 1];
                a3 = A[k][j + 2];
                a4 = A[k][j + 3];
                a5 = A[k][j + 4];
                a6 = A[k][j + 5];
                a7 = A[k][j + 6];
                a8 = A[k][j + 7];
                B[j][k] = a1;
                B[j + 1][k] = a2;
                B[j + 2][k] = a3;
                B[j + 3][k] = a4;
                B[j][k + 4] = a5;
                B[j + 1][k + 4] = a6;
                B[j + 2][k + 4] = a7;
                B[j + 3][k + 4] = a8;
            }

            //B的右上 -> B的左下
            //A的左下 -> B的右上
            for(int k = j; k < j + 4; k ++)//k枚举A中的列
            {
                //若A中枚举坐标为(i, k)，则B中对应的坐标为(k, i)
                //B的右上
                a1 = B[k][i + 4];
                a2 = B[k][i + 5];
                a3 = B[k][i + 6];
                a4 = B[k][i + 7];
                //A的左下
                a5 = A[i + 4][k];
                a6 = A[i + 5][k];
                a7 = A[i + 6][k];
                a8 = A[i + 7][k];
                //B的右上
                B[k][i + 4] = a5;
                B[k][i + 5] = a6;
                B[k][i + 6] = a7;
                B[k][i + 7] = a8;
                //B的左下
                B[k + 4][i] = a1;
                B[k + 4][i + 1] = a2;
                B[k + 4][i + 2] = a3;
                B[k + 4][i + 3] = a4;

            }
            //A的右下 -> B的右下
            for(int k = i; k < i + 4; k ++)//k枚举A中的行
            {
                a1 = A[k + 4][j + 4];
                a2 = A[k + 4][j + 5];
                a3 = A[k + 4][j + 6];
                a4 = A[k + 4][j + 7];    
                B[j + 4][k + 4] = a1;
                B[j + 5][k + 4] = a2;
                B[j + 6][k + 4] = a3;
                B[j + 7][k + 4] = a4;
            }
        }
    }
}

char solveMisc_desc[] = "TransposeMisc submission";
void solveMisc(int M, int N, int A[N][M], int B[M][N])
{
    for(int i = 0; i < N; i += 18)
    {
        for(int j = 0; j < M; j += 18)
        {
            for(int n = i; n < i + 18 && n < N; n ++)
            {
                for(int m = j; m < j + 18 && m < M; m ++)
                {
                    B[m][n] = A[n][m];
                }
            }
        }
    }
}

char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if(M == 32 && N == 32) solve32(M, N, A, B);
    else if(M == 64 && N == 64) solve64(M, N, A, B);
    else if(M == 61 && N == 67)solveMisc(M, N, A, B);
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
    //registerFunctions(transpose64_submit, transpose64_submit_desc);
    /* Register any additional transpose functions */
    //registerTransFunction(trans, trans_desc); 

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

