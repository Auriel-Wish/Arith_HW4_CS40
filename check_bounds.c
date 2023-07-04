/**************************************************************
 *
 *                     check_bounds.c
 *
 *     Assignment: arith
 *     Authors:  Adam Weiss and Auriel Wish
 *     Date:     3/7/2023
 *
 *     Purpose: contain function used in compression and
 *              decompression that ensures a supplied value
 *              is within bounds.
 *
 **************************************************************/

/*
 * Name: ensure_in_bounds_decompress
 * Purpose: keep given float in desired bounds
 * Parameters: given float, minimum and maximum desired float values
 * Returns: the bounded float
 * Notes: there is an equivalent function in compress.c
 */
float ensure_in_bounds(float val, float min, float max)
{
        /*
         * if value is below minimum, return minimum, and if value is above
         * maximum, return maximum
         */
        if (val < min) {
                return min;
        }
        else if (val > max) {
                return max;
        }
        else {
                return val;
        }
}