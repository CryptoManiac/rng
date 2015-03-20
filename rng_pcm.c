#include <stdio.h>
#include <stdlib.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

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

    uint8_t  count = 0;

    int16_t curr_val = 0, local_peak = 0;

    uint16_t rising_points = 0, falling_points = 0;

    uint64_t curr_pos=0,
         pos1=0,
         pos2=0,
         pos3=0,
         pos4=0,
         next_pos = 0;

    size_t res = 0;
    for(;;) {
        res = fread(&curr_val, sizeof(int16_t), 1, stdin);

        if (res != 1) {
            printf("\nSample data doesn't provide sufficient amount of bytes!\n");
            break;
        }

        if (feof(stdin)) {
            printf("\nNo sample data left.\n");
            break;
        }

        if (curr_val < 0)
            curr_val = 0;

        if (local_peak < curr_val) {
            local_peak = curr_val;
            rising_points++;
        } else {
            if (rising_points < 3) {
                rising_points = falling_points = local_peak = 0;
            }
            else if (local_peak > curr_val) {
                falling_points++;
            }
        }

//        printf("Peak=%i, current=%i, Rising=%d, falling=%d\n", local_peak, curr_val, rising_points, falling_points);

        if (rising_points > 3 && falling_points > 3) {

            falling_points = rising_points = local_peak = 0;

            switch(count++) {
            case 0:
                pos1 = curr_pos;
            break;
            case 1:
                pos2 = curr_pos;
            break;
            case 2:
                pos3 = curr_pos;
            break;
            case 3:
                pos4 = curr_pos;

                if (pos4 - pos3 > pos2 - pos1) {
                    printf("1");
                    WriteBit(1, f);
                } else if (pos4 - pos3 < pos2 - pos1) {
                    printf("0");
                    WriteBit(0, f);
                }
                fflush(stdout);

                count = 0;
                pos1 = pos2 = pos3 = pos4 = 0;

                break;
            default:
                count = 0;
                pos1 = pos2 = pos3 = pos4 = 0;
            }
        }

        curr_pos++;
    }
}
