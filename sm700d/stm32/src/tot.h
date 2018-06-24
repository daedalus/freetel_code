/*
 * Time-out timer.
 *
 * This is a simple time-out timer for ensuring a maximum transmission
 * time is observed.  The time-out timer is configured with a total time
 * in "ticks", which get counted down in an interrupt.
 *
 * When the "warning" level is reached, a flag is repeatedly set permit
 * triggering of LEDs/sounds to warn the user that time is nearly up.
 *
 * Upon timeout, a separate flag is set to indicate timeout has taken
 * place.
 *
 * Author Stuart Longland <me@vk4msl.id.au>
 * Copyright (C) 2015 FreeDV project.
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */

#pragma once

#include <stdint.h>

/*
 * Time-out timer state machine
 */
struct tot_t
{
	/*
	 * Number of ticks remaining, if non-zero, transmission is
	 * in progress.
	 */
	uint32_t	remaining;

	/*
	 * Number of ticks remaining, before next warning.
	 */
	uint32_t	warn_remain;

	/*
	 * Timeout timer tick period.  Used to reset the ticks counter.
	 */
	uint32_t	tick_period;

	/*
	 * Number of ticks between the remaining warnings.
	 */
	uint16_t	remain_warn_ticks;

	/*
	 * Event tick timer.  Used to slow down the source timer.
	 */
	uint16_t	ticks;

	/*
	 * Event flags.
	 */
	uint16_t	event;
};

/*
 * Time-out timer has been started.
 */
#define	TOT_EVT_START		(1 << 0)

/*
 * Start of warning period reached.
 */
#define	TOT_EVT_WARN		(1 << 1)

/*
 * Next warning is due.
 */
#define TOT_EVT_WARN_NEXT	(1 << 2)

/*
 * Time-out reached.
 */
#define TOT_EVT_TIMEOUT		(1 << 3)

/*
 * Timer sequence complete
 */
#define TOT_EVT_DONE		(1 << 4)

/*
 * Reset the time-out timer.  This zeroes the counter and event flags.
 */
void tot_reset(struct tot_t *const);

/*
 * Start the time-out timer ticking.
 */
void tot_start(struct tot_t *const, uint32_t, uint16_t);

/*
 * Count a time-out timer tick.
 */
static inline void tot_tick(struct tot_t *const tot)
{
	if (tot->ticks)
		tot->ticks--;
}

/*
 * Update the time-out timer state.
 */
void tot_update(struct tot_t *const);

