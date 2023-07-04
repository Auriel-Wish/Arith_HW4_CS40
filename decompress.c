/**************************************************************
 *
 *                     decompress.c
 *
 *     Assignment: arith
 *     Authors:  Adam Weiss and Auriel Wish
 *     Date:     3/7/2023
 *
 *     Purpose:  Provide functions to decompress a bitpacked
 *               image and output it.
 *
 **************************************************************/

#include "decompress.h"

#define DENOMINATOR 255
#define A2 A2Methods_UArray2
#define PNM_RGB_SIZE 12
#define BLOCKSIZE 2
#define WORD_LENGTH 32
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
float calculate_rgb_float(comp_video_floats *curr_video_pixel,
                                        float bluediff_num, float reddiff_num);
void rgb_float_to_rgb_int_apply(int col, int row, A2 pixmap, void *entry,
                                                                void *cl);
void component_video_to_rgb_float_apply(int col, int row, UArray2b_T pixmap,
                                                        void *entry, void *cl);
void comp_avg_float_to_comp_video_floats_apply(int col, int row,
                                UArray2_T pixmap, void *entry, void *cl);
void comp_avg_ints_to_comp_avg_floats_apply(int col, int row, UArray2_T pixmap,
                                                        void *entry, void *cl);
void word_to_comp_avg_ints_apply(int col, int row, UArray2_T pixmap,
                                                        void *entry, void *cl);
float ensure_in_bounds(float val, float min, float max);

/*
 * Name: rgb_int_to_ppm
 * Purpose: Print a pnm_ppm image struct to standard output
 * Parameters: A ppm image struct
 * Returns: none
 * Notes: output_image must not be NULL, frees output_image
 */
void rgb_int_to_ppm(Pnm_ppm output_image)
{
        /* write the image file to standard out and free the pnm_ppm struct */
        assert(output_image != NULL);
        Pnm_ppmwrite(stdout, output_image);
        Pnm_ppmfree(&output_image);
}

/*
 * Name: rgb_float_to_rgb_int
 * Purpose: Convert the rgb floats pixels to scaled rgb integers by multiplying 
 *          by the denominator.
 * Parameters: A UArray2 containing rgb floats pixels
 * Returns: A Pnm_ppm containing a pixmap of the rbg ints pixels
 * Notes: rgb_float_array must not be NULL, frees rgb_float_array
 */
Pnm_ppm rgb_float_to_rgb_int(UArray2_T rgb_float_array)
{
        assert(rgb_float_array != NULL);
        
        /* create a methods suite instance */
        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods); 

        /* Create and set values in a new Pnm_ppm struct */
        Pnm_ppm output_image;
        NEW(output_image);
        assert(output_image != NULL);
        output_image->methods = methods;
        output_image->denominator = DENOMINATOR;
        output_image->width = UArray2_width(rgb_float_array);
        output_image->height = UArray2_height(rgb_float_array);
        A2 rgb_int_array = methods->new(output_image->width,
                                        output_image->height, PNM_RGB_SIZE);

        output_image->pixels = rgb_int_array;

        /* Convert all the rgb floats to scaled rgb ints and store them */
        methods->map_default(rgb_int_array, rgb_float_to_rgb_int_apply,
                                                        rgb_float_array);
        UArray2_free(&rgb_float_array);
        return output_image;
}

/*
 * Name: rgb_float_to_rgb_int_apply
 * Purpose: convert the given rgb float pixel to an rgb integer pixel
 * Parameters: column and row of the current pixel, the pixmap itself (which is
 *             unused), a void pointer to the current pixel, and void pointer to
 *             the closure variable 
 * Returns: none
 * Notes: none
 */
void rgb_float_to_rgb_int_apply(int col, int row, A2 pixmap, void *entry,
                                                                void *cl)
{
        /* get values from void pointers */
        Pnm_rgb curr_int_pixel = entry;
        UArray2_T rgb_float_array = cl;

        /*
         * perform calculations and place results in pixel structs. Values must
         * be between 0 and 255 (denominator)
         */
        rgb_floats *curr_float_pixel = UArray2_at(rgb_float_array, col, row);
        curr_int_pixel->red = (unsigned) round(ensure_in_bounds(
                round(((curr_float_pixel->red) * DENOMINATOR)),
                                                        0, DENOMINATOR));
        curr_int_pixel->green = (unsigned) round(ensure_in_bounds(
                round(((curr_float_pixel->green) * DENOMINATOR)),
                                                        0, DENOMINATOR));
        curr_int_pixel->blue = (unsigned) round(ensure_in_bounds(
                round(((curr_float_pixel->blue) * DENOMINATOR)),
                                                        0, DENOMINATOR));
        (void) pixmap;
}

/*
 * Name: component_video_to_rgb_float
 * Purpose: Convert component video space pixels to rgb float pixels
 * Parameters: A UArray2b of the component video space pixels
 * Returns: A UArray2 containing rgb float pixels
 * Notes: comp_video_array must not be NULL, frees comp_video_array
 */
UArray2_T component_video_to_rgb_float(UArray2b_T comp_video_array)
{
        /*
         * make new array and traverse through the inputted one, changing the
         * pixels in the new one based on the pixels in the inputted one
         */
        assert(comp_video_array != NULL);
        UArray2_T rgb_float_array = UArray2_new(
                UArray2b_width(comp_video_array),
                UArray2b_height(comp_video_array), sizeof(rgb_floats));
        UArray2b_map(comp_video_array, component_video_to_rgb_float_apply,
                                                        rgb_float_array);
        UArray2b_free(&comp_video_array);
        return rgb_float_array;
}

/*
 * Name: component_video_to_rgb_float_apply
 * Purpose: convert the given component video pixel to an rgb float pixel
 * Parameters: column and row of the current pixel, the pixmap itself (which is
 *             unused), a void pointer to the current pixel, and void pointer to
 *             the closure variable 
 * Returns: none
 * Notes: none
 */
void component_video_to_rgb_float_apply(int col, int row, UArray2b_T pixmap,
                                                        void *entry, void *cl)
{
        /* get values from void pointers */
        UArray2_T rgb_float_array = cl;
        rgb_floats *curr_float_pixel = UArray2_at(rgb_float_array, col, row);

        /*
         * perform calculations and place results in pixel structs. Values must
         * be between 0 and 1
         */
        comp_video_floats *curr_video_pixel = entry;
        curr_float_pixel->red =
                 calculate_rgb_float(curr_video_pixel, 0, 1.402);
        curr_float_pixel->green =
                calculate_rgb_float(curr_video_pixel, -0.344136, -0.714136);
        curr_float_pixel->blue =
                calculate_rgb_float(curr_video_pixel, 1.772, 0);
        (void) pixmap;
}

/*
 * Name: calculate_rgb_float
 * Purpose: perform the calculation provided by the spec to convert Y, Pb, and
 *          Pr values to rgb float values
 * Parameters: a pointer to the current pixel, the constants that will be used
 *             for the calculation
 * Returns: the calculated value (red, green, or blue value)
 * Notes: none
 */
float calculate_rgb_float(comp_video_floats *curr_video_pixel,
                                        float bluediff_num, float reddiff_num)
{
        float val = (curr_video_pixel->luma) +
                        (bluediff_num * curr_video_pixel->bluediff) +
                        (reddiff_num * curr_video_pixel->reddiff);
        return ensure_in_bounds(val, 0, 1);
}

/*
 * Name: comp_avg_float_to_comp_video_floats
 * Purpose: convert pixmap of averaged component video floats to component video
 *          floats pixels
 * Parameters: UArray2 of averaged component video float pixels
 * Returns: UArray2b of component video floats pixels
 * Notes: comp_avg_float_arr must not be NULL, frees comp_avg_float_arr. Also,
 *        the component video space array is a UArray2b because it will be
 *        traversed in block major order.
 */
UArray2b_T comp_avg_float_to_comp_video_floats(UArray2_T comp_avg_float_arr) {
        /*
         * make new array and traverse through the inputted one, changing the
         * pixels in the new one based on the pixels in the inputted one
         */
        assert(comp_avg_float_arr != NULL);
        UArray2b_T comp_video_array = UArray2b_new(
                UArray2_width(comp_avg_float_arr) * BLOCKSIZE,
                UArray2_height(comp_avg_float_arr) * BLOCKSIZE,
                sizeof(comp_video_floats), BLOCKSIZE);
        UArray2_map_row_major(comp_avg_float_arr,
                comp_avg_float_to_comp_video_floats_apply, comp_video_array);
        UArray2_free(&comp_avg_float_arr);
        return comp_video_array;
}

/*
 * Name: comp_avg_float_to_comp_video_floats_apply
 * Purpose: convert the given averaged component video floats pixel to a
 *          component video float pixel
 * Parameters: column and row of the current pixel, the pixmap itself (which is
 *             unused), a void pointer to the current pixel, and void pointer to
 *             the closure variable 
 * Returns: none
 * Notes: none
 */
void comp_avg_float_to_comp_video_floats_apply(int col, int row,
                                        UArray2_T pixmap, void *entry, void *cl)
{
        /* get values from void pointers */
        UArray2b_T comp_video_array = cl;
        comp_avg_floats *curr_avg_floats = entry;

        /*
         * distribute averaged values into 4 pixels - these pixels all have the
         * same Pb and Pr, but they have different Y values
         */
        comp_video_floats *curr_video_floats1 = UArray2b_at(comp_video_array,
                                col * BLOCKSIZE, row * BLOCKSIZE);
        comp_video_floats *curr_video_floats2 = UArray2b_at(comp_video_array,
                                col * BLOCKSIZE + 1, row * BLOCKSIZE);
        comp_video_floats *curr_video_floats3 = UArray2b_at(comp_video_array,
                                col * BLOCKSIZE, row * BLOCKSIZE + 1);
        comp_video_floats *curr_video_floats4 = UArray2b_at(comp_video_array,
                                col * BLOCKSIZE + 1, row * BLOCKSIZE + 1);

        float a = curr_avg_floats->a;
        float b = curr_avg_floats->b;
        float c = curr_avg_floats->c;
        float d = curr_avg_floats->d;
        float reddiff_avg = curr_avg_floats->reddiff_avg;
        float bluediff_avg = curr_avg_floats->bluediff_avg;

        curr_video_floats1->bluediff = bluediff_avg;
        curr_video_floats2->bluediff = bluediff_avg;
        curr_video_floats3->bluediff = bluediff_avg;
        curr_video_floats4->bluediff = bluediff_avg;

        curr_video_floats1->reddiff = reddiff_avg;
        curr_video_floats2->reddiff = reddiff_avg;
        curr_video_floats3->reddiff = reddiff_avg;
        curr_video_floats4->reddiff = reddiff_avg;

        /* luminance values must be between 0 and 1 */
        curr_video_floats1->luma =
                        ensure_in_bounds((a - b - c + d), 0, 1);
        curr_video_floats2->luma =
                        ensure_in_bounds((a - b + c - d), 0, 1);
        curr_video_floats3->luma =
                        ensure_in_bounds((a + b - c - d), 0, 1);
        curr_video_floats4->luma =
                        ensure_in_bounds((a + b + c + d), 0, 1);

        (void) pixmap;
}

/*
 * Name: comp_avg_ints_to_comp_avg_floats
 * Purpose: unquantize the pixmap of averaged component video int pixels (send
 *          the integers to their float forms)
 * Parameters: UArray2 of quantized component video pixels
 * Returns: UArray2 of unquantized component video float pixels
 * Notes: comp_avg_int_arr must not be NULL, frees comp_avg_int_arr
 */
UArray2_T comp_avg_ints_to_comp_avg_floats(UArray2_T comp_avg_int_arr)
{
        /*
         * make new array and traverse through the inputted one, changing the
         * pixels in the new one based on the pixels in the inputted one
         */
        assert(comp_avg_int_arr != NULL);
        UArray2_T comp_avg_float_arr =UArray2_new(
                UArray2_width(comp_avg_int_arr),
                UArray2_height(comp_avg_int_arr), sizeof(comp_avg_floats));
        UArray2_map_row_major(comp_avg_int_arr,
                comp_avg_ints_to_comp_avg_floats_apply, comp_avg_float_arr);
        UArray2_free(&comp_avg_int_arr);
        return comp_avg_float_arr;
}

/*
 * Name: comp_avg_ints_to_comp_avg_floats_apply
 * Purpose: convert the given quantized component video pixel to an unquantized
 *          component video float pixel
 * Parameters: column and row of the current pixel, the pixmap itself (which is
 *             unused), a void pointer to the current pixel, and void pointer to
 *             the closure variable 
 * Returns: none
 * Notes: none
 */
void comp_avg_ints_to_comp_avg_floats_apply(int col, int row, UArray2_T pixmap,
                                                        void *entry, void *cl)
{
        /* get values from void pointers */
        UArray2_T comp_avg_float_arr = cl;
        comp_avg_floats *curr_avg_floats = UArray2_at(comp_avg_float_arr, col,
                                                                        row);
        comp_avg_ints *curr_avg_ints = entry;

        /*
         * perform calculations and place results in pixel structs. "a" value
         * must be between 0 and 1. "b", "c", and "d" values must be between
         * -0.5 and and 0.5
         */
        curr_avg_floats->bluediff_avg =
                        Arith40_chroma_of_index(curr_avg_ints->bluediff_avg);
        curr_avg_floats->reddiff_avg =
                        Arith40_chroma_of_index(curr_avg_ints->reddiff_avg);
        curr_avg_floats->a =
        ensure_in_bounds(((float) curr_avg_ints->a) / 63, 0, 1);
        curr_avg_floats->b =
        ensure_in_bounds(((float) curr_avg_ints->b) / 103.3, -0.5, 0.5);
        curr_avg_floats->c =
        ensure_in_bounds(((float) curr_avg_ints->c) / 103.3, -0.5, 0.5);
        curr_avg_floats->d =
        ensure_in_bounds(((float) curr_avg_ints->d) / 103.3, -0.5, 0.5);
        
        (void) pixmap;
}

/*
 * Name: word_to_comp_avg_ints
 * Purpose: reads in data from file and puts it into a UArray2
 * Parameters: pointer to input file
 * Returns: UArray2 of component video int pixels
 * Notes: input must not be NULL
 */
UArray2_T word_to_comp_avg_ints(FILE *input)
{
        assert(input != NULL);

        /* check for the correct header and get the width and height */
        unsigned height, width;
        int read = fscanf(input, "COMP40 Compressed image format 2\n%u %u",
                                                        &width, &height);
        assert(read == 2);
        int c = getc(input);
        assert(c == '\n');

        /*
         * make new array and traverse through it, changing the pixels in it
         * based on the input from the file
         */
        UArray2_T comp_avg_ints_array = UArray2_new(width / BLOCKSIZE,
                                height / BLOCKSIZE, sizeof(comp_avg_ints));
        UArray2_map_row_major(comp_avg_ints_array, word_to_comp_avg_ints_apply,
                                                                        input);

        return comp_avg_ints_array;
}

/*
 * Name: word_to_comp_avg_ints_apply
 * Purpose: read in one 32-bit word from input file and place data in current
 *          pixel struct
 * Parameters: column and row of the current pixel (which is unused), the pixmap
 *             itself (which is unused), a void pointer to the current pixel,
 *             and void pointer to the closure variable
 * Returns: none
 * Notes: none
 */
void word_to_comp_avg_ints_apply(int col, int row, UArray2_T pixmap,
                                                        void *entry, void *cl)
{
        /* get values from void pointers */
        FILE *input = cl;
        comp_avg_ints *curr_avg_int = entry;
        
        /* 
         * read from the file 1 byte at a time  and place that byte in its
         * appropriate spot in the word
         */
        int curr_bits;
        uint64_t word = 0;
        for (int i = 0; i < WORD_LENGTH / 8; i++) {
                curr_bits = getc(input);
                assert(!feof(input));
                word = Bitpack_newu(word, 8, WORD_LENGTH - (i + 1) * 8,
                                                                curr_bits);
        }

        /* get the desired values from there spots in the word */
        uint64_t a = Bitpack_getu(word, WIDTH_A, LSB_A);
        int64_t b = Bitpack_gets(word, WIDTH_B_C_D, LSB_B);
        int64_t c = Bitpack_gets(word, WIDTH_B_C_D, LSB_C);
        int64_t d = Bitpack_gets(word, WIDTH_B_C_D, LSB_D);
        uint64_t bluediff_avg = Bitpack_getu(word, WIDTH_Pb_Pr, LSB_Pb);
        uint64_t reddiff_avg = Bitpack_getu(word, WIDTH_Pb_Pr, LSB_Pr);

        /* place the values in the current pixel struct */
        curr_avg_int->a = a;
        curr_avg_int->b = b;
        curr_avg_int->c = c;
        curr_avg_int->d = d;
        curr_avg_int->bluediff_avg = bluediff_avg;
        curr_avg_int->reddiff_avg = reddiff_avg;
        
        (void) col;
        (void) row;
        (void) pixmap;
}

#undef DENOMINATOR
#undef A2
#undef PNM_RGB_SIZE
#undef BLOCKSIZE
#undef WIDTH_A
#undef WIDTH_B_C_D
#undef WIDTH_Pb_Pr
#undef LSB_A
#undef LSB_B
#undef LSB_C
#undef LSB_D
#undef LSB_Pb
#undef LSB_Pr