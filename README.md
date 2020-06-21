# TouchMIDI
Touchscreen MIDI Controller for Arduino Teensy

This project is an active work in progress:

June 2020

- automatic storing and recall of last active setlist and preset (flash remaining)
- add options to copy presets to/from flash/sd rather than whole flash contents.
- move file operations to a different thread than the GUI
- speed up flash saving (doesn't seem possible???)
- (DONE) support created and deletion of setlists on setlist screen
- (DONE) switch the nav screen over to the listDisplay class
- (DONE) support multiple setlists
- (DONE) move data files to 'data' directory and presets to 'presets' directory.

May 2020
- (DONE)fix string edit when touch button selections to clear previous letter
- (DONE)fix screen scrolling before first preset doesn't update
- (DONE) add touch to string edit letters
- (DONE)fix screen scrolling before first preset doesn't update
- (DONE) fix preset filenames past number 9
- (DONE) refactor control drawing on presetConfig
- (DONE) update PresetEdit screens to only update when necessary

April 2020
- (DONE) add copy from flash to SD to utils menu
- (DONE) fix knob rendering (text in bottom right corruption)
- (DONE) transfer to onboard flash
- (DONE) add SerialFlash utils to UTIL screen
- (PARTIALLY DONE) add preset list scrolling
- (DONE) update text edit screen to only update when necessary
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
