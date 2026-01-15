#ifndef MENU_DRIVER_H
#define MENU_DRIVER_H

#include "kv_pool.h"
#include "menu_driver_conf.h"
#include "menu_driver_icons.h"
#include <stdbool.h>
#include <stdint.h>

#define MAX_CHILDREN 15
#define MAX_PAGES 128
#define MAX_PAGE_NAME_LEN 20

typedef unsigned char menu_input;
struct menu_page_t;
struct menu_manager_t;

typedef struct {
  uint8_t num_entries;
  uint8_t selected_index;
  uint8_t entry_ids[MAX_LIST_ENTRIES];
  const uint8_t (*entry_icons)[MENU_DRIVER_ICON_BYTE_SIZE];
} page_list_state;

typedef struct {
  uint8_t num_entries;
  char entry_titles[MENU_OVERVIEW_MAX_ENTRIES]
                   [MENU_OVERVIEW_MAX_ENTRY_TITLE_LEN];
  uint8_t entry_lookups[MENU_OVERVIEW_MAX_ENTRIES];
  kv_pool *kv_pool;
  bool err_sprite_state;
  uint8_t last_states[MENU_OVERVIEW_MAX_ENTRIES];
} page_overview_state;

typedef union {
  page_list_state list;
  page_overview_state overview;

} menu_page_state;
typedef enum {
  MENU_PAGE_TYPE_LIST,
  MENU_PAGE_TYPE_OVERVIEW,
} menu_page_type_t;
/**
 */
typedef struct {
  menu_page_state *state;
  menu_page_type_t type;
  unsigned char id;
  unsigned char parent_id;
  bool needs_render; // whether the page needs to be re-rendered

  char name[MAX_PAGE_NAME_LEN];

  void (*init)(menu_page_state *state);
  void (*update)(struct menu_manager_t *manager);
  void (*render)(struct menu_manager_t *manager);
  void (*destruct)(menu_page_state *state);
} menu_page_t;

typedef struct {
  unsigned char active_page_id;
  const menu_page_t *pages;
  menu_input (*get_input)(void);
} menu_manager_t;

void menu_manager_init(menu_manager_t *manager,
                       menu_input (*get_input_func)(void));
void menu_manager_switch_page(menu_manager_t *manager,
                              unsigned char new_page_id);
#endif // !MENU_DRIVER_H
