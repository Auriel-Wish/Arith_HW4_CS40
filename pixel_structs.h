/**************************************************************
 *
 *                     pixel_structs.h
 *
 *     Assignment: arith
 *     Authors:  Adam Weiss and Auriel Wish
 *     Date:     3/7/2023
 *
 *     Purpose: contains struct definitions for different types
 *              of pixel information
 *
 **************************************************************/

#ifndef PIXEL_STRUCTS_INCLUDED
#define PIXEL_STRUCTS_INCLUDED

/*
 * Name: rgb_floats
 * contains: rgb values in float form
 */
struct rgb_floats {
        float red;
        float green;
        float blue;
};
typedef struct rgb_floats rgb_floats;

/*
 * Name: comp_video_floats
 * contains: Pb, Pr, and Y values in float form
 */
struct comp_video_floats {
        float bluediff;
        float reddiff;
        float luma;
};
typedef struct comp_video_floats comp_video_floats;

/*
 * Name: comp_avg_floats
 * contains: averaged Pb and Pr values, Y values in terms of a, b, c, and d
 */
struct comp_avg_floats {
        float bluediff_avg;
        float reddiff_avg;
        float a, b, c, d;
};
typedef struct comp_avg_floats comp_avg_floats;

/*
 * Name: comp_avg_ints
 * contains: quantized Pb, Pr, and luminance values
 */
struct comp_avg_ints {
        uint64_t bluediff_avg;
        uint64_t reddiff_avg;
        uint64_t a;
        int64_t b, c, d;
};
typedef struct comp_avg_ints comp_avg_ints;

#endif