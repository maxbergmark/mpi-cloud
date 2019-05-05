CXX = mpic++
CC = mpicc
LDFLAGS = -lz -lm -fopenmp -O3

%.o: %.c
	$(CC) $(LDFLAGS) -o $@ -c $<

%.out: %.cpp
	$(CXX) $(LDFLAGS) -o $@ -c $<
