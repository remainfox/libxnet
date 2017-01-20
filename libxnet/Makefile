

CROSS_COMPILE:=arm-hisiv100nptl-linux-
#CROSS_COMPILE:=arm-hisiv400-linux-
CC:=$(CROSS_COMPILE)gcc
CPP:=$(CROSS_COMPILE)g++
AR :=$(CROSS_COMPILE)ar
SRC:=$(wildcard *.c)

OBJ_DIR=objects
OBJ:=$(SRC:%.c=$(OBJ_DIR)/%.o)


CFLAGS := -g
#CFLAGS += -DJPEG_MODE

EXE := xnet_server
LIB := libxnet.a

all: $(EXE)

$(EXE):$(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) -lpthread
	cp $(EXE) /source/rootfs/

$(LIB):$(OBJ)
	$(AR) -cr $@ $^
	cp $(LIB) ../sample/wbc

$(OBJ):$(OBJ_DIR)/%.o:%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@


clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(EXE)
	rm -rf $(LIB)







