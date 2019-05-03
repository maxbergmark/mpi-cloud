#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <complex>
#include <math.h>

#define max(a,b) \
	({ __typeof__ (a) _a = (a); \
		 __typeof__ (b) _b = (b); \
		_a > _b ? _a : _b; })

#define min(a,b) \
	({ __typeof__ (a) _a = (a); \
		 __typeof__ (b) _b = (b); \
		_a < _b ? _a : _b; })

// #define XRES 78
// #define YRES 64
#define XRES 310
#define YRES 240
#define MAX_ITER 99

#define XMIN -3.0
#define XMAX  2.0
#define YMIN -1.5
#define YMAX  1.5

int get_count(std::complex<double> c) {
	std::complex<double> z = 0 + 0i;
	int count = 0;
	while (abs(z) < 3 && count < MAX_ITER) {
		z = z*z + c;
		count++;
	}
	return count;
}

int main(int argc, char **argv) {
	MPI_Init(NULL, NULL);
	int world_rank, world_size;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	printf("starting %d/%d\n", world_rank, world_size);
	int start_index = (YRES / world_size) * world_rank;
	int end_index = min(start_index + (YRES / world_size), YRES);
	int size = end_index - start_index;
	int *counts = new int[size*XRES];
	// int *counts = malloc((size * XRES)*sizeof(int));
	int *all_counts;

	for (int y0 = 0; y0 < size; y0++) {
		int y = y0 + start_index;
		#pragma omp parallel for
		for (int x = 0; x < XRES; x++) {
			double real = (XMIN + (XMAX - XMIN) * (x / (double)XRES));
			double imag = (YMIN + (YMAX - YMIN) * (y / (double)YRES));
			std::complex<double> z1 = real + imag * 1i;
			int idx = y0 * XRES + x;
			counts[idx] = get_count(z1);
		}
	}

	if (world_rank == 0) {
		// all_counts = malloc(XRES * YRES * sizeof(int));
		all_counts = new int[XRES * YRES];
	}
	printf("At end: %d, %d\n", world_rank, size);
	// MPI_Barrier(MPI_COMM_WORLD);
	MPI_Gather(counts, size * XRES, MPI_INT, all_counts, size * XRES, MPI_INT, 0, MPI_COMM_WORLD); 
	MPI_Finalize();

	if (world_rank == 0) {
		for (int i = 0; i < YRES; i++) {
			for (int j = 0; j < XRES; j++) {
				int idx = i * XRES + j;
				printf("%2d", all_counts[idx]);
			}
			printf("\n");
		}
	}
}