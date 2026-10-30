#pragma once
#include <cstdint>
void workshop_menu_enable();
bool workshop_menu_enabled();
bool workshop_menu_french();
void workshop_menu_register();
void workshop_menu_start_mode(int mode);
bool workshop_menu_wait_parent(unsigned pid);
void workshop_menu_restore_window();
int workshop_menu_take_boot_mode();
