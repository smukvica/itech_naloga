#pragma once


#include "raygui.h"

float TextToFloat(const char* text);                                                                          // Helper function that converts text to float
int GuiIntBox(Rectangle bounds, const char* text, int* value, int minValue, int maxValue, bool editMode); // Custom input box that works with float values. Basicly GuiValueBox(), but with some changes
int GuiCharBox(Rectangle bounds, const char* text, char* value, bool editMode);
int get_index_of_value(int value, const char *text);

void *draw_gui(void *args);