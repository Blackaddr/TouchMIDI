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
        reset();
    }

    ~ListDisplay() { free(updateLinePtr); }

    void next() {
        Serial.printf("next() selectedIndex: %d, selectedLine: %d, firstLine: %d, lastLine: %d lastIndex: %d\n", selectedIndex, selectedLine, firstLine, lastLine, lastIndex);
          if ((selectedLine == lastLine) || (selectedLine == NUM_LINES-1)) {
              // it's the last line in the display, check if it's also the last line
              // in the list
              if (selectedIndex < lastIndex) {
                  // the last line is selected and it's not the last line in the list
                  // advance the display group but don't change the selectedLine
                  firstLine++;
                  selectedIndex++;
                  lastLine = std::min(lastLine+1, lastIndex);
              } else {
                  // it is the last index, go back to the first
                  firstLine = 0;
                  lastLine = std::min(lastIndex, NUM_LINES-1);
                  selectedLine = 0;
                  selectedIndex = 0;
              }
              setUpdateAll();
          } else {
              // not the last line in the window, simply advance the selected line
              updateLinePtr[selectedLine]   = true;
              updateLinePtr[selectedLine+1] = true;
              selectedLine++;
              selectedIndex++;
          }
    }

    void previous()
    {
        Serial.printf("previous() selectedIndex: %d, selectedLine: %d, firstLine: %d, lastLine: %d lastIndex: %d\n", selectedIndex, selectedLine, firstLine, lastLine, lastIndex);
        if (selectedLine == 0) {
            // it's the first line in the display
            setUpdateAll();
            if (selectedIndex == 0) {
                // it the first index, roll back to the last
                lastLine = lastIndex;
                firstLine = std::max(static_cast<int>(lastLine)-static_cast<int>(NUM_LINES)+1, 0);
                selectedIndex = lastIndex;
                selectedLine = std::min(lastIndex, NUM_LINES-1);
            } else {
                // it's the first display line but not the first index, don't update the selectedLine
                firstLine--;
                lastLine--;
                selectedIndex--;
            }
        }  else {
            // it's not the first display line
            updateLinePtr[selectedLine] = true;
            selectedIndex--;
            selectedLine--;
            updateLinePtr[selectedLine] = true;
        }
    }

    bool getUpdate(unsigned line) {
        bool ret = updateLinePtr[line];
        updateLinePtr[line] = false;
        return ret;
    }

    unsigned getIndex(unsigned line) {
        return firstLine+line;
    }

    unsigned getFirstToDisplay() { return firstLine; }
    unsigned getLastToDisplay()  { return lastLine; }

    unsigned getSelected() { return selectedIndex; }

    void reset() {
        lastIndex = 0;
        firstLine = 0;
        selectedIndex  = 0;
        selectedLine = 0;
        lastLine = std::min(lastIndex, firstLine+NUM_LINES-1);
        updateLinePtr = (bool*)malloc(sizeof(bool)*NUM_LINES);
        for (unsigned i=0; i<NUM_LINES; i++) {
            updateLinePtr[i] = false;
        }
    }

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

    void setSelected(unsigned index) {
        selectedIndex = 0;
        setUpdateAll();
    }

private:
    const unsigned NUM_LINES;
    bool*    updateLinePtr = nullptr;
    unsigned lastIndex;
    unsigned selectedIndex  = 0;
    unsigned selectedLine = 0;
    unsigned firstLine = 0;
    unsigned lastLine  = 0;
};



#endif /* LISTDISPLAY_H_ */
