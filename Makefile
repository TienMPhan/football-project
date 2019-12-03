PROJECT_DIR=$(realpath $(dir $(MAKEFILE_LIST)))

CC=icpc
CCFLAGS=-std=c++11 -I$(PROJECT_DIR)/lib
MODE?=DEBUG

NODE_COUNT?=1
CORE_COUNT_PER_NODE?=1

BOND_ENERGY?=0.5

make:
	$(CC) $(CCFLAGS) $(OPT_CCFLAGS) -D$(MODE) -o $(PROJECT_DIR)/program.o ${PROJECT_DIR}/program.cpp

submit: make
	$(PROJECT_DIR)/submit.sh --nodes $(NODE_COUNT) --cores-per-node $(CORE_COUNT_PER_NODE) -s --bond-energy $(BOND_ENERGY)

mass_submit: make
	$(PROJECT_DIR)/submit.sh --nodes $(NODE_COUNT) --cores-per-node $(CORE_COUNT_PER_NODE)

clean:
	-rm $(PROJECT_DIR)/program.o