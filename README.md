# scpi-multimeter

Open source hardware-independent single-channel SCPI multimeter.

`SCPIMM` is a C-library containing:

* implementation of basic SCPI multimeter commands, such as `CONFigure`, `SENSe` and other;
* measurement state machine implementing `MEASure`, `INIT`, `FETCh` and relating commands;
* support of immediate, bus and external triggers.

## Steps to Implement a Multimeter Using `SCPIMM` Library

1. Implement callback functions providing access to measurement hardware.

For example, code below illustrates a `set_mode` callback function that switches our hardware to desired mode:
  
```C
scpimm_error_t set_mode_callback(scpimm_mode_t mode, const scpimm_mode_params_t* params) {
  if (SCPIMM_MODE_DCV == mode) {
    /* switch hardware to DC voltage mode */
  } else if (SCPIMM_MODE_ACV == mode) {
    /* switch hardware to AC voltage mode */
  } else {
    /* requested mode is not supported by hardware */
    return SCPIMM_ERROR_UNDEFINED_HEADER;
  }
  return SCPIMM_ERROR_OK;
}
```

2. Implement serial port access callback:

```C
size_t send_callback(const uint8_t* data, size_t len) {
  /* send data to serial port or another destination */
}
```

3. Populate `scpimm_interface_t` structure that holds pointers to all callbacks used by library.

4. Initialize library with filled `scpimm_interface_t` structure.

5. Run a loop that reads incoming data from serial port (or another source of data) and passes these data to library.

### Example

Here is a sample code illustrating usage of the library. Library declarations start with either `SCPIMM_` or `scpimm_` prefix.

```C
/* fill the structure with callback pointers */
const scpimm_interface_t interface = {
  .setup = setup_callback,
  .set_mode = set_mode_callback,
  .get_mode = get_mode_callback,
  .send = send_callback,
  ... /* other callbacks */
};

/* initialize SCPIMM library */
SCPIMM_setup(&interface);

/* main loop */
while ( !is_terminated() ) {
  if ( data_arrived_from_serial_port() ) {
    const char p = read_char_from_serial_port();
    /* parse incoming data */
    SCPIMM_parse_in_buffer(&p, 1);
  } else {
    /* run background tasks */
    SCPIMM_yield();
  }
}
```

## Library Dependencies

* [`scpi-parser`](https://github.com/andrey-nakin/scpi-parser) - generic SCPI parser.

## Library Usages

* [`v7-28-arduino`](https://github.com/andrey-nakin/v7-28-arduino) - firmware for Arduino Mega implementing a SCPI multimeter backed by V7-28 digital voltmeter.
This project can be treated as a reference implementation of SCPIMM multimeter.

