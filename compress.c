/**************************************************************
 *
 *                     compress.c
 *
 *     Assignment: arith
 *     Authors:  Adam Weiss and Auriel Wish
 *     Date:     3/7/2023
 *
 *     Purpose:  Provide functions to compress an image by
 *               bitpacking it and outputing the result.
 *
 **************************************************************/

#include "compress.h"

/* Struct definitions */

/*
 * Name: rgb_ints_closure
 * Contains: necessary information to pass into mapping function when converting
 *           from rgb ints to rgb floats - UArray2 of rgb ints, image
 *           denominator, image width and height
 */
struct rgb_ints_closure {
        UArray2_T array;
        float den;
        int width;
        int height;
};
typedef struct rgb_ints_closure rgb_ints_closure;

/*
 * Name: comp_floats_closure
 * Contains: necessary information to pass into mapping function when performing
 *           pixel averaging - UArray2 of averaged component video floats,
 *           sequence to help with calculations
 */
struct comp_floats_closure {
        UArray2_T comp_avg_float_arr;
        Seq_T seq_avg;
};
typedef struct comp_floats_closure comp_floats_closure;

#define A2 A2Methods_UArray2
#define BLOCKSIZE 2

#define SEQ_SIZE 4
/* Bit packing literals */
#define WIDTH_A 6
#define WIDTH_B_C_D 6
#define WIDTH_Pb_Pr 4
#define LSB_A 26
#define LSB_B 20
#define LSB_C 14
#define LSB_D 8
#define LSB_Pb 4
#define LSB_Pr 0

/* Helper functions */
float calculate_comp_video_nums(rgb_floats *curr_float_pixel, float red_num,
                                        float green_num, float blue_num);
void rgb_int_to_rgb_float_apply(int col, int row, A2 pixmap, void *entry,
                                                                void *cl);
void rgb_float_to_component_video_apply(int col, int row, UArray2_T pixmap,
                                                        void *entry, void *cl);
void comp_video_floats_to_comp_avg_float_apply(int col, int row,
                                UArray2b_T pixmap, void *entry, void *cl);
void clear_seq(Seq_T seq);
void comp_avg_floats_to_comp_avg_ints_apply(int col, int row, UArray2_T pixmap,
                                                        void *entry, void *cl);
void comp_avg_ints_to_out_apply(int col, int row, UArray2_T pixmap, void *entry,
                                                                void *cl);
float ensure_in_bounds(float val, float min, float max);

/*
 * Name: ppm_to_rgb_int
 * Purpose: Convert a ppm file to a pnm_ppm struct
 * Parameters: A ppm image file
 * Returns: A Pnm_ppm image struct
 * Notes: The parameter file pointer must not be null
 */
Pnm_ppm ppm_to_rgb_int(FILE *inputfp)
{
        assert(inputfp != NULL);

        /* create a methods suite instance */
        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods); 

        /* read the image file into a pnm_ppm*/
        Pnm_ppm input_image = Pnm_ppmread(inputfp, methods);
        return input_image;
}

/*
 * Name: rgb_int_to_rgb_float
 * Purpose: Convert the scaled rgb int values to floats by dividing by the 
 *          denominator.
 * Parameters: The pnm_ppm image struct containing the rbg int image data
 * Returns: A UArray2 where each slot represents a pixel with floated rbg values
 * Notes: original must not be NULL, frees original
 */
UArray2_T rgb_int_to_rgb_float(Pnm_ppm original)
{
        assert(original != NULL);

        /* Trim the image to even dimentions */
        original->width = original->width - original->width % 2;
        original->height = original->height - original->height % 2;

        /* Create a 2D array to hold the floated rgb pixel values*/
        UArray2_T rgb_float_array = UArray2_new(original->width,
                                        original->height, sizeof(rgb_floats));
        float den = original->denominator;

        /* Convert all the scaled rgb ints to floats and store them */
        rgb_ints_closure cl = {.array = rgb_float_array, .den = den,
                        .width = original->width, .height = original->height};
        original->methods->map_default(original->pixels,
                                        rgb_int_to_rgb_float_apply, &cl);

        Pnm_ppmfree(&original);

        /* return the array of rgb floats */
        return rgb_float_array;
}

/*
 * Name: rgb_int_to_rgb_float_apply
 * Purpose: convert the given rgb integer pixel to an rgb float pixel
 * Parameters: column and row of the current pixel, the pixmap itself (which is
 *             unused), a void pointer to the current pixel, and void pointer to
 *             the closure variable 
 * Returns: none
 * Notes: since the mapping function maps through the original pixmap, it will
 *        include all rows and columns. However, we ensure that it only converts
 *        the pixels that are at valid indexes for the rgb float array (the
 *        width and height will always be even, and will be one less than the
 *        rgb int array if its width and/or height are odd).
 */
void rgb_int_to_rgb_float_apply(int col, int row, A2 pixmap, void *entry,
                                                                void *cl)
{
        /* get values from void pointers */
        rgb_ints_closure *closure = cl;
        UArray2_T rgb_float_array = closure->array;
        float den = closure->den;

        /* 
         * only perform operation if index is within bounds of rgb_float_array
         * (there may be one less column and/or row)
         */
        if (col < closure->width && row < closure->height) {
                rgb_floats *curr_float_pixel = UArray2_at(rgb_float_array, col,
                                                                        row);
                Pnm_rgb curr_int_pixel = entry;
                /*
                 * integer value converted to floar by dividing by denominator
                 * value
                 */
                curr_float_pixel->red = ((float) (curr_int_pixel->red))
                                                                        / den;
                curr_float_pixel->green = ((float) (curr_int_pixel->green))
                                                                        / den;
                curr_float_pixel->blue = ((float) (curr_int_pixel->blue))
                                                                        / den;
        }

        (void) pixmap;
}

/*
 * Name: rgb_float_to_component_video
 * Purpose: Convert the floated rgb values to component video space
 * Parameters: The 2D array pixelmap of floated rgb values
 * Returns: A UArray2b where each slot represents a pixel in 
 *          component video space
 * Notes: The parameter rgb_float_array must not be NULL, frees rgb_float_array.
 *        Also, the component video space array is a UArray2b because it will be
 *        traversed in block major order.
 */
UArray2b_T rgb_float_to_component_video(UArray2_T rgb_float_array)
{
        /*
         * make new array and traverse through the inputted one, changing the
         * pixels in the new one based on the pixels in the inputted one
         */
        assert(rgb_float_array != NULL);
        UArray2b_T comp_video_array = UArray2b_new(
                UArray2_width(rgb_float_array),
                UArray2_height(rgb_float_array),
                sizeof(comp_video_floats), BLOCKSIZE);
        UArray2_map_row_major(rgb_float_array,
                        rgb_float_to_component_video_apply, comp_video_array);
        UArray2_free(&rgb_float_array);
        return comp_video_array;
}

/*
 * Name: rgb_float_to_component_video_apply
 * Purpose: convert the given rgb float pixel to a component video pixel
 * Parameters: column and row of the current pixel, the pixmap itself (which is
 *             unused), a void pointer to the current pixel, and void pointer to
 *             the closure variable 
 * Returns: none
 * Notes: none
 */
void rgb_float_to_component_video_apply(int col, int row, UArray2_T pixmap,
                                                        void *entry, void *cl)
{
        /* get values from void pointers */
        UArray2b_T comp_video_array = cl;
        rgb_floats *curr_float_pixel = entry;

        /* 
         * calculate Y, Pb, and Pr based on curr_float_pixel. Y value must be
         * between 0 and 1
         */
        comp_video_floats *curr_video_pixel = UArray2b_at(comp_video_array, col,
                                                                        row);
        curr_video_pixel->luma = ensure_in_bounds(
                calculate_comp_video_nums(curr_float_pixel, 
                                                0.299, 0.587, 0.114), 0, 1);
        curr_video_pixel->reddiff = calculate_comp_video_nums(
                                curr_float_pixel, 0.5, -0.418688, -0.081312);
        curr_video_pixel->bluediff = calculate_comp_video_nums(
                                curr_float_pixel, -0.168736, -0.331264, 0.5);

        (void) pixmap;
}

/*
 * Names: calculate_comp_video_nums
 * Purpose: perform the calculations provided in the spec
 * Parameters: a pointer to the current pixel, the constants that will be used
 *             for the calculation
 * Returns: the calculated float (either for luminance, blue difference, or red
 *          difference)
 * Notes: none
 */
float calculate_comp_video_nums(rgb_floats *curr_float_pixel, float red_num,
                                                float green_num, float blue_num)
{
        return (red_num * curr_float_pixel->red) +
                (green_num * curr_float_pixel->green) +
                (blue_num * curr_float_pixel->blue);
}

/*
 * Name: comp_video_floats_to_comp_avg_float
 * Purpose: convert pixmap of component video floats to averaged component video
 *          floats pixels
 * Parameters: UArray2b of component video floats pixels
 * Returns: UArray2 of averaged component video
 * Notes: comp_video_array must not be NULL, frees comp_video_array
 */
UArray2_T comp_video_floats_to_comp_avg_float(UArray2b_T comp_video_array)
{
        /*
         * make new array and traverse through the inputted one, changing the
         * pixels in the new one based on the pixels in the inputted one
         */
        assert(comp_video_array != NULL);
        UArray2_T comp_avg_float_arr = UArray2_new(
                UArray2b_width(comp_video_array) / BLOCKSIZE,
                UArray2b_height(comp_video_array) / BLOCKSIZE,
                sizeof(comp_avg_floats));
        /* sequence to keep track of data of other pixels in the same block */
        Seq_T seq_avg = Seq_new(SEQ_SIZE);
        comp_floats_closure cl = {.comp_avg_float_arr = comp_avg_float_arr,
                                                        .seq_avg = seq_avg};
        UArray2b_map(comp_video_array,
                                comp_video_floats_to_comp_avg_float_apply, &cl);
        Seq_free(&seq_avg);
        UArray2b_free(&comp_video_array);
        return comp_avg_float_arr;
}

/*
 * Name: comp_video_floats_to_comp_avg_float_apply
 * Purpose: convert the given component video floats pixel to an averaged
 *          component video float pixel
 * Parameters: column and row of the current pixel, the pixmap itself (which is
 *             unused), a void pointer to the current pixel, and void pointer to
 *             the closure variable 
 * Returns: none
 * Notes: sequences are used to store pixel values in the current block
 */
void comp_video_floats_to_comp_avg_float_apply(int col, int row,
                                UArray2b_T pixmap, void *entry, void *cl)
{
        /* get values from void pointers */
        comp_floats_closure *closure = cl;
        comp_video_floats *curr_video_float = entry;
        UArray2_T comp_avg_float_arr = closure->comp_avg_float_arr;
        Seq_T seq_avg = closure->seq_avg;

        /* 
         * add current pixel to sequence (keeps track of all pixels in given
         * block)
         */
        Seq_addhi(seq_avg, curr_video_float);

        /*
         * whenever all 4 pixels in the block have been traversed, perform
         * necesssary calculations. This is because the 4 pixels are averaged
         * into 1 set of numbers that represent different aspects of the pixels
         */
        if ((((row + 1) % BLOCKSIZE) == 0) && (((col + 1) % BLOCKSIZE) == 0)) {
                float bluediff = 0;
                float reddiff = 0;

                /*
                 * sequence filled with luminance values (made for easier access
                 * to the numbers). Also add together the 4 Pb values and 4
                 * Pr values.
                 */
                Seq_T luma_seq = Seq_new(BLOCKSIZE * BLOCKSIZE);
                for (int i = 0; i < Seq_length(seq_avg); i++) {
                        curr_video_float = Seq_get(seq_avg, i);
                        bluediff += curr_video_float->bluediff;
                        reddiff += curr_video_float->reddiff;
                        Seq_addhi(luma_seq, &(curr_video_float->luma));
                }

                /* perform calculations and place them in pixel structs */
                bluediff /= (BLOCKSIZE * BLOCKSIZE);
                reddiff /= (BLOCKSIZE * BLOCKSIZE);
                float *y1 = Seq_get(luma_seq, 0);
                float *y2 = Seq_get(luma_seq, 1);
                float *y3 = Seq_get(luma_seq, 2);
                float *y4 = Seq_get(luma_seq, 3);

                /*
                 * "a" value must be between 0 and 1. "b", "c", and "d" values
                 * must be between -0.3 and 0.3
                 */
                float a = ensure_in_bounds((*y4 + *y3 + *y2 + *y1)
                                        / (BLOCKSIZE * BLOCKSIZE), 0, 1);
                float c = ensure_in_bounds((*y4 - *y3 + *y2 - *y1)
                                        / (BLOCKSIZE * BLOCKSIZE), -0.3, 0.3);
                float b = ensure_in_bounds((*y4 + *y3 - *y2 - *y1)
                                        / (BLOCKSIZE * BLOCKSIZE), -0.3, 0.3);
                float d = ensure_in_bounds((*y4 - *y3 - *y2 + *y1)
                                        / (BLOCKSIZE * BLOCKSIZE), -0.3, 0.3);


                comp_avg_floats *curr_avg_floats = UArray2_at(
                        comp_avg_float_arr, col / BLOCKSIZE, row / BLOCKSIZE);
                curr_avg_floats->bluediff_avg = bluediff;
                curr_avg_floats->reddiff_avg = reddiff;
                curr_avg_floats->a = a;
                curr_avg_floats->b = b;
                curr_avg_floats->c = c;
                curr_avg_floats->d = d;

                clear_seq(seq_avg);
                Seq_free(&luma_seq);
        }

        (void) pixmap;
}

/*
 * Name: clear_seq
 * Purpose: remove all items in a sequence
 * Parameters: the given sequence
 * Returns: none
 * Notes: seq must not be NULL
 */
void clear_seq(Seq_T seq)
{
        assert(seq != NULL);
        while (Seq_length(seq) > 0) {
                Seq_remhi(seq);
        }
}

/*
 * Name: comp_avg_floats_to_comp_avg_ints
 * Purpose: quantize the pixmap of averaged component video floats pixels (send
 *          a range of float values to a set of integer values)
 * Parameters: UArray2 of averaged component video floats pixels
 * Returns: UArray2 of quantized component video pixels
 * Notes: comp_avg_floats_array must not be NULL, frees comp_avg_floats_array
 */
UArray2_T comp_avg_floats_to_comp_avg_ints(UArray2_T comp_avg_floats_array)
{
        /*
         * make new array and traverse through the inputted one, changing the
         * pixels in the new one based on the pixels in the inputted one
         */
        assert(comp_avg_floats_array != NULL);
        UArray2_T comp_avg_ints_array = UArray2_new(
                UArray2_width(comp_avg_floats_array),
                UArray2_height(comp_avg_floats_array), sizeof(comp_avg_ints));
        UArray2_map_row_major(comp_avg_floats_array,
                comp_avg_floats_to_comp_avg_ints_apply, comp_avg_ints_array);
        UArray2_free(&comp_avg_floats_array);
        return comp_avg_ints_array;
}

/*
 * Name: comp_avg_floats_to_comp_avg_ints_apply
 * Purpose: convert the given averaged component video floats pixel to a
 *          quantized integer component video pixel
 * Parameters: column and row of the current pixel, the pixmap itself (which is
 *             unused), a void pointer to the current pixel, and void pointer to
 *             the closure variable 
 * Returns: none
 * Notes: none
 */
void comp_avg_floats_to_comp_avg_ints_apply(int col, int row, UArray2_T pixmap,
                                                        void *entry, void *cl)
{
        /* get values from void pointers */
        UArray2_T comp_avg_ints_array = cl;
        comp_avg_ints *curr_avg_ints = UArray2_at(comp_avg_ints_array, col,
                                                                        row);
        comp_avg_floats *curr_avg_float = entry;

        /*
         * perform calculations and place results in pixel structs. "a" value
         * must be between 0 and 511. "b", "c", and "d" values must be between
         * -15 and and 15
         */
        curr_avg_ints->bluediff_avg = Arith40_index_of_chroma(
                                                curr_avg_float->bluediff_avg);
        curr_avg_ints->reddiff_avg = Arith40_index_of_chroma(
                                                curr_avg_float->reddiff_avg);
        curr_avg_ints->a = (int) round(ensure_in_bounds(
                                round(63 * (curr_avg_float->a)), 0, 63));
        curr_avg_ints->b = (int) round(ensure_in_bounds(
                                round(103.3 * (curr_avg_float->b)), -31, 31));
        curr_avg_ints->c = (int) round(ensure_in_bounds(
                                round(103.3 * (curr_avg_float->c)), -31, 31));
        curr_avg_ints->d = (int) round(ensure_in_bounds(
                                round(103.3 * (curr_avg_float->d)), -31, 31));

        (void) pixmap;
}

/*
 * Name: comp_avg_ints_to_out
 * Purpose: print the information in the pixels in the current UArray2 to
 *          standard output
 * Parameters: UArray2 of averaged component video ints pixels
 * Returns: none
 * Notes: comp_avg_ints_array must not be NULL, frees comp_avg_ints_array
 */
void comp_avg_ints_to_out(UArray2_T comp_avg_ints_array)
{
        /*
         * traverse through the inputted array and output the pixel information
         * in 32-bit words to standard output
         */
        assert(comp_avg_ints_array != NULL);
        printf("COMP40 Compressed image format 2\n%u %u\n",
                UArray2_width(comp_avg_ints_array) * BLOCKSIZE,
                UArray2_height(comp_avg_ints_array) * BLOCKSIZE);
        UArray2_map_row_major(comp_avg_ints_array, comp_avg_ints_to_out_apply,
                                                                        NULL);
        UArray2_free(&comp_avg_ints_array);
}

/*
 * Name: comp_avg_ints_to_out_apply
 * Purpose: print the current pixel data to standard output
 * Parameters: column and row of the current pixel (which is unused), the pixmap
 *             itself (which is unused), a void pointer to the current pixel,
 *             and void pointer to the closure variable (which is unused)
 * Returns: none
 * Notes: none
 */
void comp_avg_ints_to_out_apply(int col, int row, UArray2_T pixmap, void *entry,
                                                                void *cl)
{
        /* get values from void pointers */
        comp_avg_ints *curr_avg_ints = entry;
        uint64_t word = 0;

        /* place the values in the correct spots in the word */
        word = Bitpack_newu(word, WIDTH_A, LSB_A, curr_avg_ints->a);
        word = Bitpack_news(word, WIDTH_B_C_D, LSB_B, curr_avg_ints->b);
        word = Bitpack_news(word, WIDTH_B_C_D, LSB_C, curr_avg_ints->c);
        word = Bitpack_news(word, WIDTH_B_C_D, LSB_D, curr_avg_ints->d);
        word = Bitpack_newu(word, WIDTH_Pb_Pr, LSB_Pb,
                                                curr_avg_ints->bluediff_avg);
        word = Bitpack_newu(word, WIDTH_Pb_Pr, LSB_Pr,
                                                curr_avg_ints->reddiff_avg);
        
        /* write the word out 1 byte at a time to standard output */
        for (int lsb = 24; lsb >= 0; lsb -= 8) {
                putchar((int) (Bitpack_getu(word, 8, lsb)));
        }
        
        (void) row;
        (void) col;
        (void) cl;
        (void) pixmap;
}

#undef A2
#undef BLOCKSIZE
#undef SEQ_SIZE
#undef WIDTH_A
#undef WIDTH_B_C_D
#undef WIDTH_Pb_Pr
#undef LSB_A
#undef LSB_B
#undef LSB_C
#undef LSB_D
#undef LSB_Pb
#undef LSB_Pr