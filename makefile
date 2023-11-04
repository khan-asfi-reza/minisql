ifeq ($(OS),Windows_NT)
    detected_OS := Windows
    MKDIR_P = if not exist "$(subst /,\,$(1))" mkdir "$(subst /,\,$(1))"
    RM = del /Q /F
    FixPath = $(subst /,\,$1)
    EXEC_EXT = .exe
else
    detected_OS := $(shell uname -s)
    MKDIR_P = mkdir -p $(1)
    RM = rm -f
    FixPath = $1
    EXEC_EXT =
endif

CC = gcc
TARGET = $(call FixPath,build/minisql$(EXEC_EXT))
SRCDIR = src
BUILDDIR = build
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

all: $(BUILDDIR) $(TARGET)

$(BUILDDIR):
	@$(call MKDIR_P,$(BUILDDIR))

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@$(call MKDIR_P,$(dir $@))
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(call FixPath,$(OBJS) $(TARGET))
	-@$(RM) -r $(call FixPath,$(BUILDDIR)/*)

run: $(TARGET)
	@echo Running $(TARGET)
	@$(TARGET)

.PHONY: all clean run $(BUILDDIR)
