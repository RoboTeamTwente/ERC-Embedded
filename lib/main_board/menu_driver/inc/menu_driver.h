#ifndef MENU_DRIVER_H
#define MENU_DRIVER_H

#include "menu_driver_conf.h"
#include <stdbool.h>
#include <stdint.h>
#define MAX_CHILDREN 15
#define MAX_PAGES 128
#define MAX_PAGE_NAME_LEN 20

typedef unsigned char menu_input;

#define INPUT_DIRECTION_LEFT_FLAG 0
#define INPUT_DIRECTION_RIGHT_FLAG 1
#define INPUT_DIRECTION_TOP_FLAG 2
#define INPUT_DIRECTION_BOTTOM_FLAG 3
#define INPUT_BUTTON_PRESS_A_FLAG 4

#define IS_INPUT_DIRECTION_LEFT(x)                                             \
  (((x) & (1 << INPUT_DIRECTION_LEFT_FLAG)) != 0)

#define IS_INPUT_DIRECTION_RIGHT(x)                                            \
  (((x) & (1 << INPUT_DIRECTION_RIGHT_FLAG)) != 0)

#define IS_INPUT_DIRECTION_TOP(x) (((x) & (1 << INPUT_DIRECTION_TOP_FLAG)) != 0)

#define IS_INPUT_DIRECTION_BOTTOM(x)                                           \
  (((x) & (1 << INPUT_DIRECTION_BOTTOM_FLAG)) != 0)

#define IS_INPUT_BUTTON_PRESS_A(x)                                             \
  (((x) & (1 << INPUT_BUTTON_PRESS_A_FLAG)) != 0)

// --- Set Macros (SET_...) ---
// Sets a specific bit to 1.

#define SET_INPUT_DIRECTION_LEFT(x) ((x) |= (1 << INPUT_DIRECTION_LEFT_FLAG))

#define SET_INPUT_DIRECTION_RIGHT(x) ((x) |= (1 << INPUT_DIRECTION_RIGHT_FLAG))

#define SET_INPUT_DIRECTION_TOP(x) ((x) |= (1 << INPUT_DIRECTION_TOP_FLAG))

#define SET_INPUT_DIRECTION_BOTTOM(x)                                          \
  ((x) |= (1 << INPUT_DIRECTION_BOTTOM_FLAG))

#define SET_INPUT_BUTTON_PRESS_A(x) ((x) |= (1 << INPUT_BUTTON_PRESS_A_FLAG))

// --- Clear Macros (CLEAR_...) ---
// Sets a specific bit to 0.

#define CLEAR_INPUT_DIRECTION_LEFT(x) ((x) &= ~(1 << INPUT_DIRECTION_LEFT_FLAG))

#define CLEAR_INPUT_DIRECTION_RIGHT(x)                                         \
  ((x) &= ~(1 << INPUT_DIRECTION_RIGHT_FLAG))

#define CLEAR_INPUT_DIRECTION_TOP(x) ((x) &= ~(1 << INPUT_DIRECTION_TOP_FLAG))

#define CLEAR_INPUT_DIRECTION_BOTTOM(x)                                        \
  ((x) &= ~(1 << INPUT_DIRECTION_BOTTOM_FLAG))

#define CLEAR_INPUT_BUTTON_PRESS_A(x) ((x) &= ~(1 << INPUT_BUTTON_PRESS_A_FLAG))

struct menu_page_t;
struct menu_manager_t;

typedef struct {
  uint8_t num_entries;
  uint8_t selected_index;
  uint8_t entry_ids[MAX_LIST_ENTRIES];
  unsigned char entry_icons[MAX_LIST_ENTRIES][MENU_LIST_ICON_INTEGER_SIZE];
} page_list_state;

typedef union {
  page_list_state list;

} menu_page_state;
/**
 */
typedef struct {
  void *state;
  unsigned char id;
  unsigned char parent_id;
  bool needs_render; // whether the page needs to be re-rendered

  char name[MAX_PAGE_NAME_LEN];

  void (*init)(void *state);
  void (*update)(struct menu_manager_t *manager);
  void (*render)(struct menu_manager_t *manager);
  void (*destruct)(void *state);
} menu_page_t;

typedef struct {
  unsigned char active_page_id;
  menu_page_t pages[MAX_PAGES];
  menu_input (*get_input)(void);
} menu_manager_t;

void menu_manager_init(menu_manager_t *manager,
                       menu_input (*get_input_func)(void));
void menu_manager_switch_page(menu_manager_t *manager,
                              unsigned char new_page_id);
#endif // !MENU_DRIVER_H
