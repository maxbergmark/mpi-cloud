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
	int *buffer = new int[size];
	double *times = new double[trials];

	if (world_size % 2 != 0) {
		printf("World size should be divisible by 2\n");
		exit(1);
	}
	if (world_rank % 2 == 0) {
		dest = world_rank + 1;
	} else {
		dest = world_rank - 1;
	}

	for (int i = 0; i < trials; i++) {
		double t0 = get_wall_time();
		if (world_rank % 2 == 0) {
			MPI_Send(buffer, size, MPI_INT, dest, 0, MPI_COMM_WORLD);
		} else {
			MPI_Recv(buffer, size, MPI_INT, dest, 0,
				MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
		double t1 = get_wall_time();
		times[i] = t1-t0;
		MPI_Barrier(MPI_COMM_WORLD);
	}
	double avg_time = 0;
	for (int i = 0; i < trials; i++) {
		avg_time += times[i];
	}
	avg_time /= trials;
	double avg_speed = (double)sizeof(int) * size / avg_time / 1024 / 1024;

	char hostname[1024];
    gethostname(hostname, 1024);
	printf("%s (%2d) avg: %.2fMB/s (%8.5fs)\n", 
		hostname, world_rank, avg_speed, avg_time);		
	double tot_speed;
	MPI_Reduce(&avg_speed, &tot_speed, 1, MPI_DOUBLE,
		MPI_SUM, 0, MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);
	if (world_rank == 0) {
		printf("Total speed: %.2fMB/s (%.2fGb/s)\n", 
			tot_speed, tot_speed / 128);
	}
	MPI_Finalize();
}