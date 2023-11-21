#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <raylib.h>
#include <math.h>

#include "c_gui.h"
#include "c_queue.h"
#include "c_file.h"
#include "c_output.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// size of generated g_texture, c_samples taken each timestep
const int c_texture_size = 500;
const int c_screen_size = 500;
const int c_samples = 500;

// control if new values are being drawn on screen or not
bool g_refresh = true;

// g_texture data
Image g_image;
Texture2D g_texture;
unsigned char *g_texture_data;

// current number of c_samples in g_texture
int g_number_of_samples = 0;

// limits given for different sizes of fields
const unsigned int c_limits_of_data[4] = {0xFF, 0xFFFF, 0xFFFFFF, 0xFFFFFFFF};

enum mode {passive, receiver, file};
enum mode current_mode = passive;

// clears g_texture to default state - white with black lines dividing the fields
void clear_texture(parameters a_params){
    if(get_trace() == 1)
        printf("clearing texture\n");
    for(int i = 0; i < c_texture_size; i++){
        for(int j = 0; j < c_screen_size;j++){
            if(j%(c_screen_size / a_params.number_of_fields) == 0 && j != 0)
                g_texture_data[j * c_texture_size + i] = 127;
            else
                g_texture_data[j * c_texture_size + i] = 255;
        }
    }
    g_number_of_samples = 0;
}

void create_texture(parameters params){
    if(get_trace() == 1)
        printf("creating texture\n");
    g_texture_data = malloc(sizeof(unsigned char) * 
                            c_texture_size * 
                            c_screen_size);
    g_image = (Image){.data = g_texture_data, 
                    .width = c_texture_size, 
                    .height = c_screen_size, 
                    .mipmaps = 1, 
                    .format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE};
    for(int i = 0; i < c_texture_size * c_screen_size; i++){
        g_texture_data[i] = 255;
    }
}

void delete_texture(){
    if(get_trace() == 1)
        printf("deleting texture\n");
    free(g_texture_data);
}

void create_image_from_data(char *a_data, parameters a_params){
    if(get_trace() == 1)
        printf("updating texture\n");

    unsigned int field_height;
    float field_ratio;
    // loop through fetched c_samples and draw on g_texture
    for(int k = 0; k < c_samples; k++){
        // index of sample in char array
        int size_of_data = (a_params.size_of_field * 
                            a_params.number_of_fields + 
                            STATUS_FIELD_SIZE);
        int f = k * size_of_data;
        // temporary variable for output
        unsigned int out = 0;
        for(int i = 0; i < a_params.number_of_fields; i++){
            // copy data to out
            memcpy(&out, 
                   &a_data[f + i * a_params.size_of_field], 
                   sizeof(char) * a_params.size_of_field);
            // calculate height of field in screen size
            field_height = c_screen_size / a_params.number_of_fields;
            // calculate position from 0 to 1 based on out and max value
            field_ratio = out / (float)c_limits_of_data[a_params.size_of_field - 1];
            // field height - ratio because coordinates go down
            const unsigned int height_offset = field_height - 
                                               field_height * field_ratio;
            // set g_texture data value to black where sample is located
            g_texture_data[c_texture_size * 
                           (height_offset + i * field_height) + 
                           g_number_of_samples] = 0;
        }
        g_number_of_samples += 1;
    }
}

// gets index of specified value from a string for dropdown
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
int GuiIntBox(Rectangle bounds, const char* text, int* value,
              int minValue, int maxValue, bool editMode)
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
    #define RAYGUI_VALUEBOX_MAX_CHARS  40
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
                    if (((key >= 48) && (key <= 57)) ||     
                        ((key >= 65) && (key <= 90)) || 
                        ((key >= 97) && (key <= 122)) || 
                        key == 46 || key == 95 || key == 47)
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

void *gui_setup(void *a_args){
    SetTraceLogLevel(LOG_ERROR); // no logs from raylib
    
	InitWindow(350+c_texture_size, c_screen_size, "Client");

    parameters *params = a_args;

	// General variables
	SetTargetFPS(60);
    
    // variables to select gui elements
	bool variables[11] = {false};
    bool name_variables[MAX_NAMES] = {false};
    bool ip_vars[4] = {false};

    bool can_change = true;
	//--------------------------------------------------------------------------------------

    // data field to store c_samples
    char data[(get_limit("number_of_fields", 1) * 
               get_limit("size_of_field", 1) + STATUS_FIELD_SIZE) * c_samples];
    char corrupt_packages[32];

    create_texture(*params);
    g_texture = LoadTextureFromImage(g_image);

	// Main loop
	while (!WindowShouldClose())
	{
		// Draw 
		//----------------------------------------------------------------------------------
		BeginDrawing();

        if(get_setup_complete() == 1 && g_refresh == true)
        {
            if(g_number_of_samples == c_screen_size){
                if(current_mode == file)
                    current_mode = passive;
                g_number_of_samples = 0;
                g_refresh = false;
            }else{
                // get data from queue
                int ret = get_from_queue(&data[0], c_samples, GUI, *params);
                if(ret != 1){ // if data is successful draw to screen
                    create_image_from_data(&data[0], *params);
                    UnloadTexture(g_texture);
                    g_texture = LoadTextureFromImage(g_image);
                }
            }
        }

        DrawTexture(g_texture, 350, 0, WHITE);

        // input fields
        {
            ClearBackground(WHITE);
            DrawRectangle(0, 0, 250, c_screen_size, RAYWHITE);

            DrawText("Current Mode: ", 10, 450, 10, DARKGRAY);
            switch (current_mode)
            {
            case passive:
                DrawText("Waiting", 10, 460, 10, DARKGRAY);
                break;
            case receiver:
                DrawText("Receiving data", 10, 460, 10, DARKGRAY);
                break;
            default:
                DrawText("Reading data", 10, 460, 10, DARKGRAY);
                break;
            }

            sprintf(corrupt_packages, "package errors: %d", get_packet_errors());
            DrawText(corrupt_packages, 10, 475, 10, DARKGRAY);

            

            DrawText("number_of_fields", 10, 10, 10, DARKGRAY);
            DrawText("size_of_field", 10, 50, 10, DARKGRAY);
            DrawText("queue_size", 10, 90, 10, DARKGRAY);
            DrawText("number_of_bpm", 10, 130, 10, DARKGRAY);
            DrawText("file_entries", 10, 170, 10, DARKGRAY);
            DrawText("ip", 10, 210, 10, DARKGRAY);
            DrawText("port", 10, 250, 10, DARKGRAY);
            DrawText("save folder", 130, 295, 10, DARKGRAY);
            DrawText("file name to read", 130, 335, 10, DARKGRAY);
            GuiCheckBox((Rectangle){ 10, 290, 20, 20 }, 
                        " write to screen", &params->std_output);
            GuiCheckBox((Rectangle){ 10, 315, 20, 20 }, 
                        " write to file", &params->file_write);
            if (GuiIntBox((Rectangle){ 10, 265, 100, 20 }, 
                        NULL, 
                        &params->port, 
                        1025, 
                        10000, 
                        variables[6] & can_change)) variables[6] = !variables[6];
            for(int i = 0; i < 4; i++){
                if (GuiIntBox((Rectangle){ 10 + i * 25, 225, 24, 20 }, 
                            NULL, 
                            &params->ip[i], 
                            0, 
                            255, 
                            ip_vars[i] & can_change)) ip_vars[i] = !ip_vars[i];
            }
            if (GuiIntBox((Rectangle){ 10, 185, 100, 20 }, 
                NULL, 
                &params->file_entries,
                get_limit("file_entries", 0), 
                get_limit("file_entries", 1), 
                variables[4] & can_change)) variables[4] = !variables[4];
            if (GuiIntBox((Rectangle){ 10, 145, 100, 20 }, 
                NULL, 
                &params->number_of_bpm,
                get_limit("number_of_bpm", 0), 
                get_limit("number_of_bpm", 1), 
                variables[3] & can_change)) variables[3] = !variables[3];
            if (GuiIntBox((Rectangle){ 10, 105, 100, 20 }, 
                NULL, 
                &params->queue_size, 
                get_limit("queue_size", 0), 
                get_limit("queue_size", 1), 
                variables[2] & can_change)) variables[2] = !variables[2];
            if (GuiIntBox((Rectangle){ 10, 65, 100, 20 },
                NULL, 
                &params->size_of_field,
                get_limit("size_of_field", 0), 
                get_limit("size_of_field", 1), 
                variables[1] & can_change)) variables[1] = !variables[1];
            if (GuiIntBox((Rectangle){ 10, 25, 100, 20}, 
                NULL, 
                &params->number_of_fields, 
                get_limit("number_of_fields", 0), 
                get_limit("number_of_fields", 1), 
                variables[0] & can_change)) variables[0] = !variables[0];

            DrawText("Field names", 130, 10, 10, DARKGRAY);
            for(int i = 0; i < params->number_of_fields && i < 10; i++){
                if (GuiCharBox((Rectangle){ 130, 25 + i * 25, 100, 20 }, 
                    NULL, params->names[i], 
                    name_variables[i] & can_change)) 
                    name_variables[i] = !name_variables[i];
                if(strlen(params->names[i]) >= 32)
                    params->names[i][32] = '\0';
            }

            if (GuiCharBox((Rectangle){ 130, 350, 120, 20 }, 
                NULL, 
                &g_filename[0], 
                variables[9] && can_change)) 
                variables[9] = !variables[9];
            if (GuiCharBox((Rectangle){ 130, 310, 120, 20 }, 
                NULL, 
                &params->save_folder[0], 
                variables[10] && can_change)) 
                variables[10] = !variables[10];
        }


		if (GuiButton((Rectangle){ 10, 350, 50, 20 }, "Start") && 
            current_mode == passive)
		{
            if(get_trace() == 1)
                printf("starting net capture\n");
            set_setup_complete(1);
            current_mode = receiver;
            clear_texture(*params);
            set_start_stop(1);
		}

        if (GuiButton((Rectangle){ 10, 375, 50, 20 }, "Stop") && 
            current_mode == receiver)
        {
            if(get_trace() == 1)
                printf("stopping net capture\n");
            current_mode = passive;
            set_start_stop(0);
        }

        // sets the client to read from file
        if (GuiButton((Rectangle){ 130, 375, 50, 20 }, "Read file") && 
            current_mode == passive)
		{
            if(get_trace() == 1)
                printf("reading file\n");
            set_read_file(1);
            g_refresh = true;
            set_setup_complete(1);
		}

        DrawRectangle(250, 0, 100, c_screen_size, RAYWHITE);

        if(get_setup_complete() == 1){
            for(int i = 0; i < params->number_of_fields; i++){
                DrawText(params->names[i], 
                         260, 
                         10 + i * c_screen_size / params->number_of_fields, 
                         10, 
                         DARKGRAY);
            }
            
            if(GuiButton((Rectangle){ 130, 450, 75, 20 }, "Refresh data")){
                if(get_trace() == 1)
                    printf("refreshing data\n");
                g_refresh = true;
                clear_texture(*params);
                g_number_of_samples = 0;
                if(current_mode == receiver) // update index only when receiving
                    update_queue_index(GUI, *params);
            }
        }
        

        
		EndDrawing();
        //----------------------------------------------------------------------
        if(current_mode != passive){
            can_change = false;
        }else{
            can_change = true;
        }
	}
    
	CloseWindow();
    delete_texture();
    set_program_terminate(1);
    sleep(1);
}