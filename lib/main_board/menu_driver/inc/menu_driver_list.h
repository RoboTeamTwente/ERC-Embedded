#ifndef MENU_DRIVER_LIST_H
#define MENU_DRIVER_LIST_H
#include "menu_driver.h"
#include "menu_driver_conf.h"
#include "result.h"
#include <stdint.h>
result_t get_menu_page_list(
    uint8_t id, uint8_t parent_id, uint8_t num_entries,
    uint8_t entry_ids[MAX_LIST_ENTRIES],
    unsigned char entry_icons[MAX_LIST_ENTRIES][MENU_LIST_ICON_INTEGER_SIZE],
    char *name, page_list_state *state_ptr, menu_page_t *out_page);

void menu_page_render_list(struct menu_manager_t *manager);
void menu_page_update_list(struct menu_manager_t *manager);
void menu_page_init_list(void *state_ptr);
void menu_page_destruct_list(void *state_ptr);

#endif // !MENU_DRIVER_LIST_H
