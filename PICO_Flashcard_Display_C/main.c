    #include "OLED_1in3_c.h"
    #include "GUI_Paint.h"
    #include "DEV_Config.h"
    #include <stdio.h>
    #include <stdbool.h>
    #include <stdlib.h>
    #include <string.h>
    #include "pico/stdlib.h"
    #include "pico/cyw43_arch.h"
    #include "time.h"
    #include "picohttps.h"
    #include "hardware/watchdog.h"



    #define MAX_CARDS 1000
    #define MAX_LINE_LENGTH 256
    #define DISPLAY_INTERVAL_MS 60000 // 1 minute
    #define MAX_RESPONSE_SIZE 16384 // 16KB response buffer

    //Constants for drawing large amounts of text on the OLED across multiple pages
    #define PAGE_DURATION_MS 5000
    #define MAX_LINES 5
    #define MAX_CHARS_PER_LINE 23

    typedef struct {
        char *front;
        char *back;
    } Flashcard;

    typedef enum {
        FLASH_NONE,   // timeout or page scroll
        FLASH_FLIP,   // key1 pressed
        FLASH_SKIP,    // key0 pressed
        FLASH_TIMEOUT // card lifetime expired
    } FlashAction;


    Flashcard flashcards[MAX_CARDS];
    int flashcard_count = 0;

    // Parse CSV to flashcards and fill the array
    void trim_whitespace(char *str) {
        if (!str) return;
    
        // Trim leading
        while (*str == ' ' || *str == '\t' || *str == '\r' || *str == '\n') {
            str++;
        }
    
        // Trim trailing
        char *end = str + strlen(str) - 1;
        while (end > str && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) {
            *end-- = '\0';
        }
    }
    
   
    /* ----------------------------------------------
   Very small CSV parser for exactly two columns.
   Handles:
     • quoted and un-quoted fields
     • embedded commas
     • embedded CR/LF inside quoted fields
     • escaped quotes  ""  →  "
   Returns number of flash-cards parsed.
   ----------------------------------------------*/
    static int parse_csv(char *csv)
    {
        flashcard_count = 0;
        char *p = csv;

        while (*p && flashcard_count < MAX_CARDS) {

            // ---- Field 0 (front) ----
            char *front = NULL, *back = NULL;
            size_t front_len = 0, back_len = 0;

            if (*p == '"') {                         // ► quoted field
                p++;                                 // skip opening "
                char *start = p;
                while (*p) {
                    if (*p == '"' && p[1] == '"') {  // ""  ->  "
                        p += 2;
                    } else if (*p == '"') {          // closing "
                        break;
                    } else {
                        p++;
                    }
                }
                front_len = (size_t)(p - start);
                front = strndup(start, front_len);
                if (*p == '"') p++;                  // skip closing "
            } else {                                // ► un-quoted field
                char *start = p;
                while (*p && *p != ',' && *p != '\r' && *p != '\n') p++;
                front_len = (size_t)(p - start);
                front = strndup(start, front_len);
            }

            // expect comma separator
            if (*p == ',') p++; else break;

            // ---- Field 1 (back) ----
            if (*p == '"') {
                p++;
                char *start = p;
                while (*p) {
                    if (*p == '"' && p[1] == '"') {      // escaped quote
                        p += 2;
                    } else if (*p == '"') {              // closing quote
                        break;
                    } else {
                        p++;
                    }
                }
                back_len = (size_t)(p - start);
                back = strndup(start, back_len);
                if (*p == '"') p++;
            } else {
                char *start = p;
                while (*p && *p != '\r' && *p != '\n') p++;
                back_len = (size_t)(p - start);
                back = strndup(start, back_len);
            }

            // consume end-of-record  (CR? LF?)
            if (*p == '\r') p++;
            if (*p == '\n') p++;

            // trim leading/trailing space if you wish
            trim_whitespace(front);
            trim_whitespace(back);

            if (front[0] && back[0]) {
                flashcards[flashcard_count].front = front;
                flashcards[flashcard_count].back  = back;
                flashcard_count++;
            } else {        // bad record: release memory
                free(front);  free(back);
            }
        }

        return flashcard_count;
    }
    


        
            
    

    void show_text_on_oled(const char *text) {
        static UBYTE *BlackImage = NULL;
        if (BlackImage == NULL) {
            UWORD Imagesize = ((OLED_1in3_C_WIDTH % 8 == 0) ? 
                               (OLED_1in3_C_WIDTH / 8) : 
                               (OLED_1in3_C_WIDTH / 8 + 1)) * OLED_1in3_C_HEIGHT;
    
            BlackImage = (UBYTE *)malloc(Imagesize);
            if (BlackImage == NULL) {
                printf("Failed to allocate display buffer.\n");
                while (1); // halt
            }
    
            Paint_NewImage(BlackImage, OLED_1in3_C_WIDTH, OLED_1in3_C_HEIGHT, 0, WHITE);
        }
    
        Paint_Clear(BLACK);
        Paint_DrawString_EN(10, 17, text, &Font12, WHITE, BLACK);
        OLED_1in3_C_Display(BlackImage);
    }

    
    
    FlashAction show_flashcard(const char *text, absolute_time_t card_deadline) {

        int key0 = 15; 
        int key1 = 17;


        static UBYTE *BlackImage = NULL;
        if (BlackImage == NULL) {
            UWORD Imagesize = ((OLED_1in3_C_WIDTH % 8 == 0) ? 
                               (OLED_1in3_C_WIDTH / 8) : 
                               (OLED_1in3_C_WIDTH / 8 + 1)) * OLED_1in3_C_HEIGHT;
            BlackImage = (UBYTE *)malloc(Imagesize);
            if (BlackImage == NULL) {
                printf("Failed to allocate display buffer.\n");
                while (1);
            }
            Paint_NewImage(BlackImage, OLED_1in3_C_WIDTH, OLED_1in3_C_HEIGHT, 0, WHITE);
        }
    
        // Split the text into pages
        int text_len = strlen(text);
        int page_size = MAX_LINES * MAX_CHARS_PER_LINE;
        int num_pages = (text_len + page_size - 1) / page_size;
    
        absolute_time_t next_page_time = make_timeout_time_ms(PAGE_DURATION_MS);
        int current_page = 0;
    
        while (true) {
            Paint_Clear(BLACK);
            int start_index = current_page * page_size;
            for (int i = 0; i < MAX_LINES; i++) {
                int line_start = start_index + i * MAX_CHARS_PER_LINE;
                if (line_start >= text_len) break;
    
                char line[MAX_CHARS_PER_LINE + 1] = {0};
                strncpy(line, text + line_start, MAX_CHARS_PER_LINE);
                line[MAX_CHARS_PER_LINE] = '\0';
    
                Paint_DrawString_EN(2, i * 12 + 4, line, &Font8, WHITE, BLACK);
            }
            OLED_1in3_C_Display(BlackImage);
    
            // Wait for page duration or button interrupt
            if (DEV_Digital_Read(key1) == 0) {          // flip
                sleep_ms(100);                          // debounce
                while (DEV_Digital_Read(key1) == 0);    // wait for release
                return FLASH_FLIP;
            }
            if (DEV_Digital_Read(key0) == 0) {          // skip
                sleep_ms(100);
                while (DEV_Digital_Read(key0) == 0);
                return FLASH_SKIP;
            }
            if (absolute_time_diff_us(get_absolute_time(), card_deadline) < 0) {
                return FLASH_TIMEOUT;
            }
            if (absolute_time_diff_us(get_absolute_time(), next_page_time) < 0) {
                current_page = (current_page + 1) % num_pages;
                next_page_time = make_timeout_time_ms(PAGE_DURATION_MS);                                // show next page
            }
            else{
            sleep_ms(50);
            }
        }
    }

   
    void mainLoop(char *response_data) {
        

        printf("Main loop started...\n");
        printf("RESPONSE BUFFER:\n%s\n", response_buffer);
        
        // Parse the CSV data from the response buffer into flashcards
        char *csv_start = strstr(response_buffer, "\r\n\r\n"); //Find the first blank line in the response (after HTTP header)
        if (csv_start) {
            csv_start += 4; // Move past line break unicode ("\r\n\r\n") to the actual CSV data
            parse_csv(csv_start);
        } else {
            printf("Failed to find CSV body.\n");
        }  
        printf("Parsing finished\n");
        
       
        
        show_text_on_oled("Anki Flashcard   Pico Display");
        DEV_Delay_ms(5000);
        char message[64];
        sprintf(message, "%d flashcards      loaded", flashcard_count);
        show_text_on_oled(message);
        DEV_Delay_ms(5000);


    
        //Begin randomly displaying flashcards    
        
        srand(to_us_since_boot(get_absolute_time())); // seed for rand()

        int current_card = rand() % flashcard_count;
        bool show_front = true;
        absolute_time_t next_flashcard_time = make_timeout_time_ms(DISPLAY_INTERVAL_MS);

        int key0 = 15; 
        int key1 = 17;
        DEV_GPIO_Mode(key0, 0);
        DEV_GPIO_Mode(key1, 0);
    

        while (true) {
            FlashAction act = show_flashcard(
                show_front ? flashcards[current_card].front
                           : flashcards[current_card].back,
                           next_flashcard_time);
        
            switch (act) {
                case FLASH_FLIP:
                    show_front = !show_front;               // flip same card
                    next_flashcard_time = make_timeout_time_ms(DISPLAY_INTERVAL_MS);
                    break;
        
                case FLASH_SKIP:
                case FLASH_TIMEOUT: //timeout also performs skip
                    current_card = rand() % flashcard_count;// new random card
                    show_front = true;
                    next_flashcard_time = make_timeout_time_ms(DISPLAY_INTERVAL_MS);
                    break;
        
                case FLASH_NONE:
                default:
                    /* page-scroll timeout fell through – nothing to do here */
                    break;
            }
        }
    }



    int main() {
        stdio_init_all();
        printf("Starting Pico Anki deck loader...\n");
        
        DEV_Delay_ms(100);
        printf("DEV_Delay Set\r\n");


        if(DEV_Module_Init()!=0){
            while(1){
                printf("END\r\n");
            }
        }

        //Begin displaying flashcards
        printf("Initialising Screen\n");
        OLED_1in3_C_Init();
        printf("OLED Screen Initialised \r\n");
        OLED_1in3_C_Clear();
        printf("OLED Screen Cleared\r\n");
        DEV_Delay_ms(2000);
        show_text_on_oled("Connecting to   WiFi...");

        if(!fetch_csv()){
            printf("Fetch failed. Restarting\n");
            sleep_ms(100);
            watchdog_reboot(0, 0, 0);  // Reboots immediately
            while (true) tight_loop_contents();  // Wait for reboot
        }
        
        
        mainLoop(response_buffer); // Call the main loop with the response data

        return 0;
    }

 