/*
 * Sound effect player library.
 *
 * This implements a state machine for playing back various monophonic
 * sound effects such as morse code symbols, clicks and alert tones.
 *
 * Author Stuart Longland <me@vk4msl.id.au>
 * Copyright (C) 2015 FreeDV project.
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */

#pragma once

#include <stdint.h>

#include "tone.h"

/*
 * A sound effect "note"
 */
struct sfx_note_t
{
    /* Note frequency.  0 == pause */
    uint16_t freq;

    /* Note duration in msec.   0 == end of effect */
    uint16_t duration;
};

/*
 * Sound effect player state machine
 */
struct sfx_player_t
{
    /*
     * Pointer to the current "note".  When this is NULL,
     * playback is complete.
     */
    const struct sfx_note_t *note;

    /* Tone generator state machine */
    struct tone_gen_t tone_gen;
};

/*
 * Start playing a particular effect.
 * @param	sfx_player	Effect player state machine
 * @param	effect		Pointer to sound effect (NULL == stop)
 */
void sfx_play(struct sfx_player_t *const, const struct sfx_note_t *);

/*
 * Retrieve the next sample to be played.
 */
int16_t sfx_next(struct sfx_player_t *const);

