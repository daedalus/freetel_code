/*!
 * Callback driven menu handler.
 *
 * The following is an implementation of a callback-driven menu system.
 * It supports arbitrary levels of menus (limited by size of return stack)
 * and supports arbitrary user events.
 * 
 * Author Stuart Longland <me@vk4msl.id.au>
 * Copyright (C) 2015 FreeDV project.
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */

#include <stdlib.h>

#include "menu.h"

/*!
 * Return the Nth item on the stack.
 */
static const struct menu_stack_item_t *const menu_stack(const struct menu_t *const menu, uint8_t index)
{
	if (menu->stack_depth <= index)
		return NULL;

	return &(menu->stack[menu->stack_depth - index - 1]);
}

/*!
 * Return the Nth item on the stack.
 */
const struct menu_item_t *const menu_item(const struct menu_t *const menu, uint8_t index)
{
	const struct menu_stack_item_t *const current = menu_stack(menu, index);

	if (!current)
		return NULL;
        
	return current->item;
}

/*!
 * Enter a (sub)-menu.
 */
int menu_enter(struct menu_t *const menu, const struct menu_item_t *const item)
{
	if (menu->stack_depth == MENU_STACK_SZ)
		return -1;

	menu->stack[menu->stack_depth].item = item;
	menu->stack[menu->stack_depth].index = menu->current;
	menu->stack_depth++;

	(item->event_cb)(menu, MENU_EVT_ENTERED);

	return 0;
}

/*!
 * Return from a (sub)-menu.
 */
void menu_leave(struct menu_t *const menu)
{
	if (!menu->stack_depth)
		return;	/* Already out of the menu */

	menu->last = menu_item(menu, 0);
	menu->stack_depth--;

	const struct menu_stack_item_t *current = menu_stack(menu, 0);
        
	if (current && current->item) {
		menu->current = current->index;
		(current->item->event_cb)(menu, MENU_EVT_RETURNED);
	}
}

/*!
 * Execute the callback for the current item with a user-supplied event.
 */
void menu_exec(struct menu_t *const menu, uint32_t event)
{
	const struct menu_item_t *item = menu_item(menu, 0);
        
	if (item && item->event_cb)
		(item->event_cb)(menu, event);
}

