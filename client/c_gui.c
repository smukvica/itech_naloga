#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <raylib.h>
#include <math.h>

#include "c_gui.h"
#include "c_queue.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"



const char dropdown_num_fields_options[21] = "1;2;3;4;5;6;7;8;9;10\0";
int dropdown_num_fields_value = 0;
const char dropdown_size_fields_options[6] = "1;2;4\0";
int dropdown_size_field_value = 0;
const char dropdown_num_bpm_options[8] = "1;2;3;4\0";
int dropdown_num_bpm_value = 0;
int queue_size_value = 1000;
int port_value = 8888;
int file_entries_value = 100;

int texture_size = 1000;
int samples = 1;
bool refresh = true;

extern int program_terminate;
extern int setup_complete;
extern char filename[25];
extern int read_file;
extern int start;

Image image;
Texture2D texture;
unsigned char *texture_data;
int texture_width;
int texture_height;

int number_of_samples = 0;
int texture_offset = 0;

unsigned int limits_of_data[4] = {0xFF, 0xFFFF, 0xFFFFFFF, 0xFFFFFFFF};

// remove
void clear_texture2(parameters params){
    for(int i = 0; i < texture_width; i++){
        for(int j = 0; j < texture_height;j++){
            if(j == 250 || i == 250)
                texture_data[j * texture_width + i] = 0;
            else
                texture_data[j * texture_width + i] = 255;
        }
    }
}

void clear_texture(parameters params){
    for(int i = 0; i < texture_width; i++){
        for(int j = 0; j < texture_height;j++){
            if(j%(500 / params.num_of_fields) == 0 && j != 0)
                texture_data[j * texture_width + i] = 0;
            else
                texture_data[j * texture_width + i] = 255;
        }
    }
}

void create_texture(parameters params){
    texture_width = texture_size;
    texture_height = 500;
    texture_data = malloc(sizeof(unsigned char) * texture_width * texture_height);
    image = (Image){.data = texture_data, .width = texture_width, .height = texture_height, .mipmaps = 1, .format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE};
    clear_texture(params);
    texture = LoadTextureFromImage(image);
}

void delete_texture(){
    free(texture_data);
}

// remove
void create_image_from_data_circle(char *data, parameters params){
    clear_texture2(params);
    UnloadTexture(texture);
    texture = LoadTextureFromImage(image);
}

void create_image_from_data(char *data, parameters params){
    if(number_of_samples >= 500){
        number_of_samples = 0;
        texture_offset = 0;
        //clear_texture(params);
        refresh = false;
        return;
    }
    for(int k = 0; k < samples; k++){
        int f = k * (params.size_of_field * params.num_of_fields + 4);
        unsigned int out = 0;
        for(int i = 0; i < params.num_of_fields; i++){
            memcpy(&out, &data[f + i * params.size_of_field], sizeof(char) * params.size_of_field);
            unsigned int height_offset = 100 - (unsigned int)(((float)out/(float)limits_of_data[params.size_of_field - 1]) * (500.0 / (float)params.num_of_fields)); 
            texture_data[texture_width * (height_offset + i * 500 / params.num_of_fields) + number_of_samples] = 0;
        }
        number_of_samples += 1;
    }
    UnloadTexture(texture);
    texture = LoadTextureFromImage(image);
}


int get_index_of_value(int value, const char *text){
    int index = 0;

    while(text[index] != '0' + value){
        index++;
    }

    return index / 2;
}

// Get float value from text
int TextToInt(const char* text)
{
	int value = 0;

    // convert text to float
	for (int i = 0; (((text[i] >= '0') && (text[i] <= '9'))); i++)
	{
		value = value * 10 + (int)(text[i] - '0');
	}

	return value;
}

// Float Box control, updates input text with numbers
int GuiIntBox(Rectangle bounds, const char* text, int* value, int minValue, int maxValue, bool editMode)
{
#if !defined(RAYGUI_VALUEBOX_MAX_CHARS)
#define RAYGUI_VALUEBOX_MAX_CHARS  32
#endif

    int result = 0;
    GuiState state = guiState;

    char textValue[RAYGUI_VALUEBOX_MAX_CHARS + 1] = "\0";

    Rectangle textBounds = { 0 };
    if (text != NULL)
    {
        textBounds.width = (float)GetTextWidth(text) + 2;
        textBounds.height = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
        textBounds.x = bounds.x + bounds.width + GuiGetStyle(VALUEBOX, TEXT_PADDING);
        textBounds.y = bounds.y + bounds.height / 2 - GuiGetStyle(DEFAULT, TEXT_SIZE) / 2;
        if (GuiGetStyle(VALUEBOX, TEXT_ALIGNMENT) == TEXT_ALIGN_LEFT) textBounds.x = bounds.x - textBounds.width - GuiGetStyle(VALUEBOX, TEXT_PADDING);
    }

    // Update control
    //--------------------------------------------------------------------
    if ((state != STATE_DISABLED) && !guiLocked && !guiSliderDragging)
    {
        Vector2 mousePoint = GetMousePosition();

        sprintf(textValue, "%d", *value);

        bool valueHasChanged = false;

        int keyCount = (int)strlen(textValue);

        if (editMode)
        {
            state = STATE_PRESSED;

            // Only allow keys in range [48..57]
            if (keyCount < RAYGUI_VALUEBOX_MAX_CHARS)
            {
                if (GetTextWidth(textValue) < bounds.width)
                {
                    int key = GetCharPressed();
                    if ((key >= 48) && (key <= 57))
                    {
                        textValue[keyCount] = (char)key;
                        textValue[++keyCount] = '\0';
                        valueHasChanged = true;
                    }
                }
            }

            // Delete text
            if (keyCount > 0)
            {
                if (IsKeyPressed(KEY_BACKSPACE))
                {
                    keyCount--;
                    textValue[keyCount] = '\0';
                    valueHasChanged = true;
                }
            }

            if (valueHasChanged)
            {
                *value = TextToInt(textValue);
            }

            if (IsKeyPressed(KEY_ENTER) || (!CheckCollisionPointRec(mousePoint, bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)))
            {
                result = 1;
            }
        }
        else
        {
            if (*value > maxValue) *value = maxValue;
            else if (*value < minValue) *value = minValue;

            if (CheckCollisionPointRec(mousePoint, bounds))
            {
                state = STATE_FOCUSED;
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) result = 1;
            }
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    Color baseColor = BLANK;
    sprintf(textValue, "%d", *value);

    if (state == STATE_PRESSED)
    {
        baseColor = GetColor(GuiGetStyle(VALUEBOX, BASE_COLOR_PRESSED));
        textValue[(int)strlen(textValue)] = '\0';
    }
    else if (state == STATE_DISABLED) baseColor = GetColor(GuiGetStyle(VALUEBOX, BASE_COLOR_DISABLED));

    // WARNING: BLANK color does not work properly with Fade()
    GuiDrawRectangle(bounds, GuiGetStyle(VALUEBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(VALUEBOX, BORDER + (state * 3))), guiAlpha), baseColor);
    GuiDrawText(textValue, GetTextBounds(VALUEBOX, bounds), TEXT_ALIGN_CENTER, Fade(GetColor(GuiGetStyle(VALUEBOX, TEXT + (state * 3))), guiAlpha));

    // Draw cursor
    if (editMode)
    {
        // NOTE: ValueBox internal text is always centered
        Rectangle cursor = { bounds.x + GetTextWidth(textValue) / 2 + bounds.width / 2 + 1, bounds.y + 2 * GuiGetStyle(VALUEBOX, BORDER_WIDTH), 4, bounds.height - 4 * GuiGetStyle(VALUEBOX, BORDER_WIDTH) };
        GuiDrawRectangle(cursor, 0, BLANK, Fade(GetColor(GuiGetStyle(VALUEBOX, BORDER_COLOR_PRESSED)), guiAlpha));
    }

    // Draw text label if provided
    GuiDrawText(text, textBounds, (GuiGetStyle(VALUEBOX, TEXT_ALIGNMENT) == TEXT_ALIGN_RIGHT) ? TEXT_ALIGN_LEFT : TEXT_ALIGN_RIGHT, Fade(GetColor(GuiGetStyle(LABEL, TEXT + (state * 3))), guiAlpha));
    //--------------------------------------------------------------------

    return result;
}

int GuiCharBox(Rectangle bounds, const char* text, char* value, bool editMode){
    #if !defined(RAYGUI_VALUEBOX_MAX_CHARS)
    #define RAYGUI_VALUEBOX_MAX_CHARS  32
    #endif

    int result = 0;
    GuiState state = guiState;

    char textValue[RAYGUI_VALUEBOX_MAX_CHARS + 1] = "\0";

    Rectangle textBounds = { 0 };
    if (text != NULL)
    {
        textBounds.width = (float)GetTextWidth(text) + 2;
        textBounds.height = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
        textBounds.x = bounds.x + bounds.width + GuiGetStyle(VALUEBOX, TEXT_PADDING);
        textBounds.y = bounds.y + bounds.height / 2 - GuiGetStyle(DEFAULT, TEXT_SIZE) / 2;
        if (GuiGetStyle(VALUEBOX, TEXT_ALIGNMENT) == TEXT_ALIGN_LEFT) textBounds.x = bounds.x - textBounds.width - GuiGetStyle(VALUEBOX, TEXT_PADDING);
    }

    // Update control
    //--------------------------------------------------------------------
    if ((state != STATE_DISABLED) && !guiLocked && !guiSliderDragging)
    {
        Vector2 mousePoint = GetMousePosition();

        //if (*value >= 0) sprintf(textValue, "+%.3f", *value);
        sprintf(textValue, "%s", value);

        bool valueHasChanged = false;

        int keyCount = (int)strlen(textValue);

        if (editMode)
        {
            state = STATE_PRESSED;

            // Only allow keys in range [48..57]
            if (keyCount < RAYGUI_VALUEBOX_MAX_CHARS)
            {
                if (GetTextWidth(textValue) < bounds.width)
                {
                    int key = GetCharPressed();
                    if (((key >= 48) && (key <= 57)) || ((key >= 65) && (key <= 90)) || ((key >= 97) && (key <= 122)) || key == 46 || key == 95)
                    {
                        textValue[keyCount] = (char)key;
                        textValue[++keyCount] = '\0';
                        valueHasChanged = true;
                    }
                }
            }

            // Delete text
            if (keyCount > 0)
            {
                if (IsKeyPressed(KEY_BACKSPACE))
                {
                    keyCount--;
                    textValue[keyCount] = '\0';
                    valueHasChanged = true;
                }
            }

            if (valueHasChanged)
            {
                strcpy(value, textValue);
            }

            if (IsKeyPressed(KEY_ENTER) || (!CheckCollisionPointRec(mousePoint, bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)))
            {
                result = 1;
            }
        }
        else
        {
            if (CheckCollisionPointRec(mousePoint, bounds))
            {
                state = STATE_FOCUSED;
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) result = 1;
            }
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    Color baseColor = BLANK;
    sprintf(textValue, "%s", value);

    if (state == STATE_PRESSED)
    {
        baseColor = GetColor(GuiGetStyle(VALUEBOX, BASE_COLOR_PRESSED));
        textValue[(int)strlen(textValue)] = '\0';
    }
    else if (state == STATE_DISABLED) baseColor = GetColor(GuiGetStyle(VALUEBOX, BASE_COLOR_DISABLED));

    // WARNING: BLANK color does not work properly with Fade()
    GuiDrawRectangle(bounds, GuiGetStyle(VALUEBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(VALUEBOX, BORDER + (state * 3))), guiAlpha), baseColor);
    GuiDrawText(textValue, GetTextBounds(VALUEBOX, bounds), TEXT_ALIGN_CENTER, Fade(GetColor(GuiGetStyle(VALUEBOX, TEXT + (state * 3))), guiAlpha));

    // Draw cursor
    if (editMode)
    {
        // NOTE: ValueBox internal text is always centered
        Rectangle cursor = { bounds.x + GetTextWidth(textValue) / 2 + bounds.width / 2 + 1, bounds.y + 2 * GuiGetStyle(VALUEBOX, BORDER_WIDTH), 4, bounds.height - 4 * GuiGetStyle(VALUEBOX, BORDER_WIDTH) };
        GuiDrawRectangle(cursor, 0, BLANK, Fade(GetColor(GuiGetStyle(VALUEBOX, BORDER_COLOR_PRESSED)), guiAlpha));
    }

    // Draw text label if provided
    GuiDrawText(text, textBounds, (GuiGetStyle(VALUEBOX, TEXT_ALIGNMENT) == TEXT_ALIGN_RIGHT) ? TEXT_ALIGN_LEFT : TEXT_ALIGN_RIGHT, Fade(GetColor(GuiGetStyle(LABEL, TEXT + (state * 3))), guiAlpha));
    //--------------------------------------------------------------------

    return result;
}

void *gui_setup(void *args){
    SetTraceLogLevel(LOG_ERROR);
    
	InitWindow(250+600, 500, "Client");

    parameters *params = (parameters*)args;

	// General variables
	SetTargetFPS(60);
    
	bool variables[10] = {false};
    bool name_variables[10] = {false};
    bool ip_vars[4] = {false};
    int ip[4];

    dropdown_num_bpm_value = get_index_of_value(params->number_of_bpm, dropdown_num_bpm_options);
    dropdown_num_fields_value = params->num_of_fields - 1;
    dropdown_size_field_value = get_index_of_value(params->size_of_field, dropdown_size_fields_options);

    variables[8] = params->std_output;
    variables[7] = params->file_write;

    queue_size_value = params->queue_size;
    file_entries_value = params->file_entries;
    port_value = params->port;
    sscanf(params->ip, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
	//--------------------------------------------------------------------------------------

    char data[(params->size_of_field * params->num_of_fields + 4) * samples];

    //int ret = 1;
	// Main game loop
	while (!WindowShouldClose())
	{
		// Draw 
		//----------------------------------------------------------------------------------
		BeginDrawing();

        //texture_offset -= GetMouseWheelMove();
        

        if(setup_complete && refresh == true)
        {
            int ret = get_from_queue(&data[0], samples, GUI, *params);
            if(ret != 1){
                create_image_from_data(&data[0], *params);
                //create_image_from_data_circle(&data[0], *params);
            }
        }

        DrawTexture(texture, 350 - texture_offset, 0, WHITE);

        ClearBackground(WHITE);
        DrawRectangle(0, 0, 250, params->num_of_fields * 100, RAYWHITE);

        DrawText("number_of_fields", 10, 10, 10, DARKGRAY);
        DrawText("size_of_field", 10, 50, 10, DARKGRAY);
        DrawText("queue_size", 10, 90, 10, DARKGRAY);
        DrawText("number_of_bpm", 10, 130, 10, DARKGRAY);
        DrawText("file_entries", 10, 170, 10, DARKGRAY);
        DrawText("ip", 10, 210, 10, DARKGRAY);
        DrawText("port", 10, 250, 10, DARKGRAY);
        DrawText("file name", 130, 300, 10, DARKGRAY);
        GuiCheckBox((Rectangle){ 10, 290, 20, 20 }, " write to screen", &variables[8]);
        GuiCheckBox((Rectangle){ 10, 315, 20, 20 }, " write to file", &variables[7]);
        if (GuiIntBox((Rectangle){ 10, 265, 100, 20 }, NULL, &port_value, 0, 1000000, variables[6])) variables[6] = !variables[6];
        for(int i = 0; i < 4; i++){
            if (GuiIntBox((Rectangle){ 10 + i * 25, 225, 24, 20 }, NULL, &ip[i], 0, 255, ip_vars[i])) ip_vars[i] = !ip_vars[i];
        }
        if (GuiIntBox((Rectangle){ 10, 185, 100, 20 }, NULL, &file_entries_value, 500, 1000, variables[4])) variables[4] = !variables[4];
        if (GuiDropdownBox((Rectangle){ 10, 145, 100, 20 }, dropdown_num_bpm_options, &dropdown_num_bpm_value, variables[3])) variables[3] = !variables[3];
        if (GuiIntBox((Rectangle){ 10, 105, 100, 20 }, NULL, &queue_size_value, 100000, 100000000, variables[2])) variables[2] = !variables[2];
        if (GuiDropdownBox((Rectangle){ 10, 65, 100, 20 }, dropdown_size_fields_options, &dropdown_size_field_value, variables[1])) variables[1] = !variables[1];
        if (GuiDropdownBox((Rectangle){ 10, 25, 100, 20}, dropdown_num_fields_options, &dropdown_num_fields_value, variables[0])) variables[0] = !variables[0];

        DrawText("Field names", 130, 10, 10, DARKGRAY);
        for(int i = 0; i < dropdown_num_fields_value + 1; i++){
		    if (GuiCharBox((Rectangle){ 130, 25 + i * 25, 100, 20 }, NULL, params->names[i], name_variables[i])) name_variables[i] = !name_variables[i];
            if(strlen(params->names[i]) >= 32)
                params->names[i][32] = '\0';
        }

        if (GuiCharBox((Rectangle){ 130, 315, 100, 20 }, NULL, &filename[0], variables[9])) variables[9] = !variables[9];

		if (GuiButton((Rectangle){ 10, 350, 50, 20 }, "Setup") && setup_complete == 0)
		{
            sprintf(params->ip, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
            params->size_of_field = (int)(dropdown_size_fields_options[dropdown_size_field_value*2] - '0');
            params->number_of_bpm = (int)(dropdown_num_bpm_options[dropdown_num_bpm_value*2] - '0');
            params->num_of_fields = dropdown_num_fields_value + 1;

            params->port = port_value;
            params->file_entries = file_entries_value;
            params->queue_size = queue_size_value;

            if(variables[7] == true)
                params->file_write = 1;
            else
                params->file_write = 0;

            if(variables[8] == true)
                params->std_output = 1;
            else
                params->std_output = 0;
            setup_complete = 1;
            create_texture(*params);
		}

        if (GuiButton((Rectangle){ 10, 375, 50, 20 }, "Start") && setup_complete == 1)
		{
            start = 1;
		}

        if (GuiButton((Rectangle){ 130, 350, 50, 20 }, "Read file") && setup_complete == 1)
		{
            read_file = 1;
            refresh = true;
		}

        DrawRectangle(250, 0, 100, 500, RAYWHITE);

        if(setup_complete == 1){
            for(int i = 0; i < params->num_of_fields; i++){
                DrawText(params->names[i], 260, 10 + i * 500 / params->num_of_fields, 10, DARKGRAY);
            }
            
            if(GuiButton((Rectangle){ 130, 375, 75, 20 }, "Refresh data")){
                refresh = true;
                clear_texture(*params);
                number_of_samples = 0;
                update_queue_index(GUI);
            }
        }
        

        
		EndDrawing();
        //----------------------------------------------------------------------------------
        sleep(0);
	}
    
	CloseWindow();
    delete_texture();
    program_terminate = 1;
    sleep(1);
}