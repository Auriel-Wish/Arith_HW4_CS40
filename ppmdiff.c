#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"
#include "assert.h"

float square (float num);

int main(int argc, char *argv[]) 
{
        //create file pointers and assert correct number of command line args
        FILE *input1fp = stdin;
        FILE *input2fp = stdin;
        int numIns = 2;
        if (argc != 3) {
                fprintf(stderr, "usage: ppmdiff [file1] [file2]");
                return EXIT_FAILURE;
        }

        //open the files
        if (strcmp(argv[1], "-") != 0) {
                input1fp = fopen(argv[1], "r");
                assert(input1fp != NULL);
                numIns--;
        }
        if (strcmp(argv[2], "-") != 0) {
                input2fp = fopen(argv[2], "r");
                assert(input2fp != NULL);
                numIns--;
        }
        if (numIns == 2) {
                fprintf(stderr, "ONLY ONE ARG MAY BE STDIN");
                return EXIT_FAILURE;
        }
        
        //turn files into Pnm_ppms
        A2Methods_T methods = uarray2_methods_plain; 
        Pnm_ppm input_image1 = Pnm_ppmread(input1fp, methods);
        Pnm_ppm input_image2 = Pnm_ppmread(input2fp, methods);
        fclose(input1fp);
        fclose(input2fp);

        //check width and height difference
        int heightDiff = abs((int)
                        input_image1->height - (int) input_image2->height);
        int widthDiff = abs((int)
                        input_image1->width - (int) input_image2->width);
        if (heightDiff > 1 || widthDiff > 1) {
                fprintf(stderr, "INCOMPATABLE IMAGES");
                fprintf(stdout, "1.0");
                return EXIT_FAILURE;
        }
        
        // set width and height to be traversed
        int width = 0;
        int height = 0;
        float den = input_image1->denominator;
        
        if (input_image1->height < input_image2->height) {
                height = input_image1->height;
        } else {
                height = input_image2->height;
        }

        if (input_image1->width < input_image2->width) {
                width = input_image1->width;
        } else {
                width = input_image2->width;
        }

        ///traverse and keep the sum
        Pnm_rgb curr_pixel_img1;
        Pnm_rgb curr_pixel_img2;
        float red1, red2, green1, green2, blue1, blue2 = 0;
        float sum = 0;
        for (int i = 0; i < width; i++) {
                for (int j = 0; j < height; j++) {
                        curr_pixel_img1 =
                          input_image1->methods->at(input_image1->pixels, i, j);
                        curr_pixel_img2 =
                          input_image2->methods->at(input_image2->pixels, i, j);
                        
                        red1 = (float) curr_pixel_img1->red / den;
                        red2 = (float) curr_pixel_img2->red / den;
                        green1 = (float) curr_pixel_img1->green / den;
                        green2 = (float) curr_pixel_img2->green / den;
                        blue1 = (float) curr_pixel_img1->blue / den;
                        blue2 = (float) curr_pixel_img2->blue / den;

                        sum += square(red1 - red2) + square(green1 - green2) +
                                                        square(blue1 - blue2);
                }
        }

        sum = sum / ((float) 3 * (float) width * (float) height);
        sum = sqrt(sum);
        printf("%f%%\n", sum * 100);
        
        Pnm_ppmfree(&input_image1);
        Pnm_ppmfree(&input_image2);

        return EXIT_SUCCESS;
}


//quick LIL square function cs11 ya know what I'm SAYINNNN
float square (float num) {
        return num * num;
}