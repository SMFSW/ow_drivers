# ARM OW drivers (release notes)

The MIT License (MIT)

Copyright (c) 2021-2024 SMFSW (Sebastien Bizien)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## v0.1

* DS1825: temperature sensor component added
* DS28E07: eeprom component added
* Control sequence refactored: skip or match rom can be selected
* Search function now uses a variable for search command (allowing to use same function with other search commands following devices on bus)
* OW_dev_temp: temperature sensor device type added
* OW_dev_eeprom: no more scratchpad passed as parameter in functions, using directly scratchpad pointer in structure 
* OW_dev_eeprom: read programming status byte polling increased to 2ms in OW_EEP_Copy_Scratchpad (avoiding too much parasite power loss issues)
* OW_dev_eeprom: test scratchpad size overflow when attempting to write scratchpad
* OW_dev_eeprom: some variables type changes to be able to handle any eeprom size
* OW_dev_eeprom: fix issue and read optimization for eeprom multiple blocks write
* use of size_t typedef for length parameters
* initial commit
