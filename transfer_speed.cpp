#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <cstdlib>
#include <sys/time.h>

double get_wall_time(){
	struct timeval time;
	if (gettimeofday(&time,NULL)){
		//  Handle error
		return 0;
	}
	return (double)time.tv_sec + (double)time.tv_usec * .000001;
}


int main(int argc, char **argv) {

	if (argc < 2) {
		printf("Usage: %s <size> <trials>", argv[0]);
		exit(1);
	}
	int size = atoi(argv[1]);
	int trials = atoi(argv[2]);
	int dest;
	int *send_buffer = new int[size];
	int *receive_buffer = new int[size];
	double *times = new double[trials];


	MPI_Init(NULL, NULL);
	int world_rank, world_size;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
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
		double w0 = get_wall_time();
		if (world_rank % 2 == 0) {
			MPI_Send(send_buffer, size, MPI_INT, dest, 0, MPI_COMM_WORLD);
			MPI_Recv(receive_buffer, size, MPI_INT, dest, 0,
				MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		} else {
			MPI_Recv(receive_buffer, size, MPI_INT, dest, 0,
				MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Send(send_buffer, size, MPI_INT, dest, 0, MPI_COMM_WORLD);
		}
		clock_t t1 = clock(); double w1 = get_wall_time();
		times[i] = w1-w0;
	}
	double avg_time = 0;
	for (int i = 0; i < trials; i++) {
		avg_time += times[i];
	}
	avg_time /= trials;
	double avg_speed = sizeof(int) * size / avg_time / 1024 / 1024;
	printf("average: %.2fMB/s (%8.5fs)\n", avg_speed, avg_time);		
	double tot_speed;
	MPI_Reduce(&avg_speed, &tot_speed, 1, MPI_DOUBLE,
		MPI_SUM, 0, MPI_COMM_WORLD);

	if (world_rank == 0) {
		printf("Total speed: %.2fMB/s\n", tot_speed);
	}
	MPI_Finalize();
}