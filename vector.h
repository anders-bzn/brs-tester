#define VECTOR_LENGTH 37

struct config {
	char	pin_def[VECTOR_LENGTH];
	int	output_drive_strenght;
	float	load_low;
	float	load_low_margin;
	float	load_high;
	float	load_high_margin;
	float	load_current;
	float	load_current_margin;
	float	input_current;
	float	input_current_margin;
	int	input_active_level;
	float	input_logic_high;
	int	toggles;
};

extern char *vectors[];

int vector_loadVectors(char *filename, struct config *board);
void vector_freeVectors(void);
void vector_initVectors(void);
struct config *vector_allocConfig(void);
