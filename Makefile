ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
BUILD_DIR := $(ROOT_DIR)/build
TARGET_DIR := $(ROOT_DIR)/bin

export ROOT_DIR
export BUILD_DIR
export TARGET_DIR

daemon: 
	$(MAKE) -C src/daemon
    
all: daemon

clean: 
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILD_DIR) $(TARGET_DIR)"; $(RM) -r $(BUILD_DIR) $(TARGET_DIR)

.PHONY: clean
