/*
 * STM32F4 Virtual EEPROM driver
 *
 * This module implements a crude virtual EEPROM device stored in on-board
 * flash.  The STM32F405 has 4 16kB flash sectors starting at address
 * 0x80000000, followed by a 64kB sector, then 128kB sectors.
 *
 * The Cortex M4 core maps these all to address 0x00000000 when booting
 * from normal flash, so the first sector is reserved for interrupt
 * vectors.
 *
 * Everything else however is free game, and so we use these smaller
 * sectors to store our configuration.
 *
 * Author Stuart Longland <me@vk4msl.id.au>
 * Copyright (C) 2015 FreeDV project.
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */

#pragma once

#include <stdint.h>
#include <errno.h>

/*
 * Read data from a virtual EEPROM.
 * @param	rom		ROM ID to start reading.
 * @param	offset		Address offset into ROM to start reading.
 * @param	size		Number of bytes to read from ROM.
 * @param	out		Buffer to write ROM content to.
 * @returns			Number of bytes read from ROM.
 * @retval	-ENXIO		No valid data found for address.
 * @retval	-ESPIPE		Offset past end of ROM.
 */
int vrom_read(uint8_t, uint16_t, uint16_t, void *);

/*
 * Write data to a virtual EEPROM.
 * @param	rom		ROM ID to start writing.
 * @param	offset		Address offset into ROM to start writing.
 * @param	size		Number of bytes to write to ROM.
 * @param	in		Buffer to write ROM content from.
 * @returns			Number of bytes written to ROM.
 * @retval	-EIO		Programming failed
 * @retval	-ENOSPC		No free blocks available
 */
int vrom_write(uint8_t, uint16_t, uint16_t, const void *);

/*
 * Erase a virtual EEPROM.
 * @param	rom		ROM ID to erase.
 * @returns			Number of bytes written to ROM.
 * @retval	-EIO		Programming failed
 * @retval	-ENOSPC		No free blocks available
 */
int vrom_erase(uint8_t);

