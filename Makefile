PROJECT_DIR=$(realpath $(dir $(MAKEFILE_LIST)))

CC=icpc
CCFLAGS=-std=c++11 -I$(PROJECT_DIR)/src/lib -fopenmp
MODE?=DEBUG

NODE_COUNT?=1
CORE_COUNT_PER_NODE?=8

make:
	module load icc && \
	$(CC) $(CCFLAGS) $(OPT_CCFLAGS) -D$(MODE) -o $(PROJECT_DIR)/src/program.o ${PROJECT_DIR}/src/program.cpp

test: make
	$(PROJECT_DIR)/submit.sh --nodes $(NODE_COUNT) --cores-per-node $(CORE_COUNT_PER_NODE) -s --duplicate-jobs 3 --bond-energy 0.8 \
		--iterations 10000 --splits 5000

submit: make
	$(PROJECT_DIR)/submit.sh --nodes $(NODE_COUNT) --cores-per-node $(CORE_COUNT_PER_NODE) -s --duplicate-jobs 3 --bond-energy 0.5

mass_submit: make
	$(PROJECT_DIR)/submit.sh --nodes $(NODE_COUNT) --cores-per-node $(CORE_COUNT_PER_NODE)

clean:
	-rm $(PROJECT_DIR)/src/program.o