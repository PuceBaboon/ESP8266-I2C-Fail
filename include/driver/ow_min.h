/*
 * $Id: ow_min.h,v 1.1.1.1 2015/07/09 03:23:32 anoncvs Exp $
 */

/* Functions */
void ICACHE_FLASH_ATTR ds_init(int gpio);
void ICACHE_FLASH_ATTR ds_reset(void);
static inline void write_bit (int v);
static inline int read_bit(void);
void ICACHE_FLASH_ATTR ds_write(uint8_t v, int power);
uint8_t ICACHE_FLASH_ATTR ds_read();

// void ICACHE_FLASH_ATTR temperature_cb(void *arg);

static int gpioPin;

/*
 * One-Wire ROM Commands.
 */
#define SEARCH_ROM	0xF0
#define READ_ROM	0x33
#define MATCH_ROM	0x55
#define SKIP_ROM	0xCC
#define ALARM_SEARCH	0xEC


/*
 * One-Wire Device Specific Function Commands.
 */
#define T_CONVERT	0x44	// Temperature convert.
#define WRITE_SPAD	0x4E	// Write device scratch-pad memory.
#define READ_SPAD	0xBE	// Read device scratch-pad memory.
#define COPY_SPAD	0x48	// Copy contents of device scratch-pad to device EEPROM.
#define RECALL_SPAD	0xB8	// Recall device EEPROM contents back into scratch-pad memory.
#define READ_PSU	0xB4	// Read device power requirements.
