#include "display.h"
#include "utils.h"
#include "switches.h"

// Display setup
MultiTFT footswitchDisplay(TFT_CS1);  // Display for footswitch states
MultiTFT configDisplay(TFT_CS2);      // Display for bank/config info

// Helper: extract RGB components from RGB565 and compute brightness
static uint16_t computeBrightnessFromRGB565(uint16_t color) {
    uint8_t r = (color >> 8) & 0xF8;
    uint8_t g = (color >> 3) & 0xFC;
    uint8_t b = (color << 3) & 0xF8;
    return (r * 299 + g * 587 + b * 114) / 1000;
}

// Helper function for determining text color based on background brightness
uint16_t getTextColorForBackground(uint16_t backgroundColor) {
    uint16_t brightness = computeBrightnessFromRGB565(backgroundColor);
    return (brightness > 128) ? BLACK : WHITE;
}

// Small helper to draw a centered title on a display
static void drawCenteredTitle(MultiTFT &display, const String &text, uint16_t color, int y) {
    display.setTextDatum(MC_DATUM);
    display.setTextColor(color);
    display.setTextSize(4);
    display.drawString(text, 240, y);
}

// Display initialization
void initializeDisplays() {
    footswitchDisplay.begin(1); // Landscape
    configDisplay.begin(3);     // Landscape (rotated)
    printJsonLog("info", "Displays initialized");
}

// Internal: draw a single footswitch tile at x,y with given dimensions
static void drawFootswitchTile(MultiTFT &display, int x, int y, int w, int h, const FootswitchConfig &fs) {
    uint16_t bgColor = fs.enabled ? fs.color : BLACK;
    uint16_t textColor = fs.enabled ? getTextColorForBackground(fs.color) : RED;

    display.fillRect(x, y, w, h, bgColor);
    display.drawRect(x, y, w, h, WHITE);

    display.setTextColor(textColor);
    display.setTextSize(3);
    display.setTextDatum(MC_DATUM);
    display.drawString(fs.name, x + w / 2, y + 30);

    display.setTextSize(2);
    display.drawString("CC" + String(fs.midiCC) + " Ch" + String(fs.midiChannel), x + w / 2, y + 65);
}

// Draw the footswitch states screen (based on ST7796 current state screen)
void drawFootswitchScreen() {
    footswitchDisplay.select();
    footswitchDisplay.fillScreen(BLACK);
    footswitchDisplay.drawRect(0, 0, 480, 320, WHITE);

    const int switch_width = 220;
    const int switch_height = 90;

    const int row_y[] = {10, 115, 220};
    const int col_x[] = {10, 250};

    for (int i = 0; i < NUM_FOOTSWITCHES; ++i) {
        int row = i / 2;
        int col = i % 2;
        int x = col_x[col];
        int y = row_y[row];
        drawFootswitchTile(footswitchDisplay, x, y, switch_width, switch_height, footswitches[i]);
    }

    footswitchDisplay.deselect();
}

// Draw the configuration/bank screen (based on ST7796 bank list screen)
void drawConfigScreen() {
    configDisplay.select();

    // Determine background color; if no selection use BLACK
    uint16_t backgroundColor = BLACK;
    if (currentSelectedFootswitch >= 0 && currentSelectedFootswitch < NUM_FOOTSWITCHES) {
        backgroundColor = footswitches[currentSelectedFootswitch].color;
    }

    configDisplay.fillScreen(backgroundColor);

    uint16_t primaryTextColor = getTextColorForBackground(backgroundColor);
    uint16_t accentColor = (primaryTextColor == BLACK) ? WHITE : BLACK;

    configDisplay.drawRect(0, 0, 480, 320, primaryTextColor);

    // Title (guard against invalid selection)
    String title = "";
    if (currentSelectedFootswitch >= 0 && currentSelectedFootswitch < NUM_FOOTSWITCHES) {
        title = footswitches[currentSelectedFootswitch].name;
    }
    drawCenteredTitle(configDisplay, title, primaryTextColor, 40);

    // Count active switches and build truncated list
    int activeCount = 0;
    String activeNames = "";
    for (int i = 0; i < NUM_FOOTSWITCHES; ++i) {
        if (footswitches[i].enabled) {
            ++activeCount;
            if (activeNames.length() > 0) activeNames += ", ";
            activeNames += footswitches[i].name;
        }
    }

    configDisplay.setTextSize(2);
    configDisplay.setTextDatum(TL_DATUM);
    configDisplay.setTextColor(primaryTextColor);
    configDisplay.drawString("ACTIVE EFFECTS: " + String(activeCount), 20, 90);

    if (activeNames.length() > 35) {
        activeNames = activeNames.substring(0, 32) + "...";
    }
    configDisplay.drawString(activeNames, 20, 120);

    // MIDI Channel info (uses footswitch 0 as original code did)
    configDisplay.setTextSize(3);
    configDisplay.setTextColor(accentColor);
    configDisplay.drawString("MIDI CH: " + String(footswitches[0].midiChannel), 20, 160);

    // Status indicator and navigation
    configDisplay.setTextSize(2);
    configDisplay.setTextColor(primaryTextColor);
    configDisplay.drawString("SYSTEM READY", 20, 200);

    configDisplay.setTextDatum(TL_DATUM);
    configDisplay.drawString("<< PREV BANK", 30, 280);
    configDisplay.setTextDatum(TR_DATUM);
    configDisplay.drawString("NEXT BANK >>", 450, 280);

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

// Small helper to show a centered message on a display
static void showCenteredMessage(MultiTFT &display, const char *message, uint16_t color, int y) {
    display.fillScreen(BLACK);
    display.drawRect(0, 0, 480, 320, WHITE);
    display.setTextDatum(MC_DATUM);
    display.setTextColor(color);
    display.setTextSize(4);
    display.drawString(message, 240, y);
}

// Show "CONFIGURING..." message on both displays
void showConfiguringMessage() {
    isConfiguring = true;
    configuringStartTime = millis();

    footswitchDisplay.select();
    showCenteredMessage(footswitchDisplay, "CONFIGURING...", YELLOW, 160);
    footswitchDisplay.deselect();

    configDisplay.select();
    showCenteredMessage(configDisplay, "CONFIGURING...", YELLOW, 160);
    configDisplay.deselect();
}

// Hide configuring message and restore normal displays
void hideConfiguringMessage() {
    isConfiguring = false;
    drawFootswitchScreen();
    drawConfigScreen();
}

// Show loading screen on both displays
void showLoadingScreen() {
    footswitchDisplay.select();
    footswitchDisplay.fillScreen(BLACK);
    footswitchDisplay.drawRect(0, 0, 480, 320, WHITE);
    footswitchDisplay.setTextDatum(MC_DATUM);
    footswitchDisplay.setTextColor(GREEN);
    footswitchDisplay.setTextSize(4);
    footswitchDisplay.drawString("LOADING...", 240, 120);
    footswitchDisplay.setTextColor(WHITE);
    footswitchDisplay.setTextSize(2);
    footswitchDisplay.drawString("MIDI Footswitch Controller", 240, 180);
    footswitchDisplay.setTextSize(1);
    footswitchDisplay.drawString("Initializing System...", 240, 220);
    footswitchDisplay.deselect();

    configDisplay.select();
    configDisplay.fillScreen(BLACK);
    configDisplay.drawRect(0, 0, 480, 320, WHITE);
    configDisplay.setTextDatum(MC_DATUM);
    configDisplay.setTextColor(GREEN);
    configDisplay.setTextSize(4);
    configDisplay.drawString("LOADING...", 240, 120);
    configDisplay.setTextColor(WHITE);
    configDisplay.setTextSize(2);
    configDisplay.drawString("MIDI Footswitch Controller", 240, 180);
    configDisplay.setTextSize(1);
    configDisplay.drawString("Initializing System...", 240, 220);
    configDisplay.deselect();
}