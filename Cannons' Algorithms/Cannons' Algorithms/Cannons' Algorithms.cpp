
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

int main(int argc, char* argv[])
{
    MPI_Comm cannon_comm;
    MPI_Status status;
    int rank, size;
    int shift;
    int i, j, k;
    int dims[2];
    int periods[2];
    int left, right, up, down;
    double* A, * B, * C;
    double* buf, * tmp;
    double start, end;
    unsigned int iseed = 0;
    int Nl, N;
    printf("Matrix size:");
    printf("\n");
    scanf_s("%d", &N);
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    srand(iseed);
    dims[0] = 0; dims[1] = 0;
    periods[0] = 1; periods[1] = 1;
    MPI_Dims_create(size, 2, dims);
    if (dims[0] != dims[1]) {
        if (rank == 0) printf("The number of processors must be a square.\n");
        MPI_Finalize();
        return 0;
    }
    Nl = N / dims[0];
    A = (double*)malloc(Nl * Nl * sizeof(double));
    B = (double*)malloc(Nl * Nl * sizeof(double));
    buf = (double*)malloc(Nl * Nl * sizeof(double));
    C = (double*)calloc(Nl * Nl, sizeof(double));
    for (i = 0; i < Nl; i++)
        for (j = 0; j < Nl; j++) {
            A[i * Nl + j] = 5 - (int)(10.0 * rand() / (RAND_MAX + 1.0));
            B[i * Nl + j] = 5 - (int)(10.0 * rand() / (RAND_MAX + 1.0));
            C[i * Nl + j] = 0.0;
        }
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &cannon_comm);
    MPI_Cart_shift(cannon_comm, 0, 1, &left, &right);
    MPI_Cart_shift(cannon_comm, 1, 1, &up, &down);
    start = MPI_Wtime();
    for (shift = 0; shift < dims[0]; shift++) {
        // Matrix multiplication
        for (i = 0; i < Nl; i++)
            for (k = 0; k < Nl; k++)
                for (j = 0; j < Nl; j++)
                    C[i * Nl + j] += A[i * Nl + k] * B[k * Nl + j];
        if (shift == dims[0] - 1) break;
        // Communication
        MPI_Sendrecv(A, Nl * Nl, MPI_DOUBLE, left, 1, buf, Nl * Nl, MPI_DOUBLE, right, 1, cannon_comm, &status);
        tmp = buf; buf = A; A = tmp;
        MPI_Sendrecv(B, Nl * Nl, MPI_DOUBLE, up, 2, buf, Nl * Nl, MPI_DOUBLE, down, 2, cannon_comm, &status);
        tmp = buf; buf = B; B = tmp;
    }
    MPI_Barrier(cannon_comm);
    end = MPI_Wtime();
    if (rank == 0) printf("Time: %.4fs\n", end - start);
    free(A); free(B); free(buf); free(C);
    MPI_Finalize();
    return 0;
}

