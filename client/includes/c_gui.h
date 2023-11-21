#pragma once

#include "c_includes.h"
#include "raygui.h"

extern const int c_samples;

extern Image image;
extern Texture2D texture;
extern unsigned char *texture_data;
extern int texture_width;
extern int texture_height;

int TextToInt(const char* text);
int GuiIntBox(Rectangle bounds, const char* text, int* value, int minValue, 
              int maxValue, bool editMode);
int GuiCharBox(Rectangle bounds, const char* text, char* value, bool editMode);
int get_index_of_value(int value, const char *text);

void *gui_setup(void *args);
void clear_texture(parameters params);
void create_texture(parameters params);
void delete_texture();
void create_image_from_data(char *a_data, parameters a_params);