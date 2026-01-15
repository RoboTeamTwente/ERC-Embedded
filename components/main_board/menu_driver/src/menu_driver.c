#include "menu_driver.h"
#include "ili9341.h"
#include "menu_driver_conf.h"
#include "menu_driver_icons.h"
#include <stdbool.h>

static char *TAG = "MENU DRIVER";

void menu_manager_switch_page(menu_manager_t *manager,
                              unsigned char new_page_id) {
  manager->pages[manager->active_page_id].destruct(
      &manager->pages[manager->active_page_id].state);
  manager->active_page_id = new_page_id;
  manager->pages[manager->active_page_id].init(
      &manager->pages[manager->active_page_id].state);
}

void menu_driver_draw_ribbon() {

  ILI9341_Draw_Bitmap(0, 0, MENU_DRIVER_RIBBON_WIDTH, MENU_DRIVER_RIBBON_HEIGHT,
                      menu_driver_ui_ribbon, MENU_DRIVER_FOREGROUND_COLOR,
                      MENU_DRIVER_BACKGROUND_COLOR);
}
