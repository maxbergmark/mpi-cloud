#include <stdio.h>
#include <math.h>
#include <mpi.h>




int is_prime(int n) {
  if (n < 2 || n % 2 == 0)
    return 0;
  if (n % 3 == 0 || n % 5 == 0 || n % 7 == 0) {
    return 0;
  }

  int last = (int) sqrt(n) + 1;  /* conservatively safe */

  for (int j = 11; j <= last; j += 2)
    if (0 == n % j)
      return 0;

  return 1;
}


int main(int argc, char** argv) {

  MPI_Init(NULL, NULL);

  // Get the number of processes
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // Get the rank of the process
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  for (int i = world_rank; i < 10000000; i += world_size) {
    is_prime(i);
  }
  printf("%d finished\n", world_rank);
  MPI_Finalize();
}
