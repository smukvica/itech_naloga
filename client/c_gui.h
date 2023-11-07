#pragma once

#include "c_includes.h"
#include "raygui.h"

int TextToInt(const char* text);
int GuiIntBox(Rectangle bounds, const char* text, int* value, int minValue, int maxValue, bool editMode);
int GuiCharBox(Rectangle bounds, const char* text, char* value, bool editMode);
int get_index_of_value(int value, const char *text);

void *gui_setup(void *args);