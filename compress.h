/**************************************************************
 *
 *                     compress.h
 *
 *     Assignment: arith
 *     Authors:  Adam Weiss and Auriel Wish
 *     Date:     3/7/2023
 *
 *     Purpose:  Interface for compression implementation.
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
#include "seq.h"
#include <math.h>
#include <stdbool.h>
#include <assert.h>


#ifndef COMPRESS_INCLUDED
#define COMPRESS_INCLUDED

Pnm_ppm ppm_to_rgb_int(FILE *inputfp);
UArray2_T rgb_int_to_rgb_float(Pnm_ppm original);
UArray2b_T rgb_float_to_component_video(UArray2_T rgb_float_array);
UArray2_T comp_video_floats_to_comp_avg_float(UArray2b_T comp_video_array);
UArray2_T comp_avg_floats_to_comp_avg_ints(UArray2_T comp_avg_array);
void comp_avg_ints_to_out(UArray2_T comp_avg_ints_array);

#endif