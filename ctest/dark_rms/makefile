# Makefile for dark average operation
# Engineer: Sufeng Niu
# Company: Argonne National Lab

.PHONY: all tar clean

# Paths
OBJ_PATH = obj/
SRC_PATH = src/
LIB_PATH = lib/
INCLUDE_PATH = include/

# Flags
CC=gcc
CFLAGS=-g -O3 -I/include -I/lib

TARGET=mthreads_dark_rms
SOURCE=tif
COMP=rms

LIBS=-lm -ltiff -lrt -L. -ltpool

H_FILES := $(wildcard include/*.h)
DEPS := $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))
C_FILES := $(wildcard src/*.c)
OBJ_FILES := $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))

all: libtpool.a $(TARGET)

$(OBJ_PATH)%.o: $(LIB_PATH)%.c $(LIB_PATH)%.h
	$(CC) $(CFALGS) -c $< -o $@

libtpool.a: $(OBJ_PATH)thr_pool.o
	ar crv $@ $<

# linker
$(TARGET): $(OBJ_FILES) libtpool.a
	@echo "====================="
	@echo "Linking the target $@"
	@echo "====================="
	$(CC) $(CFLAGS) -lpthread $^ -o $@ $(LIBS)
	@echo -- Link finished --

# generic compilation rule
$(OBJ_PATH)%.o: $(SRC_PATH)%.c $(DEPS)
	@echo "================"
	@echo "compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

tar: 
	@echo "compress the project"
	tar cvf *.c *.h lib/ makefile

clean:
	rm -rf $(OBJ_PATH)*.o *.a $(TARGET) dark_avg.tif dark_rms.tif *.tar
