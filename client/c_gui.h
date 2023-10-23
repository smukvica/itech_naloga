#pragma once


#include "raygui.h"

int TextToInt(const char* text);
int GuiIntBox(Rectangle bounds, const char* text, int* value, int minValue, int maxValue, bool editMode);
int GuiCharBox(Rectangle bounds, const char* text, char* value, bool editMode);
int get_index_of_value(int value, const char *text);

int gui_setup();
void *gui_draw(void *args);