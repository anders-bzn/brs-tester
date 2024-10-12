/*
 * BRS-tester by Anders Sandahl 2023-2024
 *
 * License GPL 2.0
 *
 */

#define VECTOR_LENGTH 37

struct config {
    char    pin_def[VECTOR_LENGTH];
    int     output_drive_strength;
    float   output_voltage_high;
    float   output_voltage_margin;
    float   load_low;
    float   load_low_margin;
    float   load_high;
    float   load_high_margin;
    float   load_current;
    float   load_current_margin;
    float   input_current_high;
    float   input_current_low;
    float   input_current_margin;
    float   input_voltage_high;
    float   input_voltage_low;
    int     toggles;
};

enum type {
    TYPE_LOGIC,
    TYPE_OUTPUT,
    TYPE_DEBUG_EXIT
};

struct vector {
    char *vector;
    enum type type;
};

extern struct vector vectors[];

int vector_loadVectors(char *filename, struct config *board);
void vector_freeVectors(void);
void vector_initVectors(void);
struct config *vector_allocConfig(void);
