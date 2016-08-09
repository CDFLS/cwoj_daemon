#
# TODO: Move `libmongoclient.a` to /usr/local/lib so this can work on production servers
#
 
CC := g++ # This is the main compiler
# CC := clang --analyze # and comment out the linker last line for sanity
TARGET := $(TARGET_DIR)/cwoj_runner
SRCDIR := .
SRCEXT := cpp
INCEXT := h
SOURCES := $(wildcard $(SRCDIR)/*.$(SRCEXT))
INCLUDES := $(wildcard $(SRCDIR)/*.$(INCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILD_DIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS := -g -std=c++11 # -Wall
LIB := -pthread -lmicrohttpd -lmysqlclient -lpthread
INC := -I /usr/include/mysql

$(TARGET): $(OBJECTS)
	@echo "Linking..." 
	@echo "$(CC) $(CFLAGS) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB)

$(BUILD_DIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILD_DIR)
	@echo "$(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<