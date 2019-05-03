// Author: Wes Kendall
// Copyright 2011 www.mpitutorial.com
// This code is provided freely with the tutorials on mpitutorial.com. Feel
// free to modify it for your own use. Any distribution of the code must
// either provide a link to www.mpitutorial.com or keep this header intact.
//
// Ping pong example with MPI_Send and MPI_Recv. Two processes ping pong a
// number back and forth, incrementing it until it reaches a given value.
//
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  const int PING_PONG_LIMIT = 2000;

  // Initialize the MPI environment
  MPI_Init(NULL, NULL);
  // Find out rank, size
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // We are assuming at least 2 processes for this task
//  if (world_size != 2) {
//    fprintf(stderr, "World size must be two for %s\n", argv[0]);
//    MPI_Abort(MPI_COMM_WORLD, 1);
//  }

  int ping_pong_count = 0;
  int next_anticipated_value = world_rank;
  int partner_rank = (world_rank + 1) % world_size;
  int previous_rank = (world_size + world_rank - 1) % world_size;
  while (next_anticipated_value <= PING_PONG_LIMIT) {
//    if (world_rank == 2) {
//      printf("ping_pong: %d\n", ping_pong_count);
//    }
//    printf("%d looping: %d\n", world_rank, ping_pong_count);
    if (world_rank == ping_pong_count % world_size) {
      // Increment the ping pong count before you send it
      ping_pong_count++;
      next_anticipated_value += world_size-1;
//      printf("%d sending\n", world_rank);
      MPI_Send(&ping_pong_count, 1, MPI_INT, partner_rank, 0, MPI_COMM_WORLD);
      printf("%d sent and incremented ping_pong_count %d to %d\n",
             world_rank, ping_pong_count, partner_rank);
//    } else if (PING_PONG_LIMIT - ping_pong_count+1 >= world_size) {
} else {
//      printf("%d waiting\n", world_rank);
      MPI_Recv(&ping_pong_count, 1, MPI_INT, previous_rank, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      printf("%d received ping_pong_count %d from %d (%d)\n",
             world_rank, ping_pong_count, previous_rank, next_anticipated_value);
      next_anticipated_value++;
    }
  }
//  printf("%d finalized: %d\n", world_rank, ping_pong_count);
  MPI_Finalize();
}
