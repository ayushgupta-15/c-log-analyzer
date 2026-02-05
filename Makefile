CC       = gcc
TARGET   = loganalyzer

SRCDIR   = src
OBJDIR   = obj
INCDIR   = include
LOGDIR   = logs

CFLAGS   = -Wall -Wextra -Wpedantic -std=c99 -O2 -I$(INCDIR)
LDFLAGS  =
DEPFLAGS = -MMD -MP

SOURCES  = $(wildcard $(SRCDIR)/*.c)
OBJECTS  = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
DEPS     = $(OBJECTS:.o=.d)

# ---------- Default Target ----------
all: $(TARGET)

# ---------- Build Target ----------
$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

# ---------- Object Compilation ----------
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

# ---------- Directory Targets ----------
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(LOGDIR):
	mkdir -p $(LOGDIR)

# ---------- Run ----------
run: $(TARGET) | $(LOGDIR)
	./$(TARGET) $(LOGDIR)/sample.log

# ---------- Debug Build ----------
debug: CFLAGS = -Wall -Wextra -Wpedantic -std=c99 -g -O0 -fsanitize=address -I$(INCDIR)
debug: LDFLAGS = -fsanitize=address
debug: clean all

# ---------- Valgrind ----------
valgrind: $(TARGET) | $(LOGDIR)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET) $(LOGDIR)/sample.log

# ---------- Clean ----------
clean:
	rm -rf $(OBJDIR) $(TARGET)

# ---------- Dependency Includes ----------
-include $(DEPS)

.PHONY: all clean run debug valgrind
