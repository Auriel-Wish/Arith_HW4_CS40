/**************************************************************
 *
 *                     decompress.h
 *
 *     Assignment: arith
 *     Authors:  Adam Weiss and Auriel Wish
 *     Date:     3/7/2023
 *
 *     Purpose:  Interface for decompression implementation.
 *               Includes only modules necessary for the client
 *               (no helper functions)
 *
 **************************************************************/

#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"
#include "uarray2.h"
#include "uarray2b.h"
#include "bitpack.h"
#include "arith40.h"
#include "pixel_structs.h"
#include "mem.h"
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#ifndef DECOMPRESS_INCLUDED
#define DECOMPRESS_INCLUDED

#define A2 A2Methods_UArray2

void rgb_int_to_ppm(Pnm_ppm output_image);
Pnm_ppm rgb_float_to_rgb_int(UArray2_T rgb_float_array);
UArray2_T component_video_to_rgb_float(UArray2b_T comp_video_array);
UArray2b_T comp_avg_float_to_comp_video_floats(UArray2_T comp_avg_float_arr);
UArray2_T comp_avg_ints_to_comp_avg_floats(UArray2_T comp_avg_int_arr);
UArray2_T word_to_comp_avg_ints(FILE *input);

#undef A2

#endif