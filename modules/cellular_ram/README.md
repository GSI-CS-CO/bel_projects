# Cellular RAM

## Asynchronous Mode

- Asynchronous mode read access : 70 ns
- Power up time: 150 us
- The devices powers up in the asynchronous mode (default)
- During the initialization period, CE# should remain HIGH. When initialization is complete, the
device is ready for normal operation
- ISSI: ADV# can be held LOW during asynchronous Read and Write operations
- ISSI: CLK must be kept static Low during asynchronous Read/Write operations and Page Read access operations
- ISSI: WAIT is asserted and should be ignored during asynchronous and page mode operation
- Size: 256Mb -> A[23..0]
- Device: W968D6DA
