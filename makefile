CC=g++
NVCC=nvcc
CFLAGS=-c -Wall -g -Og -std=c++11 -Wextra -march=native -mtune=native
CUDA_CFLAGS=-c -g -G -O0 -std=c++11 -m64 -arch=compute_30 -code=sm_30
LDFLAGS=-lpthread
CUDA_LDFLAGS=-lcuda -lcudart
EXECUTABLE=bin/cugzip
DIRS=bin/ build/
CUDA_INCLUDES=-I/opt/cuda/include
OBJS=build/main.cpp.o build/thread_functions.cpp.o build/lz77.cu.o build/aux.cpp.o


all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	@mkdir -p $(DIRS)
	$(NVCC) $(OBJS) -o $(EXECUTABLE) $(LDFLAGS) $(CUDA_LDFLAGS)

build/main.cpp.o: src/main.cpp
	@mkdir -p $(DIRS)
	$(CC) $(CFLAGS) src/main.cpp -o build/main.cpp.o

build/thread_functions.cpp.o: src/thread_functions.cpp
	@mkdir -p $(DIRS)
	$(CC) $(CFLAGS) src/thread_functions.cpp -o build/thread_functions.cpp.o

build/lz77.cu.o: src/lz77.cu
	@mkdir -p $(DIRS)
	$(NVCC) $(CUDA_CFLAGS) src/lz77.cu -o build/lz77.cu.o $(CUDA_INCLUDES)

build/aux.cpp.o: src/aux.cpp
	@mkdir -p $(DIRS)
	$(CC) $(CFLAGS) src/aux.cpp -o build/aux.cpp.o

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
