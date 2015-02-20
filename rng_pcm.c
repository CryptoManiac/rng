#include <stdio.h>
#include <stdlib.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

uint64_t curr_pos=0, prev_zero_pos=0,
         pos1=0, pos2=0, pos3=0, pos4=0;

void WriteBit (int bit, FILE *f)
{
    static volatile int current_bit = 0;
    static volatile unsigned char bit_buffer = 0;
    unsigned char byte;

    if (bit)
        bit_buffer |= (1<<(7-current_bit));

    if (++current_bit > 7) {
        byte = bit_buffer;
        fwrite (&byte, 1, 1, f);
        fflush(f);
        current_bit = 0;
        bit_buffer = 0;

        printf(" 0x%02X\r\n", byte);
    }
}


main (int argc, char *argv[])
{
    FILE *f = fopen("out.bin", "ab+");

    uint8_t  curr_val = 0;
    uint8_t  count = 0;

    for(;;) {
        curr_val = fgetc(stdin);

        if (feof(stdin))
            continue;

        if (curr_pos < 44) {
            curr_pos++;
            continue;
        }


        if (curr_val == 0) {
            switch(count++) {
            case 0:
                pos1 = curr_pos;
//                printf("1\n");
                break;
            case 1:
                pos2 = curr_pos;
//                printf("2\n");
                break;
            case 2:
                pos3 = curr_pos;
//                printf("3\n");
                break;
            case 3:
                pos4 = curr_pos;
//                printf("4\n");

                if (pos4 - pos3 > pos2 - pos1) {
                    printf("1");
                    WriteBit(1, f);
                } else if (pos4 - pos3 < pos2 - pos1) {
                    printf("0");
                    WriteBit(0, f);
                }
                fflush(stdout);

                break;
            default:
                count = 0;
                pos1 = pos2 = pos3 = pos4 = 0;
            }
            prev_zero_pos = curr_pos;
        }

        curr_pos++;
    }
}
