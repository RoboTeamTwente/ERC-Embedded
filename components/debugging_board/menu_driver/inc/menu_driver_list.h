#ifndef MENU_DRIVER_LIST_H
#define MENU_DRIVER_LIST_H
#include "menu_driver.h"
#include "menu_driver_conf.h"
#include "menu_driver_icons.h"
#include "result.h"
#include <stdint.h>

#define MENU_LIST_ITEM_X 50
#define MENU_LIST_ITEM_GAP 12
#define MENU_LIST_ITEM_HEIGHT 64
#define MENU_LIST_ITEM_WIDTH 250

result_t get_menu_page_list(
    uint8_t id, uint8_t parent_id, uint8_t num_entries,
    uint8_t entry_ids[MAX_LIST_ENTRIES],
    unsigned char entry_icons[MAX_LIST_ENTRIES][MENU_DRIVER_ICON_BYTE_SIZE],
    char *name, menu_page_state *state_ptr, menu_page_t *out_page);

void menu_page_render_list(struct menu_manager_t *manager);
void menu_page_update_list(struct menu_manager_t *manager);
void menu_page_init_list(menu_page_state *state_ptr);
void menu_page_destruct_list(menu_page_state *state_ptr);

#endif // !MENU_DRIVER_LIST_H
