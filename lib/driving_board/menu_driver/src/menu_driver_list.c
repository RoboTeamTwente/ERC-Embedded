#include "menu_driver_list.h"
#include "logging.h"
#include "menu_driver.h"
#include "result.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
static char *TAG = "MENU DRIVER LIST";

result_t get_menu_page_list(
    uint8_t id, uint8_t parent_id, uint8_t num_entries,
    uint8_t entry_ids[MAX_LIST_ENTRIES],
    unsigned char entry_icons[MAX_LIST_ENTRIES][MENU_LIST_ICON_INTEGER_SIZE],
    char name[MAX_PAGE_NAME_LEN], page_list_state *state_ptr,
    menu_page_t *out_page) {
  if (num_entries > MAX_LIST_ENTRIES || out_page == NULL || state_ptr == NULL ||
      name == NULL) {
    out_page = NULL;
    return RESULT_ERR_INVALID_ARG;
  }
  // Populate state
  state_ptr->num_entries = num_entries;
  state_ptr->selected_index = 0;
  for (int i = 0; i < num_entries; i++) {
    state_ptr->entry_ids[i] = entry_ids[i];
    memcpy(state_ptr->entry_icons[i], entry_icons[i],
           MENU_LIST_ICON_INTEGER_SIZE * sizeof(int));
  }
  // Populate page
  out_page->id = id;
  out_page->init = menu_page_init_list;
  out_page->update = menu_page_update_list;
  out_page->render = menu_page_render_list;
  out_page->destruct = menu_page_destruct_list;
  out_page->state = (void *)state_ptr;
  strncpy(out_page->name, name, MAX_PAGE_NAME_LEN);
  return RESULT_OK;
}

void menu_page_init_list(void *state_ptr) {
  // Nothing needs to be done in INIT
}

void menu_page_update_list(struct menu_manager_t *manager_str) {
  // Needed to go out of struct menu_manager_t to avoid circular dependency
  menu_manager_t *manager = (menu_manager_t *)manager_str;
  menu_input input = manager->get_input();
  menu_page_t *active_page = &manager->pages[manager->active_page_id];
  page_list_state *state = (page_list_state *)active_page->state;

  if (IS_INPUT_DIRECTION_LEFT(input)) {
    // Go to parent page
    menu_manager_switch_page(manager, active_page->parent_id);
  } else if (IS_INPUT_DIRECTION_RIGHT(input) ||
             IS_INPUT_BUTTON_PRESS_A(input)) {
    // Go to selected child page
    uint8_t selected_id = state->entry_ids[state->selected_index];
    menu_manager_switch_page(manager, selected_id);
  } else if (IS_INPUT_DIRECTION_TOP(input)) {
    // Move selection up
    if (state->selected_index == 0) {
      state->selected_index = state->num_entries - 1;
    } else {
      state->selected_index--;
    }
    active_page->needs_render = true;
  } else if (IS_INPUT_DIRECTION_BOTTOM(input)) {
    // Move selection down
    if (state->selected_index == state->num_entries - 1) {
      state->selected_index = 0;
    } else {
      state->selected_index++;
    }
    active_page->needs_render = true;
  }
};

void menu_page_render_list(struct menu_manager_t *manager_str) {
  // Needed to go out of struct menu_manager_t to avoid circular dependency
  menu_manager_t *manager = (menu_manager_t *)manager_str;
  menu_page_t *active_page = &manager->pages[manager->active_page_id];
  // Redundant check
  if (!active_page->needs_render) {
    return;
  }
  page_list_state *state = (page_list_state *)active_page->state;
  uint8_t current_index = state->selected_index;

  uint8_t previous_index =
      (current_index == 0) ? (state->num_entries - 1) : (current_index - 1);

  uint8_t next_index =
      (current_index == state->num_entries - 1) ? 0 : (current_index + 1);

  uint8_t current_id = state->entry_ids[current_index];
  uint8_t previous_id = state->entry_ids[previous_index];
  uint8_t next_id = state->entry_ids[next_index];
  ssd1306_Fill(Black);
  ssd1306_DrawBitmap(MENU_LIST_ICON_X, MENU_LIST_ICON_Y_1,
                     state->entry_icons[previous_index], MENU_LIST_ICON_WIDTH,
                     MENU_LIST_ICON_WIDTH, White);
  ssd1306_SetCursor(MENU_LIST_TITLE_START, MENU_LIST_ITEM_Y_1 + 4);
  ssd1306_WriteString(manager->pages[previous_id].name, Font_7x10, White);
  LOGI(TAG, "Name of previous item: %s", manager->pages[previous_id].name);

  ssd1306_DrawBitmap(MENU_LIST_ICON_X, MENU_LIST_ICON_Y_2,
                     state->entry_icons[current_index], MENU_LIST_ICON_WIDTH,
                     MENU_LIST_ICON_WIDTH, White);
  ssd1306_SetCursor(MENU_LIST_TITLE_START, MENU_LIST_ITEM_Y_2 + 4);
  ssd1306_WriteString(manager->pages[current_id].name, Font_7x10, White);
  ssd1306_ListBorder(2, MENU_LIST_ITEM_Y_2, MENU_LIST_BORDER_WIDTH,
                     MENU_LIST_BORDER_HEIGHT, White);
  LOGI(TAG, "Name of current item: %s", manager->pages[current_id].name);
  ssd1306_DrawBitmap(MENU_LIST_ICON_X, MENU_LIST_ICON_Y_3,
                     state->entry_icons[next_index], MENU_LIST_ICON_WIDTH,
                     MENU_LIST_ICON_WIDTH, White);
  ssd1306_SetCursor(MENU_LIST_TITLE_START, MENU_LIST_ITEM_Y_3 + 4);
  ssd1306_WriteString(manager->pages[next_id].name, Font_7x10, White);
  LOGI(TAG, "Name of next item: %s", manager->pages[next_id].name);
  ssd1306_ListScrollBar(MENU_LIST_SCROLL_BAR_X, 2, 2, White);
  uint8_t puck_height =
      (SSD1306_HEIGHT - 4) / state->num_entries - 2; // 2px gap
  ssd1306_FillRectangle(
      MENU_LIST_SCROLL_BAR_X - 1,
      2 + (current_index * (puck_height + 2)), // 2px gap
      MENU_LIST_SCROLL_BAR_X + 1,
      2 + (current_index * (puck_height + 2)) + puck_height - 1, White);
  ssd1306_UpdateScreen();
  active_page->needs_render = false;
}

void menu_page_destruct_list(void *state_ptr) {
  // Nothing needs to be done in DESTRUCT
}
