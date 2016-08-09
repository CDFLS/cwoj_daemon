CC := gcc
TARGET := $(TARGET_DIR)/cwoj_runner
SRCDIR := .
SRCEXT := c
INCEXT := h
SOURCES := $(wildcard $(SRCDIR)/*.$(SRCEXT))
INCLUDES := $(wildcard $(SRCDIR)/*.$(INCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILD_DIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS := -g
LIB := 
INC := 

$(TARGET): $(OBJECTS)
	@echo "Linking..." 
	@echo "$(CC) $(CFLAGS) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB)

$(BUILD_DIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@echo "$(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<
