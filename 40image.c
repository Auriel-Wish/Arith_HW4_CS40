/**************************************************************
 *
 *                     40image.c
 *
 *     Assignment: arith
 *     Authors:  Adam Weiss and Auriel Wish
 *     Date:     3/7/2023
 *
 *     Purpose:  Either compress a ppm image into data that is
 *               approximately 1/3 the size of the original image
 *               data OR decompress the compressed image and
 *               output it as close to the original image as possible
 *
 **************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "compress40.h"
#include "compress.h"
#include "decompress.h"

static void (*compress_or_decompress)(FILE *input) = compress40;

int main(int argc, char *argv[])
{
        int i;

        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-c") == 0) {
                        compress_or_decompress = compress40;
                } else if (strcmp(argv[i], "-d") == 0) {
                        compress_or_decompress = decompress40;
                } else if (*argv[i] == '-') {
                        fprintf(stderr, "%s: unknown option '%s'\n",
                                argv[0], argv[i]);
                        exit(1);
                } else if (argc - i > 2) {
                        fprintf(stderr, "Usage: %s -d [filename]\n"
                                "       %s -c [filename]\n",
                                argv[0], argv[0]);
                        exit(1);
                } else {
                        break;
                }
        }
        assert(argc - i <= 1);    /* at most one file on command line */
        
        if (i < argc) {
                FILE *fp = fopen(argv[i], "r");
                assert(fp != NULL);
                compress_or_decompress(fp);
                fclose(fp);
        } else {
                compress_or_decompress(stdin);
        }

        return EXIT_SUCCESS; 
}

void compress40(FILE *fp) {
        Pnm_ppm original = ppm_to_rgb_int(fp);
        UArray2_T rgb_float_array = rgb_int_to_rgb_float(original);
        UArray2b_T comp_video_array =
                                rgb_float_to_component_video(rgb_float_array);
        UArray2_T comp_avg_float_array =
                        comp_video_floats_to_comp_avg_float(comp_video_array);
        UArray2_T comp_avg_int_array =
                        comp_avg_floats_to_comp_avg_ints(comp_avg_float_array);
        comp_avg_ints_to_out(comp_avg_int_array);
}

void decompress40(FILE *fp) {
        UArray2_T comp_avg_int_array = word_to_comp_avg_ints(fp);
        UArray2_T comp_avg_float_array =
                        comp_avg_ints_to_comp_avg_floats(comp_avg_int_array);
        UArray2b_T comp_video_array =
                comp_avg_float_to_comp_video_floats(comp_avg_float_array);
        UArray2_T rgb_float_array =
                                component_video_to_rgb_float(comp_video_array);
        Pnm_ppm output = rgb_float_to_rgb_int(rgb_float_array);
        rgb_int_to_ppm(output);
}