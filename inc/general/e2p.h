#ifndef  __E2P_H
#define  __E2P_H

#include <stdint.h>

extern uint32_t e2p_write_page(uint32_t address, uint8_t *data, uint32_t leg);
extern uint32_t e2p_read_page(uint32_t address, uint8_t *data, uint32_t leg);


#endif
