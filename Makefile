TOP=.
include $(TOP)/mk/env.mk
export PROJ_DIR=$(PWD)



################PROGS##################
PROGS = $(TOP)/bin/linker

################DIRS###################
LINKER_DIRS = $(TOP)/src/linker
OBJ_DIRS = $(TOP)/src/obj
UTIL_DIRS = $(TOP)/src/util

SRC_DIRS = $(LINKER_DIRS) $(OBJ_DIRS) $(UTIL_DIRS)

############PROGS USE LOCAL LIBS##########
PROG_LINKER_LDFLAGS = $(TOP)/lib/liblinker.a  $(TOP)/lib/libobj.a $(TOP)/lib/libutil.a

#################VERSION DIR#####################


#################EXTRA CLEAN#####################


export PROG_LINKER_LDFLAGS 

.PHONY: all print_platform $(SRC_DIRS) clean

all: print_platform $(SRC_DIRS) $(PROGS)

print_platform:
	@echo "###################################"
	@echo "             $(PLATFORM)"
	@echo "###################################"
	
	
$(SRC_DIRS):
	@mkdir -p $(TOP)/bin
	@mkdir -p $(TOP)/lib
	$(Q) $(MAKE) $(MAKE_DEBUG) --directory=$@

$(TOP)/bin/linker: $(LINKER_DIRS)
	$(Q) $(MAKE) $(MAKE_DEBUG) -f $(TOP)/mk/prog.mk linker

clean:
	$(Q) for d in $(SRC_DIRS); \
	do \
	$(MAKE) clean --directory=$$d; \
	done \
	
	$(Q) -rm -f $(PROGS)
	$(Q) -rm -rf $(TOP)/bin  $(TOP)/lib $(TOP)/build


