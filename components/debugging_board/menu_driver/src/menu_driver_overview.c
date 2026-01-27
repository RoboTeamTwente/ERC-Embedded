#include "menu_driver_overview.h"
#include "diagnostics/diagnostics.h"
#include "kv_pool.h"
#include "logging.h"
#include "menu_driver.h"
#include "menu_driver/inc/menu_driver.h"
#include "menu_driver_conf.h"
#include <string.h>

#define MENU_OVERVIEW_MAX_VISIBLE_ENTRY_TITLE_LEN 4

static char *TAG = "MENU DRIVER OVERVIEW";

const static unsigned char error_sprite_1[11] = {
    0x00, // 00...... (Row 0)
    0xC0, // 11...... (Row 1)
    0xC0, // 11...... (Row 2)
    0xC0, // 11...... (Row 3)
    0xC0, // 11...... (Row 4)
    0xC0, // 11...... (Row 5)
    0xC0, // 11...... (Row 6)
    0xC0, // 11...... (Row 7)
    0x00, // 00...... (Row 8)
    0xC0, // 11...... (Row 9)
    0x00  // 00...... (Row 10)
};

const static unsigned char error_sprite_2[11] = {
    0xC0, // 11...... (Row 1)
    0xC0, // 11...... (Row 2)
    0xC0, // 11...... (Row 3)
    0xC0, // 11...... (Row 4)
    0xC0, // 11...... (Row 5)
    0xC0, // 11...... (Row 6)
    0xC0, // 11...... (Row 7)
    0x00, // 00...... (Row 0)
    0x00, // 00...... (Row 8)
    0xC0, // 11...... (Row 9)
    0x00  // 00...... (Row 10)
};

result_t
get_menu_page_overview(uint8_t id, uint8_t parent_id,
                       char name[MAX_PAGE_NAME_LEN], uint8_t num_entries,
                       char entries_titles[MENU_OVERVIEW_MAX_ENTRIES]
                                          [MENU_OVERVIEW_MAX_ENTRY_TITLE_LEN],
                       uint8_t entry_lookups[MENU_OVERVIEW_MAX_ENTRIES],
                       kv_pool *kv_pool, menu_page_state *state_ptr,
                       menu_page_t *out_page) {
  // if (num_entries > MENU_OVERVIEW_MAX_ENTRIES || state_ptr == NULL ||
  //     out_page == NULL || name == NULL || kv_pool == NULL) {
  //   return RESULT_ERR_INVALID_ARG;
  // }
  // state_ptr->num_entries = num_entries;
  // state_ptr->kv_pool = kv_pool;
  // // for (int i = 0; i < num_entries; i++) {
  // //   strncpy(state_ptr->entry_titles[i], entries_titles[i],
  // //           MENU_OVERVIEW_MAX_ENTRY_TITLE_LEN);
  // //   state_ptr->entry_lookups[i] = entry_lookups[i];
  // // }
  // out_page->id = id;
  // out_page->init = menu_page_init_overview;
  // out_page->update = menu_page_update_overview;
  // out_page->render = menu_page_render_overview;
  // out_page->destruct = menu_page_destruct_overview;
  // out_page->state = state_ptr;
  // strncpy(out_page->name, name, MAX_PAGE_NAME_LEN);
  return RESULT_ERR_UNIMPLEMENTED;
}

void menu_overview_get_last_states(page_overview_state *state,
                                   uint8_t *code_buffer) {
  diagnostics_t buffer = {0};
  size_t size = sizeof(diagnostics_t);
  result_t res;
  for (int i = 0; i < state->num_entries; i++) {
    res = kv_pool_get(state->kv_pool, state->entry_lookups[i], &buffer, &size);
    if (res != RESULT_OK || size != sizeof(diagnostics_t)) {
      code_buffer[i] = 0xFF; // Indicate error
    } else {
      code_buffer[i] = buffer.last_code;
    }
  }
}

void menu_page_overview_render_entry(uint8_t offset_x, uint8_t offset_y,
                                     const char *title, uint8_t code,
                                     bool err_sprite_state) {
  // if (code != 0x00) {
  //   LOGI(TAG, "Rendering error sprite for code: %02X", code);
  //   LOGI(TAG, "Error sprite state: %d", err_sprite_state);
  //   ssd1306_DrawBitmap(offset_x + 2, offset_y,
  //                      err_sprite_state ? error_sprite_1 : error_sprite_2,
  //                      MENU_OVERVIEW_ICON_WIDTH, MENU_OVERVIEW_ICON_HEIGHT,
  //                      White);
  // }
  // ssd1306_SetCursor(offset_x + MENU_OVERVIEW_ICON_WIDTH + 6, offset_y + 2);
  // char display_title[MENU_OVERVIEW_MAX_ENTRY_TITLE_LEN + 6] = {0};
  // char formatted_title[MENU_OVERVIEW_MAX_VISIBLE_ENTRY_TITLE_LEN + 1] = {0};
  // strncpy(formatted_title, title, MENU_OVERVIEW_MAX_VISIBLE_ENTRY_TITLE_LEN);
  // char *title_format = "%s:x%02X";
  // snprintf(display_title, sizeof(display_title), title_format,
  // formatted_title,
  //          code);
  // LOGI(TAG, "Rendering overview entry: %s", display_title);
  // ssd1306_WriteString(display_title, Font_6x8, White);
}

void menu_page_render_overview(struct menu_manager_t *manager_str) {
  // menu_manager_t *manager = (menu_manager_t *)manager_str;
  // menu_page_t *active_page = &manager->pages[manager->active_page_id];
  // if (active_page->needs_render == false) {
  //   return;
  // }
  // page_overview_state *state = (page_overview_state *)active_page->state;
  // uint8_t offset_x, offset_y;
  //
  // ssd1306_Fill(Black);
  // for (uint8_t i = 0; i < state->num_entries; i++) {
  //   offset_y = i / 2 * 13 + 12; // 13 size of entry, 12 initial offset
  //   offset_x = i % 2 * 65;      // either 0 or 65 (left or right)
  //   menu_page_overview_render_entry(offset_x, offset_y,
  //   state->entry_titles[i],
  //                                   state->last_states[i],
  //                                   state->err_sprite_state);
  // }
  // ssd1306_UpdateScreen();
  // active_page->needs_render = false;
}
void menu_page_update_overview(struct menu_manager_t *manager_str) {
  // menu_manager_t *manager = (menu_manager_t *)manager_str;
  // menu_input input = manager->get_input();
  // menu_page_t *active_page = &manager->pages[manager->active_page_id];
  // if (IS_INPUT_DIRECTION_LEFT(input) || IS_INPUT_BUTTON_PRESS_A(input)) {
  //   // Go to parent page
  //   menu_manager_switch_page(manager, active_page->parent_id);
  //   return;
  // }
  // page_overview_state *state = (page_overview_state *)active_page->state;
  // menu_overview_get_last_states(state, state->last_states);
  // state->err_sprite_state =
  //     !state->err_sprite_state;     // toggle error sprite state
  // active_page->needs_render = true; // always render for animation
}
void menu_page_init_overview(menu_page_state *state_ptr) {
  // page_overview_state *ptr = (page_overview_state *)state_ptr;
  // ptr->err_sprite_state = false;
  // menu_overview_get_last_states(ptr, ptr->last_states);
}
void menu_page_destruct_overview(menu_page_state *state_ptr) {}
