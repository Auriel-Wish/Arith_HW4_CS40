# Makefile for Arith (Comp 40 Assignment 4)
# 
# Used by: Adam Weiss (aweiss15) and Auriel Wish (awish01)
# Date created: 3/7/23


############## Variables ###############

CC = gcc # The compiler being used

# Updating include path to use Comp 40 .h files and CII interfaces
IFLAGS = -I/comp/40/build/include -I/usr/sup/cii40/include/cii

# Compile flags
CFLAGS = -g -std=gnu99 -Wall -Wextra -Werror -Wfatal-errors -pedantic $(IFLAGS)

# Linking flags
LDFLAGS = -g -L/comp/40/build/lib -L/usr/sup/cii40/lib64

# Libraries needed for linking
LDLIBS = -lcii40 -l40locality -larith40 -lnetpbm -lm -lrt

# Collect all .h files in your directory.
INCLUDES = $(shell echo *.h)

############### Rules ###############

all: ppmdiff 40image-6 bitpack_test


## Compile step (.c files -> .o files)

# To get *any* .o file, compile its .c file with the following rule.
%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -c $< -o $@


## Linking step (.o -> executable program)

ppmdiff: ppmdiff.o uarray2.o uarray2b.o a2plain.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

40image-6: 40image.o compress.o decompress.o check_bounds.o bitpack.o uarray2.o uarray2b.o a2plain.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

bitpack_test: bitpack.o bitpack_test.o
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)



clean:
	rm -f ppmdiff *.o 40image

