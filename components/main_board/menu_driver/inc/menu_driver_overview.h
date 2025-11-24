#ifndef MENU_DRIVER_OVERVIEW_H
#define MENU_DRIVER_OVERVIEW_H
#include "kv_pool.h"
#include "menu_driver.h"
#include "menu_driver_conf.h"
#include "result.h"

result_t
get_menu_page_overview(uint8_t id, uint8_t parent_id,
                       char name[MAX_PAGE_NAME_LEN], uint8_t num_entries,
                       char entries_titles[MENU_OVERVIEW_MAX_ENTRIES]
                                          [MENU_OVERVIEW_MAX_ENTRY_TITLE_LEN],
                       uint8_t entry_lookups[MENU_OVERVIEW_MAX_ENTRIES],
                       kv_pool *kv_pool, page_overview_state *state_ptr,
                       menu_page_t *out_page);

void menu_page_render_overview(struct menu_manager_t *manager);
void menu_page_update_overview(struct menu_manager_t *manager);
void menu_page_init_overview(void *state_ptr);
void menu_page_destruct_overview(void *state_ptr);

#endif // !MENU_DRIVER_OVERVIEW_H
