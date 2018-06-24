/*!
 * Sound effect library.
 *
 * This provides some sound effects for the Hardware UI.
 *
 * Author Stuart Longland <me@vk4msl.id.au>
 * Copyright (C) 2015 FreeDV project.
 *
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */

#include "sounds.h"

const struct sfx_note_t sound_startup[] = {
	{.freq = 600, .duration = 80},
	{.freq = 800, .duration = 80},
	{.freq = 1000, .duration = 80},
	{.freq = 0, .duration = 0}
};

const struct sfx_note_t sound_returned[] = {
	{.freq = 1000, .duration = 80},
	{.freq = 800, .duration = 80},
	{.freq = 600, .duration = 80},
	{.freq = 0, .duration = 0}
};

const struct sfx_note_t sound_click[] = {
	{.freq = 1200, .duration = 10},
	{.freq = 0, .duration = 0}
};

const struct sfx_note_t sound_death_march[] = {
	{.freq  = 340, 	.duration = 400},
	{.freq	= 0,	.duration = 80},
	{.freq  = 340, 	.duration = 400},
	{.freq	= 0,	.duration = 80},
	{.freq  = 340, 	.duration = 400},
	{.freq	= 0,	.duration = 80},
	{.freq  = 420, 	.duration = 400},
	{.freq	= 0,	.duration = 80},
	{.freq  = 400, 	.duration = 300},
	{.freq	= 0,	.duration = 80},
	{.freq  = 340, 	.duration = 120},
	{.freq	= 0,	.duration = 80},
	{.freq  = 340, 	.duration = 120},
	{.freq	= 0,	.duration = 80},
	{.freq  = 300, 	.duration = 200},
	{.freq	= 0,	.duration = 80},
	{.freq  = 340, 	.duration = 400},
	{.freq	= 0, 	.duration = 0},
};

