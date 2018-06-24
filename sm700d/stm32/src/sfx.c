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

#include <stdlib.h>

#include "sfx.h"

static void sfx_next_tone(struct sfx_player_t *const sfx_player) {
    struct tone_gen_t *tone_gen = &(sfx_player->tone_gen);
    const struct sfx_note_t *note = sfx_player->note;

    if (note == NULL) {
        tone_reset(tone_gen, 0, 0);
    } else {
        tone_reset(tone_gen, note->freq, note->duration);

        if (note->duration == 0)
            /* We are done */
            sfx_player->note = NULL;
        else
            /* Move to next note */
            sfx_player->note++;
    }
}

/*
 * Start playing a particular effect.
 * @param	sfx_player	Effect player state machine
 * @param	effect		Pointer to sound effect (NULL == stop)
 */
void sfx_play(struct sfx_player_t *const sfx_player, const struct sfx_note_t *effect) {
    sfx_player->note = effect;
    sfx_next_tone(sfx_player);
}

/*!
 * Retrieve the next sample to be played.
 */
int16_t sfx_next(struct sfx_player_t *const sfx_player) {
    if (sfx_player == NULL)
        return 0;
    
    if (sfx_player->tone_gen.remain == 0)
        sfx_next_tone(sfx_player);
    
    return tone_next(&(sfx_player->tone_gen));
}

