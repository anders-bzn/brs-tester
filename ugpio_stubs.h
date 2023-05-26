#ifndef UGPIO_H
#define UGPIO_H

#include <stddef.h>

/* taken from linux/gpio.h */
#define GPIOF_DIR_OUT                (0 << 0)
#define GPIOF_DIR_IN                 (1 << 0)

#define GPIOF_INIT_LOW               (0 << 1)
#define GPIOF_INIT_HIGH              (1 << 1)

#define GPIOF_IN                     (GPIOF_DIR_IN)
#define GPIOF_OUT_INIT_LOW           (GPIOF_DIR_OUT | GPIOF_INIT_LOW)
#define GPIOF_OUT_INIT_HIGH          (GPIOF_DIR_OUT | GPIOF_INIT_HIGH)

#define GPIOF_TRIG_FALL              (1 << 2)
#define GPIOF_TRIG_RISE              (1 << 3)
#define GPIOF_TRIGGER_MASK           (GPIOF_TRIG_FALL | GPIOF_TRIG_RISE)

#define GPIOF_REQUESTED              (1 << 4)
#define GPIOF_CLOEXEC                (1 << 5)
#define GPIOF_ALTERABLE_DIRECTION    (1 << 6)
#define GPIOF_DIRECTION_UNKNOWN      (1 << 7)
#define GPIOF_ALTERABLE_EDGE         (1 << 8)

struct gpio;
typedef struct gpio ugpio_t;

ugpio_t *ugpio_request_one(unsigned int gpio, unsigned int flags, const char *label)
{
	/* Its ugly to return something here, but the value is only passed back
	 * to be used by those functions (which they don't).
	 */ 
	return (ugpio_t *)1;
}

void ugpio_free(ugpio_t *ctx)
{
	return;
}

int ugpio_full_open(ugpio_t *ctx)
{
	return 0;
}

void ugpio_close(ugpio_t *ctx)
{
	return;
}

int ugpio_get_value(ugpio_t *ctx)
{
	return 0;
}

int ugpio_set_value(ugpio_t *ctx, int value)
{
	return 0;
}

int ugpio_set_activelow(ugpio_t *ctx, int flag)
{
	return 0;
}

int ugpio_direction_input(ugpio_t *ctx)
{
	return 0;
}

int ugpio_direction_output(ugpio_t *ctx, int value)
{
	return 0;
}


#endif  /* UGPIO_H */
