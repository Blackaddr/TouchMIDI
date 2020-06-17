/*
 * listDisplay.h
 *
 *  Created on: Jun. 14, 2020
 *      Author: blackaddr
 */

#ifndef LISTDISPLAY_H_
#define LISTDISPLAY_H_

#include <algorithm>

class ListDisplay {

public:

    ListDisplay(const unsigned numLines) : NUM_LINES(numLines) {
        lastIndex = 0;
        firstLine = 0;
        lastLine = std::min(lastIndex, firstLine+NUM_LINES-1);
        updateLinePtr = (bool*)malloc(sizeof(bool)*NUM_LINES);
        for (unsigned i=0; i<NUM_LINES; i++) {
            updateLinePtr[i] = false;
        }
    }

    ~ListDisplay() { free(updateLinePtr); }

    void next() {
        Serial.printf("selected: %d, firstLine: %d lastIndex: %d\n", selected, firstLine, lastIndex);
          if (selected == lastLine) {
              // it's the last line in the display, check if it's also the last line
              // in the list
              if (selected < lastIndex) {
                  // the last line is select and it's not the last line in the list
                  // advance the display group
                  updateLinePtr[selected]   = true;
                  updateLinePtr[selected+1] = true;
                  firstLine++;
                  selected++;
                  lastLine++;
              } else {
                  // it is the last index, go back to the first
                  firstLine = 0;
                  lastLine = std::min(lastIndex, firstLine+NUM_LINES-1);
                  selected = 0;
                  for (unsigned i=0; i<NUM_LINES; i++) {
                      if (firstLine+i <= lastLine) { updateLinePtr[i] = true; }
                      else { updateLinePtr[i] = false; }
                  }
              }
          } else {
              // not the last line in the window, simply advance the selected line
              updateLinePtr[selected]   = true;
              updateLinePtr[selected+1] = true;
              selected++;
          }
    }

    void previous()
    {
        //Serial.printf("selected: %d, firstLine: %d lastLine: %d, lastIndex: %d\n", selected, firstLine, lastLine, lastIndex);
        if (selected == firstLine) {
            // it's the first line in the display
            if (selected == 0) {
                // it the first index, roll back to the last
                lastLine = lastIndex;
                firstLine = std::max(static_cast<int>(lastLine)-static_cast<int>(NUM_LINES)+1, 0);
                selected = lastIndex;
                for (unsigned i=0; i<NUM_LINES; i++) {
                    if (firstLine+i <= lastLine) { updateLinePtr[i] = true; }
                    else { updateLinePtr[i] = false; }
                }
            } else {
                // it's the first display line but not the first index
                firstLine--;
                lastLine--;
                updateLinePtr[selected] = true;
                selected--;
                updateLinePtr[selected] = true;
            }
        }  else {
            // it's not the first display line
            updateLinePtr[selected] = true;
            selected--;
            updateLinePtr[selected] = true;
        }
    }

    bool getUpdate(unsigned index) {
        bool ret = updateLinePtr[index];
        updateLinePtr[index] = false;
        return ret;
    }

    unsigned getIndex(unsigned index) {
        return firstLine+index;
    }

    unsigned getFirstToDisplay() { return firstLine; }
    unsigned getLastToDisplay()  { return lastLine; }

    unsigned getSelected() { return selected; }

    void setUpdate(unsigned index) {
        if ((index >= firstLine) && (index <= lastLine)) {
            updateLinePtr[index-firstLine] = true;
        }
    }

    void setUpdateAll() {
        for (unsigned i=0; i<NUM_LINES; i++) {
            if (firstLine+i <= lastLine) { updateLinePtr[i] = true; }
            else { updateLinePtr[i] = false; }
        }
    }

    void setSize(unsigned sizeIn) {
        lastIndex = sizeIn-1;
        lastLine = min(firstLine+NUM_LINES-1, lastIndex);
        for (unsigned i=0; i<NUM_LINES; i++) {
            if (firstLine+i <= lastLine) { updateLinePtr[i] = true; }
            else { updateLinePtr[i] = false; }
        }
    }

private:
    const unsigned NUM_LINES;
    bool*    updateLinePtr = nullptr;
    unsigned lastIndex;
    unsigned selected  = 0;
    unsigned firstLine = 0;
    unsigned lastLine  = 0;
};



#endif /* LISTDISPLAY_H_ */
