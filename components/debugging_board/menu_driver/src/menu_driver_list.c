#include "menu_driver_list.h"
#include "ili9341.h"
#include "ili9341_fonts.h"
#include "menu_driver.h"
#include "menu_driver_conf.h"
#include "logging.h"
#include "menu_driver.h"
#include "menu_driver_conf.h"
#include "menu_driver_icons.h"
#include "result.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
static char *TAG = "MENU DRIVER LIST";

result_t get_menu_page_list(
    uint8_t id, uint8_t parent_id, uint8_t num_entries,
    uint8_t entry_ids[MAX_LIST_ENTRIES],
    unsigned char entry_icons[MAX_LIST_ENTRIES][MENU_DRIVER_ICON_BYTE_SIZE],
    char name[MAX_PAGE_NAME_LEN], menu_page_state *state_ptr,
    menu_page_t *out_page) {
  // LOGI(TAG, "Creating List Menu Page: %s with %d entries\n", name,
  // num_entries); if (num_entries > MAX_LIST_ENTRIES || out_page == NULL ||
  // state_ptr == NULL ||
  //     name == NULL) {
  //   out_page = NULL;
  //   return RESULT_ERR_INVALID_ARG;
  // }
  // LOGI(TAG, "Allocating List Menu Page State\n");
  // // Populate state
  // state_ptr->num_entries = num_entries;
  // state_ptr->selected_index = 0;
  // LOGI(TAG, "Populating List Menu Page State\n");
  // for (int i = 0; i < num_entries; i++) {
  //   state_ptr->entry_ids[i] = entry_ids[i];
  //   LOGI(TAG, "Copying icon for entry %d\n", i);
  //   memcpy(state_ptr->entry_icons[i], entry_icons[i],
  //          MENU_DRIVER_ICON_BYTE_SIZE * sizeof(int));
  // }
  // LOGI(TAG, "List Menu Page State Populated\n");
  // // Populate page
  // out_page->id = id;
  // out_page->init = menu_page_init_list;
  // out_page->update = menu_page_update_list;
  // out_page->render = menu_page_render_list;
  // out_page->destruct = menu_page_destruct_list;
  // out_page->state = state_ptr;
  // LOGI(TAG, "Copying name for List Menu Page\n");
  // strncpy(out_page->name, name, MAX_PAGE_NAME_LEN);
  return RESULT_OK;
}

void menu_page_init_list(menu_page_state *state_ptr) {
  state_ptr->list.first_render = true;
  // Nothing needs to be done in INIT
}

void menu_page_update_list(struct menu_manager_t *manager_str) {
  // Needed to go out of struct menu_manager_t to avoid circular dependency
  menu_manager_t *manager = (menu_manager_t *)manager_str;
  menu_input input = manager->get_input();
  menu_page_t *active_page = &manager->pages[manager->active_page_id];
  page_list_state *state = (page_list_state *)active_page->state;
  //
  // if (IS_INPUT_DIRECTION_LEFT(input)) {
  //   // Go to parent page
  //   menu_manager_switch_page(manager, active_page->parent_id);
  // } else if (IS_INPUT_DIRECTION_RIGHT(input) ||
  //            IS_INPUT_BUTTON_PRESS_A(input)) {
  //   // Go to selected child page
  //   uint8_t selected_id = state->entry_ids[state->selected_index];
  //   menu_manager_switch_page(manager, selected_id);
  // } else if (IS_INPUT_DIRECTION_TOP(input)) {
  //   // Move selection up
  //   if (state->selected_index == 0) {
  //     state->selected_index = state->num_entries - 1;
  //   } else {
  //     state->selected_index--;
  //   }
  //   active_page->needs_render = true;
  // } else if (IS_INPUT_DIRECTION_BOTTOM(input)) {
  //   // Move selection down
  //   if (state->selected_index == state->num_entries - 1) {
  //     state->selected_index = 0;
  //   } else {
  //     state->selected_index++;
  //   }
  //   active_page->needs_render = true;
  // }
};

#define MENU_DRIVER_LIST_ICON_OFFSET 10
#define MENU_DRIVER_LIST_TEXT_OFFSET (MENU_DRIVER_LIST_ICON_OFFSET + 20 + MENU_DRIVER_ICON_WIDTH)

static uint16_t last_text_widths[3] = {0};
void menu_driver_list_render_item(uint8_t index, uint16_t x_offset, uint16_t y_offset,
                                  const uint8_t *icon, char *name) {
  uint16_t current_width = strlen(name)*ILI9341_Font_11x18.width;
  if (current_width < last_text_widths[index]) {
    uint16_t difference = last_text_widths[index]-current_width;
    ILI9341_Draw_Rectangle(x_offset + MENU_DRIVER_LIST_TEXT_OFFSET+current_width, y_offset+28, difference,
                           MENU_DRIVER_ICON_HEIGHT, MENU_DRIVER_BACKGROUND_COLOR);
  }
  // draw icon
  ILI9341_Draw_Bitmap(x_offset + 10, y_offset + 14, MENU_DRIVER_ICON_WIDTH,
                      MENU_DRIVER_ICON_HEIGHT, icon,
                      MENU_DRIVER_BACKGROUND_COLOR,
                      MENU_DRIVER_FOREGROUND_COLOR);
  ILI9341_WriteString(x_offset + MENU_DRIVER_LIST_TEXT_OFFSET,
                      y_offset + 28, name, ILI9341_Font_11x18,
                      MENU_DRIVER_FOREGROUND_COLOR,
                      MENU_DRIVER_BACKGROUND_COLOR);
  last_text_widths[index] = current_width;
}
void menu_page_render_list(struct menu_manager_t *manager_str) {
  // Needed to go out of struct menu_manager_t to avoid circular dependency
  menu_manager_t *manager = (menu_manager_t *)manager_str;
  if (manager->active_page_id >= MAX_PAGES) {
    return;
  }
  menu_page_t *active_page = &manager->pages[manager->active_page_id];

  if (active_page->type != MENU_PAGE_TYPE_LIST) {
    LOGE(TAG, "Page type is not LIST for page: %s\n", active_page->name);
    return;
  }
  // Redundant check
  page_list_state *state = &active_page->state->list;
  uint8_t current_index = state->selected_index;

  if (state->first_render) {
    state->first_render=false;
    // draw outline double stroke
    ILI9341_Draw_Rectangle(50, 88, MENU_LIST_ITEM_WIDTH, 2,
                         MENU_DRIVER_FOREGROUND_COLOR);
    ILI9341_Draw_Rectangle(50, 88 + MENU_LIST_ITEM_HEIGHT - 2,
                         MENU_LIST_ITEM_WIDTH, 2, MENU_DRIVER_FOREGROUND_COLOR);
    ILI9341_Draw_Rectangle(50, 88, 2, MENU_LIST_ITEM_HEIGHT,
                         MENU_DRIVER_FOREGROUND_COLOR);
    ILI9341_Draw_Rectangle(50 + MENU_LIST_ITEM_WIDTH - 2, 88, 2,
                         MENU_LIST_ITEM_HEIGHT, MENU_DRIVER_FOREGROUND_COLOR);
  }

  uint8_t previous_index =
      (current_index == 0) ? (state->num_entries - 1) : (current_index - 1);

  uint8_t next_index =
      (current_index == state->num_entries - 1) ? 0 : (current_index + 1);

  // ILI9341_Draw_Rectangle(MENU_DRIVER_RIBBON_WIDTH, 0, ILI9341_SCREEN_WIDTH,
  // ILI9341_SCREEN_HEIGHT, MENU_DRIVER_BACKGROUND_COLOR);

  uint8_t indexes[3] = {previous_index, current_index, next_index};

  for (int i = 0; i < 3; i++) {
    uint16_t y_position =
        MENU_LIST_ITEM_GAP + i * (MENU_LIST_ITEM_HEIGHT + MENU_LIST_ITEM_GAP);
    menu_driver_list_render_item(i,
        MENU_LIST_ITEM_X, y_position, state->entry_icons[indexes[i]],
        manager->pages[state->entry_ids[indexes[i]]].name);
  }

}

void menu_page_destruct_list(menu_page_state *state_ptr) {
  // Nothing needs to be done in DESTRUCT
}
