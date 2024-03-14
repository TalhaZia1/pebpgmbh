# Define the compiler and compiler flags
CC = gcc
CFLAGS = -Wall -O2 -Wunused-variable

# Define the source file and the output executable name
SOURCE = task_scheduler.c
OUTPUT = app.exe

# The default target is to build and run the program
all: $(OUTPUT)

# Build the executable
$(OUTPUT): $(SOURCE)
	$(CC) $(CFLAGS) -o $(OUTPUT) $(SOURCE)
	./$(OUTPUT)

# Run the executable
run: all
	./$(OUTPUT)

# Clean up the generated files
clean:
	rm -f $(OUTPUT)

# PHONY targets don't represent files
.PHONY: all run clean
