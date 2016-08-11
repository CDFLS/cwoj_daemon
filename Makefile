all: daemon runner

ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
BUILD_DIR := $(ROOT_DIR)/build
TARGET_DIR := $(ROOT_DIR)/bin

create_dir: 
	@mkdir -p $(BUILD_DIR) 
	@mkdir -p $(TARGET_DIR)

export ROOT_DIR
export BUILD_DIR
export TARGET_DIR

daemon: create_dir
	$(MAKE) -C src/daemon

runner: create_dir
	$(MAKE) -C src/runner

clean: 
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILD_DIR) $(TARGET_DIR)"; $(RM) -r $(BUILD_DIR) $(TARGET_DIR)

.PHONY: clean create_dir
