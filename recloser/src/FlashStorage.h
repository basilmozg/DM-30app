#ifndef FLASHSTORAGE_H__
#define FLASHSTORAGE_H__

#include <stdint.h>
#include <inttypes.h>

/*STORAGE SETTINGS*/
#ifdef CONFIG_SOC_NRF52840
#define start_address_flash             0x000F8000     //last kb of NRF52840 SoC flash
#endif
#ifdef CONFIG_SOC_NRF52833
#define start_address_flash             0x0007F000     //last kb of NRF52833 SoC flash
#endif
#define count_bytes_flash               0x1000

void FlashStorageRead(int16_t *reg_03);

void storage_save(uint8_t *eeprom_write);

#endif