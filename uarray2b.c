/**************************************************************
 *
 *                     uarray2b.c
 *
 *     Assignment: locality
 *     Authors:  Sam Hecht (shecht02) and Auriel Wish (awish01)
 *     Date:     2/23/2023
 *
 *     Purpose:  implementation for a UArray2b which
 *               is a blocked implementation of a 2D array
 *               (i.e., each block is stored contiguously in memory)
 *
 **************************************************************/
#include <stdlib.h>
#include <math.h>
#include "uarray2b.h"
#include "uarray2.h"
#include "uarray.h"
#include "assert.h"
#include "mem.h"

#define BLOCK_64K (64 * 1024)
#define T UArray2b_T

struct T {
        UArray2_T blocks;
        int width;
        int height;
        int blocksize;
        int size;
};

/* 
 * Closure used to initialize each UArray_T within the UArray2_T with the
 * correct length (blocksize * blocksize) and size (size)
 */
struct Closure {
        int blocksize;
        int size;
};

typedef struct Closure Closure;

int calc_width_or_height(int dist, int blocksize);
void create_UArray_apply(int col, int row, UArray2_T uarray2, void *entry,
                                                                void *cl);
void free_UArray_apply(int col, int row, UArray2_T uarray2, void *entry, 
                                                                void *cl);
void traverse_block(int block_col, int block_row, T array2b, 
       void apply(int col, int row, T array2b, void *elem, void *cl), void *cl);

/*
 * Name: UArray2b_new
 * Purpose: construct a new UArray2b
 * Parameters: desired width, height, and blocksize for the new UArray2b, and
 *             the size of the elements it will contain
 * Returns: a UArray2b
 * Notes: the width, height, size, and blocksize must all be postive
 */
extern T UArray2b_new(int width, int height, int size, int blocksize) {
        assert(width > 0 && height > 0 && size > 0 && blocksize > 0);
        /* Make and set uarray2b variable */
        T uarray2b;
        NEW(uarray2b);
        uarray2b->width = width;
        uarray2b->height = height;
        uarray2b->blocksize = blocksize;
        uarray2b->size = size;

        /* calculate width and height of uarray2 and make uarray2 */
        int uarray2Width = calc_width_or_height(width, blocksize);
        int uarray2Height = calc_width_or_height(height, blocksize);
        UArray2_T uarray2 = UArray2_new(uarray2Width, uarray2Height,
                                                        sizeof(UArray_T));
        Closure cl = {blocksize, size};

        /* fill the uarray2 */
        UArray2_map_row_major(uarray2, create_UArray_apply, &cl);
        uarray2b->blocks = uarray2;

        return uarray2b;
}

/*
 * Name: calc_width_or_height
 * Purpose: calculate the width/height for the UArray2
 * Parameters: a dimension of the UArray2b (height or width), blocksize of 
 *             UArray2b
 * Returns: desired dimension for the UArray2
 * Notes: this is basically a ceiling function
 */
int calc_width_or_height(int dim, int blocksize) {
        /* custom ceiling function */
        int uarray2dim = dim / blocksize;
        if ((dim % blocksize) != 0) {
                uarray2dim++;
        }
        return uarray2dim;
}

/*
 * Name: create_UArray_apply
 * Purpose: helper function for making blocks out of UArrays
 * Parameters: a void pointer that will point to the current block (that is
 *             being made), a void pointer that will point to a struct
 *             containing the necessary size information to make the new block
 *             (UArray), all other parameters are ignored
 * Returns: nothing
 * Notes: entry must not be NULL
 */
void create_UArray_apply(int col, int row, UArray2_T uarray2, void *entry,
                                                                void *cl) {
        assert(entry != NULL);
        /* make new uarray and place it in the current uarray2 slot */
        Closure *sizes = cl;
        UArray_T *curr_uarray = entry;
        *curr_uarray = UArray_new(sizes->blocksize * sizes->blocksize,
                                                                sizes->size);
        (void) col;
        (void) row;
        (void) uarray2;
}

/*
 * Name: free_UArray_apply
 * Purpose: helper function to free what's in the blocks (the UArrays)
 * Parameters: a void pointer that will point to the current block (UArray),
 *             all other parameters are ignored
 * Returns: nothing
 * Notes: entry must not be NULL
 */
void free_UArray_apply(int col, int row, UArray2_T uarray2, void *entry,
                                                                void *cl) {
        assert(entry != NULL);
        /* free the uarray in the current uarray2 slot */
        UArray_T *curr_uarray = entry;
        UArray_free(curr_uarray);
        (void) col;
        (void) row;
        (void) uarray2;
        (void) cl;
}

/*
 * Name: UArray2b_new_64K_block
 * Purpose: make a new UArray2b whose blocks can hold 64 kilobytes
 * Parameters: the desired width and height of the new UArray2b, the size of the
 *             elements
 * Returns: a UArray2b
 * Notes: the width, height, and size must be postive
 */
extern T UArray2b_new_64K_block(int width, int height, int size) {

        assert(width > 0);
        assert(height > 0);
        assert(size > 0);
        
        int blocksize = sqrt(BLOCK_64K / size);

        /* if less than 4 elements can fit in a block, block holds 1 element */
        if (blocksize == 0) {
                blocksize = 1;
        }
        return UArray2b_new(width, height, size, blocksize);
}

/*
 * Name: UArray2b_free
 * Purpose: free data from UArray2b
 * Parameters: the UArray2b to free
 * Returns: nothing
 * Notes: array2b and *array2b must not be NULL
 */
extern void UArray2b_free(T *array2b) {
        assert(array2b != NULL);
        assert((*array2b) != NULL);
        UArray2_map_row_major((*array2b)->blocks, free_UArray_apply, NULL);
        UArray2_free(&((*array2b)->blocks));
        FREE(*array2b);
}

/*
 * Name: UArray2b_width
 * Purpose: get width of UArray2b
 * Parameters: the UArray2b to check
 * Returns: the width of the UArray2b
 * Notes: array2b must not be NULL
 */
extern int UArray2b_width(T array2b) {
        assert(array2b != NULL);
        return array2b->width;
}

/*
 * Name: UArray2b_height
 * Purpose: get height of UArray2b
 * Parameters: the UArray2b to check
 * Returns: the height of the UArray2b
 * Notes: array2b must not be NULL
 */
extern int UArray2b_height(T array2b) {
        assert(array2b != NULL);
        return array2b->height;
}

/*
 * Name: UArray2b_size
 * Purpose: get size of elements UArray2b
 * Parameters: the UArray2b to check
 * Returns: the size of the elements in the UArray2b
 * Notes: array2b must not be NULL
 */
extern int UArray2b_size(T array2b) {
        assert(array2b != NULL);
        return array2b->size;
}

/*
 * Name: UArray2b_blocksize
 * Purpose: get blocksize of UArray2b
 * Parameters: the UArray2b to check
 * Returns: the blocksize of the UArray2b
 * Notes: array2b must not be NULL
 */
extern int UArray2b_blocksize(T array2b) {
        assert(array2b != NULL);
        return array2b->blocksize;
}

/*
 * Name: UArray2b_at
 * Purpose: get the element at a specified index in a UArray2b
 * Parameters: UArray2b to be traversed, column and row of desired element
 * Returns: void pointer to the element at the specified index
 * Notes: array2b must not be NULL, column and row must be within bounds
 */
extern void *UArray2b_at(T array2b, int column, int row) {
        assert(array2b != NULL);
        assert(column < array2b->width && column >= 0);
        assert(row < array2b->height && row >= 0);
        /* calculate which block the index is in */
        int block_row = row / array2b->blocksize;
        int block_col = column / array2b->blocksize;

        /* calculate where the index is within the block */
        int uarray_row = row % array2b->blocksize;
        int uarray_col = column % array2b->blocksize;
        UArray_T *uarray = UArray2_at(array2b->blocks, block_col, block_row);
        return UArray_at(*uarray, uarray_col + uarray_row * array2b->blocksize);
}

/*
 * Name: UArray2b_map
 * Purpose: perform a function onto every element of a UArray2b in block
 *          major order
 * Parameters: UArray2b to be traversed, the function to be applied, the closure
 *             variable
 * Returns: nothing
 * Notes: array2b must not be NULL
 */
extern void UArray2b_map(T array2b, void apply(int col, int row, T array2b,
                                        void *elem, void *cl), void *cl) {
        assert(array2b != NULL);
        UArray2_T blocks = array2b->blocks;
        /* first 2 loops - go block by block */
        for (int row = 0; row < UArray2_height(blocks); row++) {
                for (int col = 0; col < UArray2_width(blocks); col++) {
                        /* Traverse each individual block in array2b */
                        traverse_block(col, row, array2b, apply, cl);
                }
        }
}

/*
 * Name: traverse_block
 * Purpose: go through each element in the current uarray and call the apply
 *          function on them
 * Parameters: column and row of the current block, the UArray2b to traverse, 
 *             the apply function, and the closure variable
 * Returns: nothing, but applies the user defined apply function to each element
 *          within a block and traverses the entirety of the block
 * Notes: some blocks may have indices which are undefined but we avoid calling
 *        apply on these garbage value slots
 */
void traverse_block(int block_col, int block_row, T array2b, 
void apply(int col, int row, T array2b, void *elem, void *cl), void *cl) {
        int blocksize = UArray2b_blocksize(array2b);
        UArray_T *curr_uarray = UArray2_at(array2b->blocks, block_col,
                                                                block_row);
        /* go through current block element by element and call apply func */
        for (int i = 0; i < UArray_length(*curr_uarray); i++) {
                /* Row and col of element in within a block in array2b */
                int col = block_col * blocksize + i % blocksize;
                int row = block_row * blocksize + i / blocksize;
                
                /* Make sure we're accessing a valid index of the array2b */
                if (row < array2b->height && col < array2b->width) {
                        apply(col, row, array2b, 
                                UArray_at(*curr_uarray, i), cl);
                }    
        }
}

#undef T
#undef BLOCK_64K