include $(PROJ_DIR)/mk/env.mk
TOP = ..


PROG_LINKER_LDFLAGS += 

linker: FORCE
	@echo [LD] $@
	$(Q) $(CC) -o $(PROJ_DIR)/bin/$@ $(PROG_LINKER_LDFLAGS) 

	

.PHONY:linker FORCE