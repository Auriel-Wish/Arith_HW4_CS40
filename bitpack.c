/**************************************************************
 *
 *                     bitpack.c
 *
 *     Assignment: arith
 *     Authors:  Adam Weiss and Auriel Wish
 *     Date:     3/7/2023
 *
 *     Purpose:  Provides implementation to pack data in 64 bit
 *               words. Handles signed and unsigned integers.
 *
 **************************************************************/

#include "bitpack.h"
#include "assert.h"

Except_T Bitpack_Overflow = { "Overflow packing bits" };

#define WORD_LENGTH 64

/*
 * Name: Bitpack_fitsu
 * Purpose: determine if the binary representation of an unsigned number
 * can fit into a certain amount of bits
 * Parameters: The number and the proposed width
 * Returns: True if it fits and false if not
 * Notes: none
 */
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
        assert(width <= 64);

        /*
         * if the number can fit in the width, then there will be no 1s after
         * width offset
         */
        n = n >> width;
        return n == 0;
}

/*
 * Name: Bitpack_fitss
 * Purpose: determine if the binary representation of a signed number
 * can fit into a certain amount of bits
 * Parameters: The number and the proposed width
 * Returns: True if it fits and false if not
 * Notes: none
 */
bool Bitpack_fitss(int64_t n, unsigned width)
{
        assert(width <= 64);

        /* 0 is an edge case */
        if (n == 0) {
                return width >= 1;
        }

        /* 
         * shift the number to the right by width - 1. If the number started
         * off negative and if fits in the width, then the shifted number will
         * be -1. If the number started off positive and it fits in the width,
         * then the shifted number will be 0.
         */
        int64_t n_copy = n;
        n_copy = n_copy >> (width - 1);

        if (n < 0) {
                return n_copy == -1;
        }
        else {
                return n_copy == 0;
        }
}


/*
 * Name: Bitpack_getu
 * Purpose: Extract an unsigned number from a 64 bit codeword
 * Parameters: The codeword, the width in bits of the number to extract, and
 * the least significant bit location of the number to extract.
 * Returns: The extrated unsigned value
 * Notes: It is a CRE for the width and lsb being too large for the codeword.
 *        A field width of 0 automatically returns 0.
 */
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        assert(width + lsb <= 64);
        if (width == 0) {
                return 0;
        }

        /* make the mask */
        uint64_t mask = ~0;
        mask = mask >> (WORD_LENGTH - width);
        mask = mask << lsb;

        /*
         * get only the word bits that correspond with the mask and shift them
         * to the right edge of the uint64_t
         */
        uint64_t extraction = mask & word;
        extraction = extraction >> lsb;

        return extraction;
}

/*
 * Name: Bitpack_gets
 * Purpose: Extract a signed number from a 64 bit codeword
 * Parameters: The codeword, the width in bits of the number to extract, and
 * the least significant bit location of the number to extract.
 * Returns: The extrated signed value
 * Notes: It is a CRE for the width and lsb being too large for the codeword.
 *        A field width of 0 automatically returns 0.
 */
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
        if (width == 0) {
                return 0;
        }
        
        /*
         * store unsigned word returned by Bitpack_getu as signed word. Shift
         * word such that the leftmost bit in the desired field is also the
         * leftmost bit of the word. Then shift it back to where it was. When
         * it shifts back, it will either pull 1s or 0s along with it depending
         * on what the leftmost bit was
         */
        int64_t extraction = Bitpack_getu(word, width, lsb);
        extraction = extraction << (WORD_LENGTH - width);
        extraction = extraction >> (WORD_LENGTH - width);

        return extraction;
}


/*
 * Name: Bitpack_newu
 * Purpose: Insert an unsigned number into an existing codeword
 * Parameters: The codeword, the width in bits of the number to insert,
 * the least significant bit location of the number to insert, and the value.
 * Returns: The new codeword with the new value inserted
 * Notes: It is a CRE for the width and lsb being to large for the codeword.
          The Bitpack_Overflow exception will be raised if the proposed value
          doesn't fit in the proposed width. 
 */
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb,
                                                                uint64_t value)
{
        assert(width + lsb <= 64);
        if (!Bitpack_fitsu(value, width)) {
                RAISE(Bitpack_Overflow);
        }

        /*
         * get all the bits in the word except 0s for all of the bits in the
         * indicated field. Then merge the new value for that field with the
         * rest of the bits in the word
         */
        value = value << lsb;
        uint64_t mask = ~0;
        mask = mask >> (WORD_LENGTH - width);
        mask = mask << lsb;
        mask = ~mask;

        word = word & mask;
        word = word | value;

        return word;
}


/*
 * Name: Bitpack_news
 * Purpose: Insert a signed number into an existing codeword
 * Parameters: The codeword, the width in bits of the number to insert,
 * the least significant bit location of the number to insert, and the value.
 * Returns: The new codeword with the new value inserted
 * Notes: It is a CRE for the width and lsb being to large for the codeword.
          The Bitpack_Overflow exception will be raised if the proposed value
          doesn't fit in the proposed width. 
 */
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb,
                                                                int64_t value)
{
        assert(width + lsb <= 64);
        if (!Bitpack_fitss(value, width)) {
                RAISE(Bitpack_Overflow);
        }

        /* knock off any leading 1s and treat value as an unsigned value */
        uint64_t real_val = value << (WORD_LENGTH - width);
        real_val = real_val >> (WORD_LENGTH - width);

        return Bitpack_newu(word, width, lsb, real_val);
}
