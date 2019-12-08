PROJECT_DIR=$(realpath $(dir $(MAKEFILE_LIST)))

CC=icpc
CCFLAGS=-std=c++11 -I$(PROJECT_DIR)/src/lib
MODE?=DEBUG

NODE_COUNT?=1
CORE_COUNT_PER_NODE?=1

BOND_ENERGY?=0.5
DUPLICATE_JOBS?=1

make:
	module load icc && \
	$(CC) $(CCFLAGS) $(OPT_CCFLAGS) -D$(MODE) -o $(PROJECT_DIR)/src/program.o ${PROJECT_DIR}/src/program.cpp

submit: make
	$(PROJECT_DIR)/submit.sh --nodes $(NODE_COUNT) --cores-per-node $(CORE_COUNT_PER_NODE) -s --duplicate-jobs $(DUPLICATE_JOBS) --bond-energy $(BOND_ENERGY)

mass_submit: make
	$(PROJECT_DIR)/submit.sh --nodes $(NODE_COUNT) --cores-per-node $(CORE_COUNT_PER_NODE) --duplicate-jobs $(DUPLICATE_JOBS)

clean:
	-rm $(PROJECT_DIR)/src/program.o