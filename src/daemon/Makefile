CC := g++
TARGET := $(TARGET_DIR)/cwoj_daemon
SRCDIR := .
SRCEXT := cpp
INCEXT := h
SOURCES := $(wildcard $(SRCDIR)/*.$(SRCEXT))
INCLUDES := $(wildcard $(SRCDIR)/*.$(INCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILD_DIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS := -g -std=c++11
LIB := -pthread -lmicrohttpd -lmysqlclient -lpthread
INC := -I /usr/include/mysql

$(TARGET): $(OBJECTS)
	@echo "Linking..." 
	@echo "$(CC) $(CFLAGS) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB)

$(BUILD_DIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILD_DIR)
	@echo "$(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<