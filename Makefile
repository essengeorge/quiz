SRCDIR   = src
INCDIR   = include
OBJDIR   = obj
CC       = clang
CFLAGS   = -g -std=c89 -pedantic -Wall -Wextra -Werror -I$(INCDIR)
TARGET   = quizengine
SOURCES  := $(wildcard $(SRCDIR)/*.c)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
all: $(TARGET)
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(OBJDIR):
	mkdir -p $(OBJDIR)
clean:
	rm -rf $(OBJDIR) $(TARGET)
.PHONY: all clean
