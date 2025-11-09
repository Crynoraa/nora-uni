CC = gcc
CFLAGS = -Wall -Werror -Wpedantic
RM = rm -f
EXE = time_sys_stdio
OBJECTS = $(SOURCE:%.c=%.o) #scans the directory for any .o files created, in accordance to the amount of .c files present
SOURCE :=  $(shell find . -name '*.c') #similar to the above, but scans it for .c files
TXT = file.txt
.PHONY: all
all:  $(EXE) init

$(EXE):$(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(EXE)
#Creates the (singular) exe, using a given list of objects (standard using all .o files).
%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
#Creates and compiles the .o file for each .c file. this uses the names of the files themselves ($< for input-file and $@ for output-file).

.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(EXE) $(TXT)
#removes all .o, .exe, and .txt files. ignores nonexistent/missing files due to -f.
.PHONY: init
init:
	dd if=/dev/zero of=file.txt bs=1K count=50000
#initialises the txt file to be read by our program. 50 MB
