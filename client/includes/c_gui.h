#pragma once

#include "c_includes.h"
#include "raygui.h"

extern const int c_texture_size;
extern const int c_screen_size;
extern const int c_samples;

extern Image g_image;
extern Texture2D g_texture;
extern unsigned char *g_texture_data;

int TextToInt(const char* text);
int GuiIntBox(Rectangle bounds, const char* text, int* value, int minValue, 
              int maxValue, bool editMode);
int GuiCharBox(Rectangle bounds, const char* text, char* value, bool editMode);
int get_index_of_value(int value, const char *text);

void *gui_setup(void *args);
void clear_texture(parameters params);
void create_texture(parameters params);
void delete_texture();
void create_image_from_data(const char *a_data, parameters a_params);