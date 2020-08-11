#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <memory.h>
#include <time.h>

//
// Example usage 1:
//  ./psc file "AA BB ?? CC"
// Example output:
//  Pattern length: 4
//  File size: 0xFEFEFEFE
//  Found result[0] at offset 0xdeadbeef
//
//  Found 1 match(es) in 0.0000001s.
//

//
// Example usage 2:
//  ./psc file "AA BB ?? CC" 4 (bytes to show)
// Example output 2:
//  Pattern length: 4
//  File size: 0xFEFEFEFE
//  Found result[0] at offset 0xdeadbeef
//  Bytes:
//  AA BB C3 CC DD FF EE C4
//
//  Found 1 match(es) in 0.0000001s.
//

uint8_t hex_char_to_byte(char c);

int main(int argc, char **argv) {
    FILE *fp;               // File pointer
    long len;               // File length (in bytes)
    char *buf, *pt_str;     // Pointer to file buffer & pattern
    uint8_t *pt_buf;        // Pointer to array of bytes parsed from pattern
    uint64_t pt_mask;       // Bitset mask of wildcards (bytes to ignore)
    clock_t start, end;     // Start and end times
    double diff, dtime;     // Difference between start & end and time of execution
    uint32_t pt_len;        // Length of pattern
    uint32_t bytes_to_show; // Amount of bytes to show per result
    uint32_t results_found; // Amount of pattern matches

    if (argc < 3) {
        printf("Incorrect usage!\n");
        return 1;
    }

    start = clock();

    //
    // Parse pattern length, simply assume that len = spaces + 1
    //

    pt_str = argv[2];
    pt_len = 1;

    for (uint32_t i = 0; pt_str[i] != '\0'; ++i) {
        if (pt_str[i] == ' ') pt_len++;
    }

    printf("Pattern length: %u\n", pt_len);

    //
    // Decide how many bytes to show of pattern result
    //

    if (argc > 3) {
        bytes_to_show = strtoul(argv[3], NULL, 0);
        bytes_to_show += pt_len;
    } else {
        bytes_to_show = 0;
    }

    //
    // Parse pattern & mask from string
    //

    pt_buf = malloc(pt_len * sizeof(uint8_t));
    memset(pt_buf, 0, pt_len * sizeof(uint8_t));

    pt_mask = ~0;

    for (uint32_t i = 0; i < pt_len; ++i) {
        char *byte_str = &pt_str[i * 3];
        bool is_wildcard = byte_str[0] == '?';
        if (is_wildcard) {
            pt_mask &= ~(1 << i);
        } else {
            //
            // Sure we could use some shit like strtoul but since we know bytes are two characters this is faster and easier.
            //

            pt_buf[i] = 0x10 * hex_char_to_byte(byte_str[0]);
            pt_buf[i] += 0x01 * hex_char_to_byte(byte_str[1]);
        }
    }

    //
    // Open file, read to buffer and close
    //

    fp = fopen(argv[1], "rb");
    if (!fp) {
        printf("Failed to open file \"%s\"!\n", argv[1]);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    buf = malloc(len);
    fread(buf, 1, len, fp);

    fclose(fp);
    fp = NULL;

    printf("File size: 0x%lX\n", len);

    //
    // Scan for pattern
    //

    for (uintptr_t i = 0; i < len; ++i ) {
	const uint8_t *scan_buf = (uint8_t *)((uintptr_t)buf + i);
	bool found_pt = true;
	for (uintptr_t h = 0; h < pt_len; ++h) {
		if (!(pt_mask & ((uint64_t)1 << h)))
			continue;

		if (scan_buf[h] != pt_buf[h]) {
			found_pt = 0;
			break;
		}
	}

	if (found_pt) {
            printf("Found pattern[%u] at offset %p\n", results_found++, (void *)i);
            if (bytes_to_show > 0) {
                puts("Bytes: ");
                for (uint32_t j = 0; j < bytes_to_show; ++j) {
                    printf("%02X ", scan_buf[j]);
                }
                puts("\n");
            }
        }
    }

    end = clock();
    diff = (double)end - start;
    dtime = diff / CLOCKS_PER_SEC;

    printf("Found %u match(es) in %fs.\n", results_found, dtime);

    free(buf);
    free(pt_buf);

    return 0;
}

uint8_t hex_char_to_byte(char c) {
    if (c >= '0' && c < ';')
        return c - '0';
    if (c >= 'A' && c < 'G')
        return 10 + c - 'A';
    if (c >= 'a' && c < 'g')
        return 10 + c - 'a' ;

    return 0;
}
