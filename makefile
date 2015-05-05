CC=g++
NVCC=nvcc
CFLAGS=-c -Wall -g -O2 -std=c++11 -Wextra -march=native -mtune=native -pg
CUDA_CFLAGS=-c -Wall -g -G -O2 -std=c++11 -Wextra -m64 -arch=compute_30 -code=sm_30
LDFLAGS=-lpthread -pg
CUDA_LDFLAGS=-lcuda -lcudart
EXECUTABLE=bin/cugzip
DIRS=bin/ build/
CUDA_INCLUDES=-I/opt/cuda/include
OBJS=build/main.o build/thread_functions.o


all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	@mkdir -p $(DIRS)
	$(CC) $(OBJS) -o $(EXECUTABLE) $(LDFLAGS)

build/main.o: src/main.cpp
	@mkdir -p $(DIRS)
	$(CC) $(CFLAGS) src/main.cpp -o build/main.o

build/thread_functions.o: src/thread_functions.cpp
	@mkdir -p $(DIRS)
	$(CC) $(CFLAGS) src/thread_functions.cpp -o build/thread_functions.o

clean:
	rm -f build/* bin/*

#$(CUDA_EXE): build/particle.o build/octree.o build/vector.o build/thread_functions.o build/cuda_code.o build/cuda_helper.o
#	mkdir -p $(DIRS)
#	$(CC) $(CFLAGS) -DCUDA src/main.cpp -o build/main.o
#	$(NVCC) build/cuda_code.o build/cuda_helper.o build/main.o build/vector.o build/particle.o build/octree.o build/thread_functions.o -o $(CUDA_EXE) $(CUDA_LDFLAGS) $(LDFLAGS)
#
#build/cuda_code.o: src/cuda_code.cu
#	mkdir -p $(DIRS)
#	$(NVCC) $(CUDA_CFLAGS) src/cuda_code.cu -o build/cuda_code.o $(CUDA_INCLUDES)
