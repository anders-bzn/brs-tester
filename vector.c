/*
 * BRS-tester by Anders Sandahl 2023-2024
 *
 * License GPL 2.0
 *
 * vector.c: Parser for test vectors
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"

char *vectors[1000];

enum check_format {
    FORMAT_CONFIG,
    FORMAT_VECTOR,
    FORMAT_LOAD
};

static int validate_format(char *str, char *cfg, enum check_format format)
{
    int numTogglePins = 0;
    /*
     * Check string for correct length, expect a trailing "'"
     */
    if (VECTOR_LENGTH + 1 != strlen(str)) {
        return -1;
    }
    /*
     * Check vector string. Only allow "-01T". T for Toggle, and only "-" on power
     * pin location. Toggle one pin only.
     */
    if (format == FORMAT_VECTOR){
        for(int i=0; i < VECTOR_LENGTH - 1; i++){
            switch (str[i]) {
                case '0':
                case '1':
                case 'T':
                    if (i==0 || i==1 || i==2 || i==18 || i==19 || i==20) {
                        return -1;
                    }

                    if (str[i] == 'T' && ++numTogglePins > 1){
                        /* Can only toggle one pin at a time (currently). */
                        return -1;
                    }
                case '-':
                    break;
                default:
                    return -1;
                    break;

            }
        }
    }

    /*
     * Check vector string. Only allow "-01". And only "-" on power
     * pin location.
     *
     * 'p' - power pin, do nothing
     * 'i' - input pin on tested board
     * 'o' - output pin on tested board
     * 'O' - output pin on tested board, open collector
     * 'd' - pull down net on tested board
     * 'g' - pin should be grounded on tested board
     * '-' - pin should not be used
     */

    if (format == FORMAT_CONFIG){
        for(int i=0; i < VECTOR_LENGTH - 1; i++){
            switch (str[i]) {
                case 'i':
                case 'o':
                case 'O':
                case 'd':
                case 'g':
                    if (i==0 || i==1 || i==2 || i==18 || i==19 || i==20) {
                        return -1;
                    }
                    break;
                case 'p':
                    if (i!=0 && i!=1 && i!=2 && i!=18 && i!=19 && i!=20) {
                        return -1;
                    }
                    break;
                case '-':
                    break;
                default:
                    return -1;
                    break;
            }
        }
    }

    /*
     * Check vector string. Only allow "L" on outputs ('O' and 'o'). And only "-" on other pins
     * pin location.
     *
     * 'p' - power pin, do nothing
     * 'i' - input pin on tested board
     * 'o' - output pin on tested board
     * 'O' - output pin on tested board, open collector
     * 'd' - pull down net on tested board
     * 'g' - pin should be grounded on tested board
     * '-' - pin should not be used
     */

    if (format == FORMAT_LOAD){
        if (cfg == NULL)
            return -1;

        for(int i=0; i < VECTOR_LENGTH - 1; i++){
            if (str[i] == '-')
                continue;;

            if (str[i] == 'L' && (cfg[i] == 'o' || cfg[i] == 'O'))
                continue;

            return -1;
        }
    }
    return 0;
}

int vector_loadVectors(char *filename, struct config *board)
{
    char str[200];
    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {
        printf("ERROR: could not open file '%s'\n", filename);
        return -1;
    }

    int k=0, i=0;
    while (!feof(fp)) {
        if (!fgets(str, 200, fp))
            break;
        i++;

        /*
         * Don't care about comments and newlines
         */
        if (str[0] == '#' || str[0] == '\n')
            continue;

        if (0 == strncmp("config='", str, 8)){
            if (0 > validate_format(&str[8], NULL, FORMAT_CONFIG)){
                printf("ERROR: Error parse file line %d: %s\n", i, str);
                return -1;
            }
            /*
             * Break if error
             */
            strncpy(board->pin_def, &str[8], VECTOR_LENGTH);
            board->pin_def[VECTOR_LENGTH-1] = '\0';
        } else if (0 == strncmp("vector='", str, 8)){
            if (0 > validate_format(&str[8], NULL, FORMAT_VECTOR)){
                printf("ERROR: Error parse file line %d: %s\n", i, str);
                return -1;
            }
            /*
             * Break if error
             */
            vectors[k] = malloc(VECTOR_LENGTH);
            strncpy(vectors[k], &str[8], VECTOR_LENGTH);
            vectors[k][VECTOR_LENGTH-1] = '\0';
            k++;
            vectors[k] = NULL;
        } else if (0 == strncmp("load=  '", str, 8)){
            if (0 > validate_format(&str[8], board->pin_def, FORMAT_LOAD)){
                printf("ERROR: Error parse file line %d: %s\n", i, str);
                return -1;
            }
            /*
             * Break if error
             */

            printf("LOAD string OK %s", &str[8]);
        } else if (0 == strncmp("load-low='", str, sizeof("load-low='")-1)) {
            float value = 0;
            int k = sscanf(str, "load-low='%fmV'", &value);
            if (k == 1) {
                board->load_low = value;
            } else {
                printf("ERROR: Error parse file line %d: %s\n", i, str);
                return -1;
            }
        } else if (0 == strncmp("load-low-margin='", str, sizeof("load-low-margin='")-1)) {
            float value = 0;
            int k = sscanf(str, "load-low-margin='%fmV'", &value);
            if (k == 1) {
                board->load_low_margin = value;
            } else {
                printf("ERROR: Error parse file line %d: %s\n", i, str);
                return -1;
            }
        } else if (0 == strncmp("load-high='", str, sizeof("load-high='")-1)) {
            float value = 0;
            int k = sscanf(str, "load-high='%fmV'", &value);
            if (k == 1) {
                board->load_high = value;
            } else {
                printf("ERROR: Error parse file line %d: %s\n", i, str);
                return -1;
            }
        } else if (0 == strncmp("load-high-margin='", str, sizeof("load-high-margin='")-1)) {
            float value = 0;
            int k = sscanf(str, "load-high-margin='%fmV'", &value);
            if (k == 1) {
                board->load_high_margin = value;
            } else {
                printf("ERROR: Error parse file line %d: %s\n", i, str);
                return -1;
            }
        } else if (0 == strncmp("load-current='", str, sizeof("load-current='")-1)) {
            float value = 0;
            int k = sscanf(str, "load-current='%fmA'", &value);
            if (k == 1) {
                board->load_current = value;
            } else {
                printf("ERROR: Error parse file line %d: %s\n", i, str);
                return -1;
            }
        } else if (0 == strncmp("load-current-margin='", str, sizeof("load-current-margin='")-1)) {
            float value = 0;
            int k = sscanf(str, "load-current-margin='%fmA'", &value);
            if (k == 1) {
                board->load_current_margin = value;
            } else {
                printf("ERROR: Error parse file line %d: %s\n", i, str);
                return -1;
            }
        } else if (0 == strncmp("input-current-low='", str, sizeof("input-current-low='")-1)) {
            float value = 0;
            int k = sscanf(str, "input-current-low='%fmA'", &value);
            if (k == 1) {
                board->input_current_low = value;
            } else {
                printf("ERROR: Error parse file line %d: %s\n", i, str);
                return -1;
            }
        } else if (0 == strncmp("input-current-high='", str, sizeof("input-current-high='")-1)) {
            float value = 0;
            int k = sscanf(str, "input-current-high='%fmA'", &value);
            if (k == 1) {
                board->input_current_high = value;
            } else {
                printf("ERROR: Error parse file line %d: %s\n", i, str);
                return -1;
            }
        } else if (0 == strncmp("input-current-margin='", str, sizeof("input-current-margin='")-1)) {
            float value = 0;
            int k = sscanf(str, "input-current-margin='%fmA'", &value);
            if (k == 1) {
                board->input_current_margin = value;
            } else {
                printf("ERROR: Error parse file line %d: %s\n", i, str);
                return -1;
            }
        } else if (0 == strncmp("input-voltage-low='", str, sizeof("input-voltage-low='")-1)) {
            float value = 0;
            int k = sscanf(str, "input-voltage-low='%fmV'", &value);
            if (k == 1) {
                board->input_voltage_low = value;
            } else {
                printf("ERROR: Error parse file line %d: %s\n", i, str);
                return -1;
            }
        } else if (0 == strncmp("input-voltage-high='", str, sizeof("input-voltage-high='")-1)) {
            float value = 0;
            int k = sscanf(str, "input-voltage-high='%fmV'", &value);
            if (k == 1) {
                board->input_voltage_high = value;
            } else {
                printf("ERROR: Error parse file line %d: %s\n", i, str);
                return -1;
            }
        } else if (0 == strncmp("output-drive-strength='", str, sizeof("output-drive-strength='")-1)) {
            int k, current = 0;
            k = sscanf(str, "output-drive-strength='%dmA'", &current);
            if (k == 1 && current >= 0 && current < 127) {
                board->output_drive_strength = current;
            } else {
                printf("ERROR: Error parse file line %d: %s\n", i, str);
                return -1;
            }
        } else if (0 == strncmp("toggles='", str, sizeof("toggles='")-1)) {
            int k, toggles = 0;
            k = sscanf(str, "toggles='%d'", &toggles);
            if (k == 1) {
                board->toggles = toggles;
            } else {
                printf("ERROR: Error parse file line %d: %s\n", i, str);
                return -1;
            }
        } else {
            printf("ERROR: Error parse file line %d: %s\n", i, str);
            return -1;
        }
    }
    fclose(fp);
    return 0;
}


void vector_freeVectors(void)
{
    int k=0;
    while(vectors[k] != NULL) {
        free(vectors[k]);
        vectors[k]  = NULL;
        k++;
    }
}


void vector_initVectors(void)
{
    memset(vectors, 0, sizeof(vectors[1000]));
}


/*
 * Allocate a config struct and initialize it
 */
struct config *vector_allocConfig(void)
{
    struct config *b_cfg = malloc(sizeof(struct config));
    memset(b_cfg->pin_def, 0, sizeof(char) * VECTOR_LENGTH);
    b_cfg->output_drive_strength = 0;
    b_cfg->load_low = 0;
    b_cfg->load_low_margin = 0;
    b_cfg->load_high = 0;
    b_cfg->load_high_margin = 0;
    b_cfg->load_current = 0;
    b_cfg->load_current_margin = 0;
    b_cfg->input_current_low = 0;
    b_cfg->input_current_high = 0;
    b_cfg->input_current_margin = 0;
    b_cfg->input_voltage_low = 0;
    b_cfg->input_voltage_high = 0;
    b_cfg->toggles = 0;
    return b_cfg;
}
