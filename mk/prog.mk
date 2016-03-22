include $(PROJ_DIR)/mk/env.mk
TOP = ..


PROG_LINKER_LDFLAGS += 

x86_64_linker: FORCE
	@echo [LD] $@
	$(Q) $(CC) -o $(PROJ_DIR)/bin/$(ARCH)/$@ $(PROG_LINKER_LDFLAGS) 
	
mips32_linker: FORCE
	@echo [LD] $@
	$(Q) $(CC) -o $(PROJ_DIR)/bin/$(ARCH)/$@ $(PROG_LINKER_LDFLAGS) 

.PHONY:x86_64_linker FORCE