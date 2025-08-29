ARCH ?= aarch64
CROSS_COMPILE ?= aarch64-linux-gnu-

# Set the output directory for compiled binaries
OUTPUT_DIR = bin

# Set the C compiler and flags for static compilation
CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -Wextra -I.
LDFLAGS = -static

# --- 2. SOURCE FILES ---
# Define the source files and their corresponding output binaries.
# The 'linuxrc' and 'sh' binaries are named separately for clarity.
BINS = echo ls mini_sh
SRC = $(BINS:%=%.c)
SRC += linuxrc.c
OBJS = $(SRC:.c=.o)
TARGETS = $(BINS) linuxrc

# --- 3. BUILD RULES ---
.PHONY: all clean install

all: $(TARGETS)

# Rule to compile a C file into an object file
%.o: %.c
	@echo "  CC      $@"
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to link object files into executable binaries
# Note: Use LDFLAGS for static linking
$(BINS): %: %.o
	@echo "  LD      $@"
	$(CC) $(LDFLAGS) $< -o $@

# Special rule for the linuxrc binary
linuxrc: linuxrc.o
	@echo "  LD      $@"
	$(CC) $(LDFLAGS) linuxrc.o -o $@

# --- 4. INSTALLATION ---
# Creates a minimal filesystem tree and copies binaries into it
install: all
	@echo "  INSTALL $(OUTPUT_DIR)"
	mkdir -p $(OUTPUT_DIR)
	# Copy all compiled binaries to the output directory
	cp $(TARGETS) $(OUTPUT_DIR)
	# Create a symbolic link for 'sh' to 'mini_sh'
	ln -sf mini_sh $(OUTPUT_DIR)/sh
	# Create a symbolic link for 'init' to 'linuxrc' as per convention
	ln -sf linuxrc $(OUTPUT_DIR)/init

# --- 5. CLEANING ---
# Removes all compiled files and the output directory
clean:
	@echo "  CLEAN"
	rm -rf $(OUTPUT_DIR) *.o $(TARGETS)