#include "c_gui.h"
#include "c_includes.h"
/*******************************************************************************************
*
*   raygui - basic calculator app with custom input box for float values
*
*   DEPENDENCIES:
*       raylib 4.5  - Windowing/input management and drawing.
*       raygui 3.5  - Immediate-mode GUI controls.
*
*   COMPILATION (Windows - MinGW):
*       gcc -o $(NAME_PART).exe $(FILE_NAME) -I../../src -lraylib -lopengl32 -lgdi32 -std=c99
*
**********************************************************************************************/

#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"


extern int program_terminate;
extern int num_of_fields;
extern int size_of_field;
extern int queue_size;
extern int number_of_bpm;
extern int number_of_packets;
extern int file_entries;
extern int file_write;
extern int std_output;
extern arg_struct arguments;

extern char names[10][32];

const char dropdown_num_fields_options[21] = "1;2;3;4;5;6;7;8;9;10\0";
int dropdown_num_fields_value = 0;
const char dropdown_size_fields_options[6] = "1;2;4\0";
int dropdown_size_field_value = 0;
const char dropdown_num_bpm_options[8] = "1;2;3;4\0";
int dropdown_num_bpm_value = 0;
int queue_size_value = 1000;
int port_value = 8888;
int file_entries_value = 100;


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
                    if (((key >= 48) && (key <= 57)) || ((key >= 65) && (key <= 90)) || key == 46)
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

void gui_setup(void *args){
    
	InitWindow(250, 400, "Client");

	// General variables
	SetTargetFPS(60);
    
	bool variables[17];
    bool name_variables[10];
    bool ip_vars[4];
    int ip[4];

    dropdown_num_bpm_value = get_index_of_value(number_of_bpm, dropdown_num_bpm_options);
    dropdown_num_fields_value = num_of_fields - 1;
    dropdown_size_field_value = get_index_of_value(size_of_field, dropdown_size_fields_options);

    queue_size_value = queue_size;
    file_entries_value = file_entries;
    port_value = arguments.port;
    sscanf(arguments.ip, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
	//--------------------------------------------------------------------------------------

	// Main game loop
	while (1)
	{
		// Draw 
		//----------------------------------------------------------------------------------
		BeginDrawing();

		ClearBackground(RAYWHITE);

        DrawText("number_of_fields", 10, 10, 10, DARKGRAY);
        DrawText("size_of_field", 10, 50, 10, DARKGRAY);
        DrawText("queue_size", 10, 90, 10, DARKGRAY);
        DrawText("number_of_bpm", 10, 130, 10, DARKGRAY);
        DrawText("file_entries", 10, 170, 10, DARKGRAY);
        DrawText("ip", 10, 210, 10, DARKGRAY);
        DrawText("port", 10, 250, 10, DARKGRAY);
        GuiCheckBox((Rectangle){ 10, 290, 20, 20 }, " write to std out", &variables[8]);
        GuiCheckBox((Rectangle){ 10, 315, 20, 20 }, " write to file", &variables[7]);
        if (GuiIntBox((Rectangle){ 10, 265, 100, 20 }, NULL, &port_value, 0, 1000000, variables[6])) variables[6] = !variables[6];
        for(int i = 0; i < 4; i++){
            if (GuiIntBox((Rectangle){ 10 + i * 25, 225, 24, 20 }, NULL, &ip[i], 0, 255, ip_vars[i])) ip_vars[i] = !ip_vars[i];
        }
        if (GuiIntBox((Rectangle){ 10, 185, 100, 20 }, NULL, &file_entries_value, 1, 100000, variables[4])) variables[4] = !variables[4];
        if (GuiDropdownBox((Rectangle){ 10, 145, 100, 20 }, dropdown_num_bpm_options, &dropdown_num_bpm_value, variables[3])) variables[3] = !variables[3];
        if (GuiIntBox((Rectangle){ 10, 105, 100, 20 }, NULL, &queue_size_value, 1, 100000, variables[2])) variables[2] = !variables[2];
        if (GuiDropdownBox((Rectangle){ 10, 65, 100, 20 }, dropdown_size_fields_options, &dropdown_size_field_value, variables[1])) variables[1] = !variables[1];
        if (GuiDropdownBox((Rectangle){ 10, 25, 100, 20}, dropdown_num_fields_options, &dropdown_num_fields_value, variables[0])) variables[0] = !variables[0];

        DrawText("Field names", 123, 10, 10, DARKGRAY);
        for(int i = 0; i < dropdown_num_fields_value + 1; i++){
		    if (GuiCharBox((Rectangle){ 123, 25 + i * 25, 100, 20 }, NULL, names[i], name_variables[i])) name_variables[i] = !name_variables[i];
        }

		if (GuiButton((Rectangle){ 10, 350, 50, 20 }, "Connect"))
		{
            sprintf(arguments.ip, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
            size_of_field = (int)(dropdown_size_fields_options[dropdown_size_field_value*2] - '0');
            number_of_bpm = (int)(dropdown_num_bpm_options[dropdown_num_bpm_value*2] - '0');
            num_of_fields = dropdown_num_fields_value + 1;

            arguments.port = port_value;
            file_entries = file_entries_value;
            queue_size = queue_size_value;

            if(variables[7] == true)
                file_write = 1;
            else
                file_write = 0;

            if(variables[8] == true)
                std_output = 1;
            else
                std_output = 0;
            break;
		}
		
		EndDrawing();
        //----------------------------------------------------------------------------------
        if(WindowShouldClose()){
            break;
        }
	}
    
	CloseWindow();
    
}

void *gui_draw(void *args){
    while(program_terminate != 1){}
}