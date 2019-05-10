#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <mpi.h>
#include <ctime>
#include <cstdlib>
#include <sys/time.h>
#include <unistd.h>

double get_wall_time(){
	struct timeval time;
	if (gettimeofday(&time,NULL)){
		//  Handle error
		return 0;
	}
	return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

void print_hostname(int world_rank) {
	char hostname[1024];
	gethostname(hostname, 1024);
	printf("%s (%2d)\n", hostname, world_rank);
}

double test_transfer_speed(int *buffer, uint64_t n, int dest, bool sending) {
	double t0 = get_wall_time();
	if (sending) {
		MPI_Send(buffer, n, MPI_INT, dest, 0, MPI_COMM_WORLD);
	} else {
		MPI_Recv(buffer, n, MPI_INT, dest, 0,
			MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
	double t1 = get_wall_time();
	MPI_Barrier(MPI_COMM_WORLD);
	return t1-t0;
}

void run_neighbor_test(int world_rank, int world_size, int dest, bool sending, 
	int trials, uint64_t size, double *times, int *buffer) {
	

	for (int i = 0; i < trials; i++) {
		for (int n = 0; n < size; n++) {
			times[n] += test_transfer_speed(buffer, 1 << n, dest, sending);
		}
	}

	double *avg_speed = new double[size];
	for (int i = 0; i < size; i++) {
		times[i] /= trials;
		avg_speed[i] = (double)(1 << i) / times[i] * sizeof(int) / 1024 / 1024;
	}


	double *avg_time = new double[size];
	MPI_Reduce(times, avg_time, size, MPI_DOUBLE,
		MPI_SUM, 0, MPI_COMM_WORLD);

	for (int i = 0; i < size; i++) {
		avg_time[i] /= world_size;
	}

	double *tot_speed = new double[size];
	MPI_Reduce(avg_speed, tot_speed, size, MPI_DOUBLE,
		MPI_SUM, 0, MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);
	if (world_rank == 0) {
		for (int i = 0; i < size; i++) {
			printf("Total speed: %8.2fMB/s\t(%6.2fGb/s)\t%.8fµs\n", 
				tot_speed[i], tot_speed[i] / 128, avg_time[i]*1e6);
		}
	}
}

int main(int argc, char **argv) {

	MPI_Init(NULL, NULL);
	int world_rank, world_size;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	if (argc != 3) {
		if (world_rank == 0) {
			printf("Usage: %s <size> <trials>\n", argv[0]);
		}
		exit(1);
	}
	uint64_t size = atol(argv[1]);
	int trials = atoi(argv[2]);
	int dest;
	bool sending;
	int *buffer = new int[1 << size];
	double *times = new double[size]();

	if (world_size % 2 != 0) {
		if (world_rank == 0) {
			printf("World size should be divisible by 2\n");
		}
		exit(1);
	}

	if (world_size % 2 == 0) {
		if (world_rank == 0) {
			printf("\nRunning external transfer speed test\n");
		}
		if (world_rank % 2 == 0) {
			sending = true;
			dest = world_rank + 1;
		} else {
			sending = false;
			dest = world_rank - 1;
		}
		run_neighbor_test(world_rank, world_size, dest, sending, 
			trials, size, times, buffer);
	}

	if (world_size % 4 == 0) {
		if (world_rank == 0) {
			printf("\nRunning internal transfer speed test\n");
		}
		if (world_rank % 4 < 2) {
			sending = true;
			dest = world_rank + 2;
		} else {
			sending = false;
			dest = world_rank - 2;
		}
		run_neighbor_test(world_rank, world_size, dest, sending, 
			trials, size, times, buffer);
	}

	if (world_rank == 0) {
		printf("\n");
	}
	MPI_Finalize();
}