/**************************************************************
 *
 *                     a2plain.c
 *
 *     Assignment: locality
 *     Authors:  Sam Hecht (shecht02) and Auriel Wish (awish01)
 *     Date:     2/23/2023
 *
 *     Purpose:  implementation for an A2Methods_UArray2 which
 *               can be implemented using a UArray2 or a UArray2b.
 *               This file allows for polymorphism between the 2
 *               implementations of 2D arrays by creating and
 *               exporting an A2_Methods_T
 *
 **************************************************************/


#include <string.h>

#include <a2plain.h>
#include "uarray2.h"

/************************************************/
/* Define a private version of each function in */
/* A2Methods_T that we implement.               */
/************************************************/
typedef A2Methods_UArray2 A2;

/*
 * Name: new
 * Purpose: Constructs and returns a new UArray2 as an A2
 * Parameters: an integer representing the width, an integer representing the
 *             height, and an integer representing the size of each element
 *             within the A2/UArray2
 * Returns: A UArray2 which should be interpreted by the client as an A2
 * Notes: None
 */
static A2 new(int width, int height, int size)
{
        return UArray2_new(width, height, size);
}

/*
 * Name: new_with_blocksize
 * Purpose: Constructs a new UArray2
 * Parameters: an integer representing the width, an integer representing the
 *             height, and an integer representing the size of each element
 *             within the A2/UArray2, an integer representing the blocksize
 * Returns: A UArray2 which should be interpreted by the client as an A2
 * Notes: blocksize is not used because UArray2s do not support blocksize
 */
static A2 new_with_blocksize(int width, int height, int size,
                                            int blocksize)
{
        (void) blocksize;
        return UArray2_new(width, height, size);
}

/*
 * Name: a2free
 * Purpose: free the memory allocated for the A2
 * Parameters: a pointer to the A2 who's memory needs to be freed
 * Returns: nothing
 * Notes: nothing
 */
static void a2free(A2 * array2p)
{
        UArray2_free((UArray2_T *) array2p);
}

/*
 * Name: width
 * Purpose: get the width of the A2
 * Parameters: the A2 who's width will be found
 * Returns: the width of the A2
 * Notes: none
 */
static int width(A2 array2)
{
        return UArray2_width(array2);
}

/*
 * Name: height
 * Purpose: get the height of the A2
 * Parameters: the A2 who's height will be found
 * Returns: the height of the A2
 * Notes: none
 */
static int height(A2 array2)
{
        return UArray2_height(array2);
}

/*
 * Name: size
 * Purpose: get the size of the elements in the A2
 * Parameters: the A2 who's elements' size will be found
 * Returns: the size of the elements in the A2
 * Notes: none
 */
static int size(A2 array2)
{
        return UArray2_size(array2);
}

/*
 * Name: blocksize
 * Purpose: get the blocksize of the A2
 * Parameters: the A2 who's height will be found
 * Returns: the height of the A2
 * Notes: none
 */
static int blocksize(A2 array2)
{
        (void) array2;
        return 1;
}

/*
 * Name: at
 * Purpose: returns an A2Methods_Object ptr to the element at position
 *              (col, row) in the A2 array2
 * Parameters: the A2 array2 to return the element at pos (col, row) of, and
 *             an integer col representing the column and an integer row
 *             representing the row
 * Returns: a ptr to an A2Methods_Object which is a ptr to an unknown sequence
 *          of bytes in memory 
 * Notes: none
 */
static A2Methods_Object *at(A2 array2, int col, int row)
{
        return UArray2_at(array2, col, row);
}

// typedef void UArray2_applyfun(int i, int j, UArray2_T array2b, void *elem, 
// void *cl);

/*
 * Name: map_row_major
 * Purpose: go through the A2 in row major order and run apply function at each
 *          index
 * Parameters: A2 to be traversed, apply function, pointer to closure variable
 * Returns: nothing
 * Notes: nothing
 */
static void map_row_major(A2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_row_major(uarray2, (UArray2_applyfun*)apply, cl);
}

/*
 * Name: map_col_major
 * Purpose: go through the A2 in col major order and run apply function at each
 *          index
 * Parameters: A2 to be traversed, apply function, void pointer to closure
 *             variable
 * Returns: nothing
 * Notes: nothing
 */
static void map_col_major(A2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_col_major(uarray2, (UArray2_applyfun*)apply, cl);
}

struct small_closure {
        A2Methods_smallapplyfun *apply; 
        void                    *cl;
};

/*
 * Name: apply_small
 * Purpose: act as apply function but only the closure variable and current
 *          A2 element matter
 * Parameters: column and row of current index, the UArray2 in use, a void
 *             pointer to the element at the current index, void pointer to
 *             closure variable
 * Returns: nothing
 * Notes: column, row, and UArray2 in use are unused
 */
static void apply_small(int i, int j, UArray2_T uarray2,
                        void *elem, void *vcl)
{
        struct small_closure *cl = vcl;
        (void)i;
        (void)j;
        (void)uarray2;
        cl->apply(elem, cl->cl);
}

/*
 * Name: small_map_row_major
 * Purpose: traverse through A2 in row major order, performing small apply
 *          function at each index
 * Parameters: A2 to be traversed, apply function, void pointer to closure
 *             variable
 * Returns: nothing
 * Notes: nothing
 */
static void small_map_row_major(A2 a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_row_major(a2, apply_small, &mycl);
}

/*
 * Name: small_map_col_major
 * Purpose: traverse through A2 in col major order, performing small apply
 *          function at each index
 * Parameters: A2 to be traversed, apply function, void pointer to closure
 *             variable
 * Returns: nothing
 * Notes: nothing
 */
static void small_map_col_major(A2 a2,
                                A2Methods_smallapplyfun apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_col_major(a2, apply_small, &mycl);
}


static struct A2Methods_T uarray2_methods_plain_struct = {
        new,
        new_with_blocksize,
        a2free,
        width,
        height,
        size,
        blocksize,
        at,
        map_row_major,        // map_row_major
        map_col_major,        // map_col_major
        NULL,                 // block_major (NULL)
        map_row_major,        // map_default
        small_map_row_major,  // small_map_row_major
        small_map_col_major,  // small_map_col_major
        NULL,                 // small_block_major (NULL)
        small_map_row_major,  // small_map_default
};

// finally the payoff: here is the exported pointer to the struct

A2Methods_T uarray2_methods_plain = &uarray2_methods_plain_struct;
