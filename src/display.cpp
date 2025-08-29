#include "display.h"
#include "utils.h"

// Display setup
MultiTFT footswitchDisplay(TFT_CS1);  // Display for footswitch states
MultiTFT configDisplay(TFT_CS2);      // Display for bank/config info

// Helper function for determining text color based on background brightness
uint16_t getTextColorForBackground(uint16_t backgroundColor) {
    // Extract RGB components from RGB565
    uint8_t r = (backgroundColor >> 8) & 0xF8;
    uint8_t g = (backgroundColor >> 3) & 0xFC;
    uint8_t b = (backgroundColor << 3) & 0xF8;
    
    // Calculate brightness using standard formula
    uint16_t brightness = (r * 299 + g * 587 + b * 114) / 1000;
    
    // Use black text for bright colors, white text for dark colors
    return (brightness > 128) ? BLACK : WHITE;
}

// Display initialization
void initializeDisplays() {
    // Initialize both displays
    footswitchDisplay.begin(1); // Landscape
    configDisplay.begin(3);     // Landscape (rotated)
    
    // Draw initial screens
    drawFootswitchScreen();
    drawConfigScreen();
    
    printJsonLog("info", "Displays initialized");
}

// Draw the footswitch states screen (based on ST7796 current state screen)
void drawFootswitchScreen() {
    footswitchDisplay.select();
    footswitchDisplay.fillScreen(BLACK);
    footswitchDisplay.drawRect(0, 0, 480, 320, WHITE);

    // Define switch dimensions and positions for 2x3 grid (2 columns, 3 rows)
    int switch_width = 220;
    int switch_height = 90;
    int margin = 10;

    // Row Y positions (3 rows)
    int row1_y = 10;
    int row2_y = 115;
    int row3_y = 220;

    // Column X positions (2 columns)
    int left_col_x = 10;
    int right_col_x = 250;

    // Draw switches in a 2x3 grid
    for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
        int row = i / 2;  // 0, 1, 2
        int col = i % 2;  // 0, 1
        
        int x = (col == 0) ? left_col_x : right_col_x;
        int y = (row == 0) ? row1_y : (row == 1) ? row2_y : row3_y;
        
        uint16_t bgColor = footswitches[i].enabled ? footswitches[i].color : BLACK;
        uint16_t textColor = footswitches[i].enabled ? 
                            getTextColorForBackground(footswitches[i].color) : RED;
        
        footswitchDisplay.fillRect(x, y, switch_width, switch_height, bgColor);
        footswitchDisplay.drawRect(x, y, switch_width, switch_height, WHITE);
        footswitchDisplay.setTextColor(textColor);
        footswitchDisplay.setTextSize(3);
        footswitchDisplay.setTextDatum(MC_DATUM);
        footswitchDisplay.drawString(footswitches[i].name, 
                                   x + switch_width/2, y + 30);
        footswitchDisplay.setTextSize(2);
        footswitchDisplay.drawString("CC" + String(footswitches[i].midiCC) + " Ch" + String(footswitches[i].midiChannel), 
                                   x + switch_width/2, y + 65);
    }

    footswitchDisplay.deselect();
}

// Draw the configuration/bank screen (based on ST7796 bank list screen)
void drawConfigScreen() {
    configDisplay.select();
    
    // Find the first enabled footswitch color for background
    uint16_t backgroundColor = BLACK;
    if (currentSelectedFootswitch != -1) {
        backgroundColor = footswitches[currentSelectedFootswitch].color;
    }
    
    // Fill background with selected preset color
    configDisplay.fillScreen(backgroundColor);
    
    // Determine text color based on background brightness
    uint16_t primaryTextColor = getTextColorForBackground(backgroundColor);
    uint16_t accentColor = (primaryTextColor == BLACK) ? WHITE : BLACK;
    
    configDisplay.drawRect(0, 0, 480, 320, primaryTextColor);

    // Title row (centered, big)
    configDisplay.setTextDatum(MC_DATUM);
    configDisplay.setTextColor(primaryTextColor);
    configDisplay.setTextSize(4);
    configDisplay.drawString("PRESET STATUS", 240, 40); // Centered at top

    // Count active switches
    int activeCount = 0;
    String activeNames = "";
    for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
        if (footswitches[i].enabled) {
            activeCount++;
            if (activeNames.length() > 0) activeNames += ", ";
            activeNames += footswitches[i].name;
        }
    }

    // Active switches info
    configDisplay.setTextSize(2);
    configDisplay.setTextDatum(TL_DATUM);
    configDisplay.setTextColor(primaryTextColor);
    configDisplay.drawString("ACTIVE EFFECTS: " + String(activeCount), 20, 90);
    
    // Show active switch names (truncate if too long)
    configDisplay.setTextSize(2);
    if (activeNames.length() > 35) {
        activeNames = activeNames.substring(0, 32) + "...";
    }
    configDisplay.drawString(activeNames, 20, 120);

    // MIDI Channel info
    configDisplay.setTextSize(3);
    configDisplay.setTextColor(accentColor);
    configDisplay.drawString("MIDI CH: " + String(footswitches[0].midiChannel), 20, 160);
    
    // Status indicator
    configDisplay.setTextSize(2);
    configDisplay.setTextColor(primaryTextColor);
    configDisplay.drawString("SYSTEM READY", 20, 200);

    // Navigation row (bottom)
    configDisplay.setTextSize(2);
    configDisplay.setTextColor(primaryTextColor);
    configDisplay.setTextDatum(TL_DATUM);
    configDisplay.drawString("< CONFIG", 30, 280);
    configDisplay.setTextDatum(TR_DATUM);
    configDisplay.drawString("SWITCHES >", 450, 280);

    configDisplay.deselect();
}

// Update footswitch display when states change
void updateFootswitchDisplay() {
    drawFootswitchScreen();
}

// Update config display (can be called when configuration changes)
void updateConfigDisplay() {
    drawConfigScreen();
}