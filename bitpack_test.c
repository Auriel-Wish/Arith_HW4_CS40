#include "bitpack.h"
#include <stdio.h>

#include "assert.h"

int main() {
        uint64_t word = 1238478491; 
        unsigned width = 34;
        unsigned width2 = 55;
        unsigned lsb = 13;
        unsigned lsb2 = 49;
        uint64_t u_num = 62946;
        int64_t s_num = 7901736;

        assert(Bitpack_getu(Bitpack_newu(word, width, lsb, u_num), width, lsb) == u_num);
        assert(Bitpack_gets(Bitpack_news(word, width, lsb, s_num), width, lsb) == s_num);

        assert(Bitpack_getu(Bitpack_newu(word, width, lsb, u_num), width2, lsb2) == Bitpack_getu(word, width2, lsb2));
        assert(Bitpack_gets(Bitpack_news(word, width, lsb, s_num), width2, lsb2) == Bitpack_gets(word, width2, lsb2));
        
        // unsigned width = 1;
        // uint64_t test_num2 = 16;
        // int64_t test_num1 = -1;

        // if (Bitpack_fitsu(test_num2, width)) {
        //         printf("Unsigned: %lu fits in %u\n", test_num2, width);
        // } else {
        //         printf("Unsigned: L loser + ratio\n");
        // }
        // if (Bitpack_fitss(test_num1, width)) {
        //         printf("Signed: %ld fits in %u\n", test_num1, width);
        // } else {
        //         printf("Signed: L loser + ratio\n");
        // }
        
        // uint64_t word = 5;
        // unsigned width = 3;
        // unsigned lsb = 0;
        // uint64_t value1 = Bitpack_getu(word, width, lsb);
        // uint64_t value2 = Bitpack_gets(word, width, lsb);
        // printf("Unsigned: %ld\nSigned: %ld\n", value1, value2);

        // uint64_t word = 53;
        // unsigned width = 3;
        // unsigned lsb = 2;
        // int64_t field1 = 4;

        // uint64_t original1 = Bitpack_getu(word, width, lsb);
        // uint64_t new1 = Bitpack_news(word, width, lsb, field1);
        // new1 = Bitpack_getu(new1, width, lsb);
        // printf("Original: %ld\nNew: %ld\n", original1, new1);
        return 0;
}