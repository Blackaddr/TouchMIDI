/*
 * Graphics.cpp
 *
 *  Created on: Mar. 2, 2019
 *      Author: blackaddr
 */
#include "Graphics.h"

constexpr float KNOB_SIZE_F = 0.10f;
constexpr float SWITCH_SIZE_F = 0.10f;
constexpr float ACTIVE_SIZE_F = max(KNOB_SIZE_F, SWITCH_SIZE_F) * 1.15f;
constexpr float DEG2RAD = 0.0174532925f;

// #########################################################################
// Draw an arc with a defined thickness
// #########################################################################

// x,y == coords of centre of arc
// start_angle = 0 - 359
// seg_count = number of 3 degree segments to draw (120 => 360 degree arc)
// rx = x axis radius
// yx = y axis radius
// w  = width (thickness) of arc in pixels
// colour = 16 bit colour value
// Note if rx and ry are the same an arc of a circle is drawn
void fillArc(ILI9341_t3 &tft, int x, int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour)
{
    byte seg = 3; // Segments are 3 degrees wide = 120 segments for 360 degrees
    byte inc = 3; // Draw segments every 3 degrees, increase to 6 for segmented ring

    // Draw colour blocks every inc degrees
    for (int i = start_angle; i < start_angle + seg * seg_count; i += inc) {
        // Calculate pair of coordinates for segment start
        float sx = cos((i - 90) * DEG2RAD);
        float sy = sin((i - 90) * DEG2RAD);
        uint16_t x0 = sx * (rx - w) + x;
        uint16_t y0 = sy * (ry - w) + y;
        uint16_t x1 = sx * rx + x;
        uint16_t y1 = sy * ry + y;

        // Calculate pair of coordinates for segment end
        float sx2 = cos((i + seg - 90) * DEG2RAD);
        float sy2 = sin((i + seg - 90) * DEG2RAD);
        int x2 = sx2 * (rx - w) + x;
        int y2 = sy2 * (ry - w) + y;
        int x3 = sx2 * rx + x;
        int y3 = sy2 * ry + y;

        tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
        tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);
    }
}

// x,y positions passed in should specify the center of the knob.
void drawKnob(ILI9341_t3 &tft, MidiControl &control, int16_t xPos, int16_t yPos)
{
    // Calculate knob radius in pixels based on smaller dimension
    unsigned knobRadius;
    if (tft.height() < tft.width()) {
        // calculate based on height
        knobRadius = static_cast<unsigned>(tft.height() * KNOB_SIZE_F + 0.5f); // includes rounding
    } else {
        // calculate based on width
        knobRadius = static_cast<unsigned>(tft.width() * KNOB_SIZE_F + 0.5f); // includes rounding
    }

    unsigned innerKnobRadious = static_cast<unsigned>(knobRadius*0.85f);

    tft.fillCircle(xPos, yPos, knobRadius, ILI9341_BLUE);
    tft.fillCircle(xPos, yPos, static_cast<int16_t>(knobRadius*0.85f), ILI9341_LIGHTGREY);

    // TODO : draw the indicator on the knob
    int angleOffset = map(control.value, 0, 127, 0, 300) - 240; // map to degress
    int xOffset0 = cos((angleOffset-4) * DEG2RAD) * innerKnobRadious;
    int yOffset0 = sin((angleOffset-4) * DEG2RAD) * innerKnobRadious;
    int xOffset1 = cos((angleOffset+4) * DEG2RAD) * innerKnobRadious;
    int yOffset1 = sin((angleOffset+4) * DEG2RAD) * innerKnobRadious;
    tft.fillTriangle(xPos,yPos, xPos+xOffset0, yPos+yOffset0, xPos+xOffset1, yPos+yOffset1, ILI9341_MAGENTA);

    tft.setTextColor(ILI9341_MAGENTA);
    printCenteredJustified(tft, control.shortName.c_str(), xPos, yPos-knobRadius-tft.getTextSize()*8);

    // If physically controlled, draw that label
    if (control.inputControl != InputControl::NOT_CONFIGURED) {
        tft.setTextColor(ILI9341_DARKGREEN);
        printCenteredJustified(tft, MidiControl::InputControlToString(control.inputControl),
                xPos, yPos-tft.getTextSize()*8/2);
    }
}

void redrawKnob(ILI9341_t3 &tft, MidiControl &control, int16_t xPos, int16_t yPos)
{
    // Calculate knob radius in pixels based on smaller dimension
    unsigned knobRadius;
    if (tft.height() < tft.width()) {
        // calculate based on height
        knobRadius = static_cast<unsigned>(tft.height() * KNOB_SIZE_F + 0.5f); // includes rounding
    } else {
        // calculate based on width
        knobRadius = static_cast<unsigned>(tft.width() * KNOB_SIZE_F + 0.5f); // includes rounding
    }

    unsigned innerKnobRadious = static_cast<unsigned>(knobRadius*0.85f);

    tft.fillCircle(xPos, yPos, static_cast<int16_t>(knobRadius*0.85f), ILI9341_LIGHTGREY);

    // TODO : draw the indicator on the knob
    int angleOffset = map(control.value, 0, 127, 0, 300) - 240; // map to degress
    int xOffset0 = cos((angleOffset-4) * DEG2RAD) * innerKnobRadious;
    int yOffset0 = sin((angleOffset-4) * DEG2RAD) * innerKnobRadious;
    int xOffset1 = cos((angleOffset+4) * DEG2RAD) * innerKnobRadious;
    int yOffset1 = sin((angleOffset+4) * DEG2RAD) * innerKnobRadious;
    tft.fillTriangle(xPos,yPos, xPos+xOffset0, yPos+yOffset0, xPos+xOffset1, yPos+yOffset1, ILI9341_MAGENTA);

    // Draw the knob label
    tft.setTextColor(ILI9341_MAGENTA);
    printCenteredJustified(tft, control.shortName.c_str(), xPos, yPos-knobRadius-tft.getTextSize()*8);

    // If physically controlled, draw that label
    if (control.inputControl != InputControl::NOT_CONFIGURED) {
        tft.setTextColor(ILI9341_DARKGREEN);
        printCenteredJustified(tft, MidiControl::InputControlToString(control.inputControl),
                xPos+knobRadius-tft.getTextSize()*8 , yPos-knobRadius-tft.getTextSize()*8);
    }


}

void drawSwitch(ILI9341_t3 &tft, MidiControl &control, int16_t xPos, int16_t yPos)
{
    // Set color OFF is dark grey, on is red.
    //uint16_t color = (control.value < 64) ? ILI9341_LIGHTGREY : ILI9341_RED;
    uint16_t color = (control.value == MIDI_ON_VALUE) ? ILI9341_RED : ILI9341_LIGHTGREY;

    // Calculate switch radius in pixels based on smaller dimension
    unsigned switchRadius;
    if (tft.height() < tft.width()) {
        // calculate based on height
        switchRadius = static_cast<unsigned>(tft.height() * SWITCH_SIZE_F + 0.5f); // includes rounding
    } else {
        // calculate based on width
        switchRadius = static_cast<unsigned>(tft.width() * SWITCH_SIZE_F + 0.5f); // includes rounding
    }
    tft.fillCircle(xPos, yPos, switchRadius, ILI9341_DARKGREY);
    tft.fillCircle(xPos, yPos, static_cast<int16_t>(switchRadius*0.75f), color);

    // Draw the label
    tft.setTextColor(ILI9341_MAGENTA);
    printCenteredJustified(tft, control.shortName.c_str(), xPos, yPos-switchRadius-tft.getTextSize()*8);

    // If physically controlled, draw that label
    if (control.inputControl != InputControl::NOT_CONFIGURED) {
        tft.setTextColor(ILI9341_DARKGREEN);
        printCenteredJustified(tft, MidiControl::InputControlToString(control.inputControl),
                xPos, yPos-tft.getTextSize()*8/2);
    }

}

void drawActiveControl(ILI9341_t3 &tft, int16_t xPos, int16_t yPos, uint16_t color)
{
    unsigned size;
    if (tft.height() < tft.width()) {
        // calculate based on height
        size = static_cast<unsigned>(tft.height() * ACTIVE_SIZE_F + 0.5f); // includes rounding
    } else {
        // calculate based on width
        size = static_cast<unsigned>(tft.width() * ACTIVE_SIZE_F + 0.5f); // includes rounding
    }
    //tft.fillRoundRect(xPos, yPos, size, size, 4, ILI9341_YELLOW);
    tft.fillRoundRect(xPos-size, yPos-size, 2*size, 2*size, 4, color);
}




