scpi-multimeter
===============

Open source implementation of generic 1-channel SCPI multimeter providing following functions:

* VOLTAGE:DC
* VOLTAGE:DC:RATIO
* VOLTAGE:AC
* CURRENT:DC
* CURRENT:AC
* RESISTANCE
* FRESISTANCE

Library contains implementation of SCPI commands and measurement state machine. It DOES NOT include a hardware-supporting module, but it provides an interface for low-level operations. Thus library is hardware-independent.

Library is based on j123b567/scpi-parser.

