//
//
//    "Efinix Constant Converter Software" for
//             Efinix SPI Passive Mode Arduino Programmer
//
//     https://github.com/htlabnet/Efinix_SPI_Passive_Mode_Arduino_Programmer
//
//    Copyright (C) 2023
//      Hideto Kikuchi / PJ (@pcjpnet) - http://pc-jp.net/
//      Tsukuba Science Inc. - http://www.tsukuba-kagaku.co.jp/
//
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <ctype.h>

#include "md5.h"

#define BUF_SIZE 256

int main(int argc, char *argv[])
{
    struct stat stat_buf;

    printf("==================================================\n");
    printf("File : %s\n",argv[1]);

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *file_ext = strrchr(argv[1], '.');
    if(strcmp(file_ext, ".hex") != 0) {
        fprintf(stderr, "Must Use HEX File!\nInput: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    if (stat(argv[1], &stat_buf) == 0) {
        // Get File Info (Size/Time)
        printf("Size : %ld\n", (long)stat_buf.st_size);
        struct tm *m_time;
        m_time = localtime(&stat_buf.st_mtime);
        printf("Date : %d/%d/%d - ", m_time->tm_year + 1900, m_time->tm_mon + 1, m_time->tm_mday);
        printf("%d:%d:%d\n", m_time->tm_hour, m_time->tm_min, m_time->tm_sec);

        // Get File Info (MD5 Hash)
        printf("Hash : ");
        uint8_t file_md5[16];
        FILE *fp;
        fp = fopen(argv[1], "r");
        fseek(fp, 0, SEEK_SET);
        md5File(fp, file_md5);
        for(uint8_t i = 0; i < 16; ++i){
            printf("%02X", file_md5[i]);
        }
        printf("\n");

        // Get File Info (Line Count)
        char header[1003];
        char line_buf[BUF_SIZE];
        uint32_t bitstream_size = 0;
        fseek(fp, 0, SEEK_SET);
        while (fgets(line_buf, BUF_SIZE, fp) != NULL) {
            if (strlen(line_buf)==3 && isalnum(line_buf[0]) && isalnum(line_buf[1])) {
                if (bitstream_size < 1000) {
                    header[bitstream_size] = strtol(line_buf, NULL, 16);
                }
                bitstream_size++;
            }
        }
        printf("Line : %d\n", bitstream_size);
        printf("==================================================\n");

        // Show File Header
        fseek(fp, 0, SEEK_SET);
        for (uint16_t i=0; i < 1000; i++) {
            printf("%c", header[i]);
            if (header[i] == 0x0A && header[i+1] == 0x0A && header[i+2] == 0x0A) {
                break;
            }
            if (header[i+1] == 0x16) {
                break;
            }
        }
        printf("==================================================\n");

        printf("File Generating... ");

        // Malloc
        uint8_t *bitstream;
        bitstream = (uint8_t *)malloc(sizeof(uint8_t) * bitstream_size);
        if (bitstream == NULL) {
            perror("Malloc");
            exit(EXIT_FAILURE);
        };
        fseek(fp, 0, SEEK_SET);
        for (uint32_t i=0; fgets(line_buf, BUF_SIZE, fp) != NULL; i++) {
            if (strlen(line_buf)==3 && isalnum(line_buf[0]) && isalnum(line_buf[1])) {
                bitstream[i] = strtol(line_buf, NULL, 16);
            }
        }

        // File Output
        FILE *fpo;
        fpo = fopen("Bitstream.h", "w");
        if (fpo == NULL) {
            perror("file_out");
            exit(EXIT_FAILURE);
        }


        fprintf(fpo, "#ifndef BITSTREAM_H\n#define BITSTREAM_H\n\n");

        uint16_t header_last_pos = 0;
        for (uint16_t i=0; i < 1000; i++) {
            if (header[i] == 0x0A) {
                if (i-header_last_pos > 1) {
                    strncpy(line_buf, &header[header_last_pos], i-header_last_pos);
                    line_buf[i-header_last_pos] = 0x00;
                    if (strspn(line_buf, "Version: ") == 9) {
                        fprintf(fpo, "    const String bitstream_version = \"%s\";\n", line_buf + 9);
                    }
                    if (strspn(line_buf, "Generated: ") == 11) {
                        fprintf(fpo, "    const String bitstream_date = \"%s\";\n", line_buf + 11);
                        char bs_wday[BUF_SIZE];
                        char bs_mon[BUF_SIZE];
                        uint16_t bs_year, bs_day, bs_hour, bs_min, bs_sec;
                        sscanf(line_buf, "Generated: %[a-zA-Z] %[a-zA-Z] %hd %hd:%hd:%hd %hd", bs_wday, bs_mon, &bs_day, &bs_hour, &bs_min, &bs_sec, &bs_year);
                        fprintf(fpo, "    const uint16_t bitstream_year = %d;\n", bs_year);
                        char *month[] = {"Jan","Feb", "Mar","Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
                        for (uint8_t j = 0; j < 12; j++) {
                            if (strcmp(month[j], bs_mon) == 0) {
                                fprintf(fpo, "    const uint16_t bitstream_mon = %d;\n", j + 1);
                            }
                        }
                        fprintf(fpo, "    const uint8_t bitstream_day = %d;\n", bs_day);
                        fprintf(fpo, "    const uint8_t bitstream_hour = %d;\n", bs_hour);
                        fprintf(fpo, "    const uint8_t bitstream_min = %d;\n", bs_min);
                        fprintf(fpo, "    const uint8_t bitstream_sec = %d;\n", bs_sec);
                    }
                    if (strspn(line_buf, "Project: ") == 9) {
                        fprintf(fpo, "    const String bitstream_project = \"%s\";\n", line_buf + 9);
                    }
                    if (strspn(line_buf, "Family: ") == 8) {
                        fprintf(fpo, "    const String bitstream_family = \"%s\";\n", line_buf + 8);
                    }
                    if (strspn(line_buf, "Device: ") == 8) {
                        fprintf(fpo, "    const String bitstream_device = \"%s\";\n", line_buf + 8);
                    }
                    if (strspn(line_buf, "Width: ") == 7) {
                        fprintf(fpo, "    const String bitstream_width = \"%s\";\n", line_buf + 7);
                    }
                    if (strspn(line_buf, "Mode: ") == 6) {
                        fprintf(fpo, "    const String bitstream_mode = \"%s\";\n", line_buf + 6);
                    }
                    if (strspn(line_buf, "PADDED_BITS: ") == 13) {
                        fprintf(fpo, "    const String bitstream_padded_bits = \"%s\";\n", line_buf + 13);
                    }

                }
                header_last_pos = i + 1;
            }
        }

        fprintf(fpo, "    const uint16_t bitstream_file_year = %d;\n", m_time->tm_year + 1900);
        fprintf(fpo, "    const uint8_t bitstream_file_mon = %d;\n", m_time->tm_mon + 1);
        fprintf(fpo, "    const uint8_t bitstream_file_day = %d;\n", m_time->tm_mday);
        fprintf(fpo, "    const uint8_t bitstream_file_hour = %d;\n", m_time->tm_hour);
        fprintf(fpo, "    const uint8_t bitstream_file_min = %d;\n", m_time->tm_min);
        fprintf(fpo, "    const uint8_t bitstream_file_sec = %d;\n", m_time->tm_sec);
        fprintf(fpo, "    const uint32_t bitstream_file_byte_size = %ld;\n", (long)stat_buf.st_size);
        fprintf(fpo, "    const uint8_t bitstream_file_md5[] = {");
        for(uint8_t i = 0; i < 16; ++i){
            if (i != 0) {
                fprintf(fpo, ", ");
            }
            fprintf(fpo, "0x%02X", file_md5[i]);
        }
        fprintf(fpo, "};\n");
        fprintf(fpo, "    const String bitstream_file_md5_str = \"");
        for(uint8_t i = 0; i < 16; ++i){
            fprintf(fpo, "%02X", file_md5[i]);
        }
        fprintf(fpo, "\";\n");
        fprintf(fpo, "    const uint32_t bitstream_byte_size = %d;\n", bitstream_size);
        fprintf(fpo, "    const uint8_t bitstream_data[] PROGMEM = {\n        ");
        for (uint32_t i=0; i < bitstream_size; i++) {
            if (i != 0) {
                fprintf(fpo, ", ");
                if (i % 100 == 0) {
                    fprintf(fpo, "\n        ");
                }
            }
            fprintf(fpo, "0x%02X", bitstream[i]);
        }
        fprintf(fpo, "\n    };\n");

        fprintf(fpo, "\n#endif\n\n");


        printf("Done!\n");
        printf("==================================================\n");

        // Close
        free(bitstream);
        fclose(fp);
        fclose(fpo);

    } else {
        perror("stat");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

