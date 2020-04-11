# TouchMIDI
Touchscreen MIDI Controller for Arduino Teensy

This project is an active work in progress:

April 2020
- (PARTIALLY DONE) add preset list scrolling
- update text edit screen to only update when necessary
- speed up flash saving
- transfer to onboard flash
- (DONE) move input control sampling to it's own thread
- (DONE) improve preset select nav screen by only redrawing the lines that needs updating.
- (DONE) Use updated filed on preset control to only draw that control. This will reduce overall flicker.
- (DONE) fix delay on Preset Control screen when using expression pedal. Drawing is slow, we need expression to be very fast!
- (DONE) Add "Savings" status screen
- (DONE) add SPACE to text editor
- (DONE) add touch to text editor
- (DONE) and switch ID to physically controlled buttons
- (DONE) centre text label over control

March 2019
- (DONE) preset saving now works. Need to fix the SAVE icon and speed up the SD writing. It's SLOW!

January 2019
- (DONE) currently working on the Preset Editor. Preset navigation works and the preset control screen permits touch control to select which knob/switch to modify.
