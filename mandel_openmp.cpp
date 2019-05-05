#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <complex>
#include <math.h>
#include <ctime>
#include <cstdlib>
#include <sys/time.h>

#define max(a,b) \
	({ __typeof__ (a) _a = (a); \
		 __typeof__ (b) _b = (b); \
		_a > _b ? _a : _b; })

#define min(a,b) \
	({ __typeof__ (a) _a = (a); \
		 __typeof__ (b) _b = (b); \
		_a < _b ? _a : _b; })

#define MAX_ITER 99

#define XMIN -3.0
#define XMAX  2.0
#define YMIN -1.5
#define YMAX  1.5

double get_wall_time(){
	struct timeval time;
	if (gettimeofday(&time,NULL)){
		//  Handle error
		return 0;
	}
	return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

char get_count(std::complex<double> c) {
	std::complex<double> z = 0 + 0i;
	char count = 0;
	while (abs(z) < 3 && count < MAX_ITER) {
		z = z*z + c;
		count++;
	}
	return count;
}

int main(int argc, char **argv) {

	int XRES = atoi(argv[1]);
	int YRES = XRES;

	clock_t t0 = clock(); double w0 = get_wall_time();

	MPI_Init(NULL, NULL);
	int world_rank, world_size;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	clock_t t1 = clock(); double w1 = get_wall_time();


	// printf("starting %d/%d\n", world_rank, world_size);
	int start_index = (YRES / world_size) * world_rank;
	int end_index = min(start_index + (YRES / world_size), YRES);
	int size = end_index - start_index;
	char *counts = new char[size*XRES];
	// int *counts = malloc((size * XRES)*sizeof(int));
	char *all_counts;
	clock_t t2 = clock(); double w2 = get_wall_time();

	#pragma omp parallel for schedule(dynamic,64)
	for (int y0 = 0; y0 < size; y0++) {
		int y = y0 + start_index;
		for (int x = 0; x < XRES; x++) {
			double real = (XMIN + (XMAX - XMIN) * (x / (double)XRES));
			double imag = (YMIN + (YMAX - YMIN) * (y / (double)YRES));
			std::complex<double> z1 = real + imag * 1i;
			int idx = y0 * XRES + x;
			counts[idx] = get_count(z1);
		}
	}

	clock_t t3 = clock(); double w3 = get_wall_time();

	if (world_rank == 0) {
		// all_counts = malloc(XRES * YRES * sizeof(int));
		all_counts = new char[XRES * YRES];
	}
	clock_t t4 = clock(); double w4 = get_wall_time();


	printf("At end: %d, %d\n", world_rank, size);
	// MPI_Barrier(MPI_COMM_WORLD);
	MPI_Gather(counts, size * XRES, MPI_CHAR, all_counts, size * XRES, MPI_CHAR, 0, MPI_COMM_WORLD); 
	MPI_Finalize();

	clock_t t5 = clock(); double w5 = get_wall_time();
	int sum = 0;
	if (world_rank == 0) {
		for (int i = 0; i < YRES; i++) {
			for (int j = 0; j < XRES; j++) {
				int idx = i * XRES + j;
				sum += all_counts[idx];
				// printf("%2d", all_counts[idx]);
			}
			// printf("\n");
		}
		printf("sum: %d\n", sum);
	}
	clock_t t6 = clock(); double w6 = get_wall_time();
	printf("1: %8.5f, %8.5f\n", double(t1 - t0) / CLOCKS_PER_SEC, w1 - w0);
	printf("2: %8.5f, %8.5f\n", double(t2 - t1) / CLOCKS_PER_SEC, w2 - w1);
	printf("3: %8.5f, %8.5f\n", double(t3 - t2) / CLOCKS_PER_SEC, w3 - w2);
	printf("4: %8.5f, %8.5f\n", double(t4 - t3) / CLOCKS_PER_SEC, w4 - w3);
	printf("5: %8.5f, %8.5f\n", double(t5 - t4) / CLOCKS_PER_SEC, w5 - w4);
	printf("6: %8.5f, %8.5f\n", double(t6 - t5) / CLOCKS_PER_SEC, w6 - w5);
}