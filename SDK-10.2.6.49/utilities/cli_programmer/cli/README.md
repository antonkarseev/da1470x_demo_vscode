CLI programmer {#cli_programmer}
================================

## Overview

`cli_programmer` is a command line interface (CLI) memory programming tool.
CLI offers an extensive command set targeting all programmable memories a device can be equipped 
with (RAM, QSPI-based Flash, OTP). The supported commands' functionality include memory accessing 
(R/W), diagnostics, NVMS related functions, and application firmware programming of the actual
target device.

## Usage

To run the cli_programmer, the user has to specify the interface (GDB server or serial port) 
followed by the intended command name and its arguments.

    cli_programmer [<options>] <interface> <command> [<args>]

### Interface

* the serial port file name as presented by the operating system e.g. \b `COM5` (Windows), 
\b `/dev/ttyUSB0` (Linux), or
* \b `gdbserver`, if JTAG interface is to be used (J-Link debugger with the GDB server that must
first be initiated in another terminal session).

> Note: Writing an image to flash requires adding a header to the image.

### Commands and arguments

    write_qspi <address> <file> [<size>]

Writes up to `size` bytes of `file` into the FLASH/RAM at `address`. If `size` is omitted, the 
complete file is written.

    write_qspi_bytes <address> <data1> [<data2> [...]]

Writes bytes specified on command line into the FLASH/RAM at `address`.

    read_qspi <address> <file> <size>

Reads `size` bytes from the FLASH/RAM, starting at `address` into `file`. If `file` is specified as
either '-' or '--', data is output to stdout as hexdump. The hexdump is either 16-bytes (-) or
32-bytes (--) wide.

    erase_qspi <address> <size>

Erases `size` bytes of the FLASH, starting at `address`.
Note: The actual area erased may be bigger, due to the size of the erase block.

    chip_erase_qspi [<address>]

Completely erases the QSPI flash device starting at `address`.
Note: if no address is given, the flash memory connected to the first QSPI controller is erased.

    copy_qspi <address_ram> <address_qspi> <size>

Copies `size` bytes from the RAM memory, starting at `address_ram` to FLASH/RAM at `address_qspi`.
This is an advanced command and is not needed by the end user.

    is_empty_qspi [<start_address> <size>]

Checks if the FLASH contains only 0xFF values. If no arguments are specified, starting address is 0 
and size is 16MB.
Command prints whether flash is empty and, if not, the offset of first non empty byte.

    read_partition_table

Reads the partition table (if any exists) and prints its contents.

    read_partition <part_name|part_id> <address> <file> <size>

Reads `size` bytes from partition, selected by `part_name` or `part_id` according to the below
table, starting at `address` into `file`. If `file` is specified as either '-' or '--', data is 
output to stdout as hexdump. The hexdump is either 16-bytes (-) or 32-bytes (--) wide.

|         part_name         | part_id |
|---------------------------|---------|
|NVMS_FIRMWARE_PART         |    1    |
|NVMS_PARAM_PART            |    2    |
|NVMS_BIN_PART              |    3    |
|NVMS_LOG_PART              |    4    |
|NVMS_GENERIC_PART          |    5    |
|NVMS_PLATFORM_PARAMS_PART  |    15   |
|NVMS_PARTITION_TABLE       |    16   |
|NVMS_FW_EXEC_PART          |    17   |
|NVMS_FW_UPDATE_PART        |    18   |
|NVMS_PRODUCT_HEADER_PART   |    19   |
|NVMS_IMAGE_HEADER_PART     |    20   |

    write_partition <part_name|part_id> <address> <file> [<size>]

Writes up to `size` bytes of `file` into NVMS partition, selected by `part_name` or `part_id`
according to the above table, at `address`.
If `size` is omitted, the complete file is written.
If `file` is specified as either '-' or '--', data is output to stdout as hexdump. The hexdump is
either 16-bytes (-) or 32-bytes (--) wide.

    write_partition_bytes <part_name|part_id> <address> <data1> [<data2> [...]]

Writes bytes specified on command line into the NVMS partition, selected by `part_name` or `part_id`
according to the above table, at `address`.

    write <address> <file> [<size>]

Writes up to `size` bytes of `file` into the RAM memory at `address`. If `size` is omitted, the 
complete `file` is written.

    read <address> <file> <size>

Reads `size` bytes from the RAM memory, starting at `address` into `file`. If `file` is specified as
either '-' or '--', data is output to stdout as hexdump. The hexdump is either 16-bytes (-) or 
32-bytes (--) wide.

    write_otp <address> <length> [<data> [<data> [...]]]

Writes `length` words to the OTP at `address`. `data` are 32-bit words to be written. If less than
`length` words are specified, remaining words are assumed to be 0x00.

    write_otp_raw_file <address> <file> [<size>]

Writes up to `size` bytes of `file` into the OTP at `address`. If `size` is omitted, the complete
file is written. Remaining bytes in the last word are assumed to be 0x00.

    read_otp <address> <length>

Reads `length` 32-bit words from the OTP at `address`.

    write_otp_file <file>

Writes data to the OTP as defined in `file` (default specified values are written). `file` is a CSV
file separated by tabs. <b> `example_otp_file.csv` </b> is an example file which could be used with
this command. Most of columns in this file have only informational purpose. `cli_programmer` uses
only the 'Address', 'Size' and 'Default' columns, but the other columns are also needed for proper 
parsing of the file. A short description of each column is given below:

- <b> Address </b> - could be a cell address or cell address combined with `OTP_BASE` (0x07F80000)

- <b> Size </b> - number of bytes required by this item

- <b> Type </b> - type of item, e.g flag, region or byte

- <b> RW </b> or <b> RO </b> - read/write or read-only item (only for information)

- <b> ShortName </b> - name of item

- <b> Description </b> - short description about item. It could also contain option's values and
descriptions.

- <b> Default </b> - value which will be written to OTP

- <b> Number of Options </b> - this column contains a number of options written in the next columns.
Each option contains its description placed in a separate column as follows:
option 1 | description 1 | option 2 | description 2 | ...

    read_otp_file <file>

Reads data from the OTP as defined in `file` (cells with default value provided are read) contents 
of each cell is printed to stdout.

    boot <binary_file>

Boot application binary using the 1st stage bootloader (ROM booter) and then exit.
If the application is too big (more than 64kB) and serial interface is used, then the 'run' command
should be executed instead.

    run <binary_file>

Run application binary using the 2nd stage bootloader (uartboot) and then exit.
It supports bigger application binaries than 'boot' commands.
The limitation is the 'BUFFER' section size in the 'uartboot' application.

    get_product_info

Returns device classification and production information. The product information can serve as a
unique identifier that is readable and not editable by the application. The information is a
combination of device classification attributes (family, variant, chip ID, version) as stored in
designated device registers, production layout (package, wafer number, die coordinates) and testing 
information as stored in the device's OTP memory.

**_NOTE:_** The testing timestamp value is printed in raw hex format and equals to the seconds 
passed since Jan 1, 2009, 00:00:00 (UTC).

    write_oqspi <address> <file> [<size>]

Writes up to `size` bytes of `file` into the FLASH at `address`. If `size` is omitted, the complete
file is written.

    write_oqspi_bytes <address> <data1> [<data2> [...]]

Writes bytes specified on command line into the FLASH at `address`.

    read_oqspi <address> <file> <size>

Reads `size` bytes from the FLASH memory, starting at `address` into `file`. If `file` is specified
as either '-' or '--', data is output to stdout as hexdump. The hexdump is either 16-bytes (-) or 
32-bytes (--) wide.

    erase_oqspi <address> <size>

Erases `size` bytes of the FLASH, starting at `address`.
Note: The actual area erased may be bigger, due to the size of the erase block.

    chip_erase_oqspi [<address>]

Completely erases the OQSPI flash device starting at `address`.
Note: if no address is given, the flash memory connected to the OQSPI controller is erased.

    copy_oqspi <address_ram> <address_oqspi> <size>

Copies `size` bytes from the RAM memory, starting at `address_ram` to FLASH at `address_oqspi`.
This is an advanced command an is not needed by end user.

    is_empty_oqspi [<start_address> <size>]

Checks if the FLASH contains only 0xFF values. If no arguments are specified, starting address is 0
and size is 8MB.
Command prints whether flash is empty and, if not, the offset of first non empty byte.

    read_flash_info

Reads manufacturer ID as well as device type and density of all the available flash memories.

### General options

    -h

Prints help screen and exits.

    --save-ini

Saves CLI programmer configuration to the `cli_programmer.ini` file and exits.

    -b <file>

Filename of 2nd stage bootloader or application binary. In GDB Server interface mode, this could be
also the 'attach' keyword. This keyword omits platform reset and loading of bootloader binary.

    --trc <cmd>

Target reset command. May be used if there is a need to replace the default localhost reset command.
This option shouldn't be used with the '--check-booter-load' option - in such a case, it is ignored.

### GDB server specific options

    -p <port_num>

TCP port number that GDB server listens to. The Default value is 2331.

    -r <host>

GNU server host. The default is \`localhost\`.

    --no-kill [mode]

Don't stop running GDB Server instances. Available modes:
                                \'0\': Stop GDB Server instances during initialization and closing
                                \'1\': Don't stop GDB Server during initialization
                                \'2\': Don't stop GDB Server during closing
                                \'3\' or none: Don't stop any GDB Server instance (default)


    --gdb-cmd <cmd>

GDB server command used for executing and passing the right parameters to GDB server.
Without this parameter, no GDB server instance will be started or stopped. As GDB server command 
line can be quite long, it is recommended that it is stored in cli_programmer.ini file using 
--save-ini command line option.

    --check-booter-load

Don't force bootloader loading if it is running on the platform already. This option shouldn't be 
used with the '--trc' option - in such a case, the '--trc' option is ignored.

### Serial port specific options

    -s <baudrate>

Baud rate used for UART by uartboot. The parameter is patched to the uploaded uartboot binary
(in that way passed as a parameter). This can be 9600, 19200, 57600, 115200, 230400, 500000,
1000000 (default).

    -i <baudrate>

Initial baud rate used for uploading the uartboot or a user supplied binary. This depends on the
rate used by the bootloader of the device. The default behavior is to use the value passed by
'-s' or its default, if the parameter is not given. The argument is ignored by the `boot` command.
'-s' option should be used in this case.

    --tx-port <port_num>

GPIO port used for UART Tx by uartboot. This parameter is patched to the uploaded uartboot binary 
(in that way passed as a parameter). The default value is 0. The argument is ignored when the `boot`
command is given.  

    --tx-pin <pin_num>

GPIO pin used for UART Tx by uartboot. This parameter is patched to the uploaded uartboot binary
(in that way passed as a parameter). The default value is 8. The argument is ignored when the `boot`
command is given.

    --rx-port <port_num>

GPIO port used for UART Rx by uartboot. This parameter is patched to the uploaded uartboot binary
(in that way passed as a parameter). The default value is 2. The argument is ignored when the `boot`
command is given.

    --rx-pin <pin_num>

GPIO pin used for UART Rx by uartboot. This parameter is patched to the uploaded uartboot binary
(in that way passed as a parameter). The default value is 1. The argument is ignored when the `boot`
command is given.

    -w <timeout>

Serial port communication timeout is used only during download of uartboot binary. If the device does
not respond during this time, cli_programmer exits with timeout error.

### Configuration file

When cli_programmer is executed, it first tries to read cli_programmer.ini file which may contain 
various cli_programmer options. Instead of creating this file manually, user should use --save-ini 
command line option.
Format of cli_programmer.ini adheres to standard Windows ini file syntax.
The cli_programmer looks for ini file in the following locations:

- current directory
- home directory
- cli_programmer executable directory

## Usage examples

Read data from FLASH/RAM to local file.

    cli_programmer COM40 read_qspi 0x0 data_o 0x100

Upload custom binary `test_api.bin` to RAM and execute it.

    cli_programmer -b test_api.bin COM40 boot

Modify FLASH/RAM at specified location with arguments passed in command line.

    cli_programmer COM40 write_qspi_bytes 0x80000 0x11 0x22 0x33

Run a few commands with uartboot, using UART Tx/Rx P0_9/P2_2 at baud rate 115200 (initial rate for 
uartboot uploading is 9600).

    cli_programmer -i 9600 -s 115200 --tx-port 0 --tx-pin 9 --rx-port 2 --rx-pin 2 COM40 write_qspi 0x0 data_i
    cli_programmer -i 9600 -s 115200 --tx-port 0 --tx-pin 9 --rx-port 2 --rx-pin 2 COM40 read_qspi 0x0 data_o 0x100

Read FLASH/RAM contents (10 bytes at address 0x0).

    cli_programmer gdbserver read_qspi 0 -- 10

Write settings to the `cli_programmer.ini` file. Long bootloader path is passed with -b option and 
command line to start GDB server is passed with --gdb-cmd.
In this example GDB server command line contains arguments and path to executable has space so whole
command line is put in quotes and quotes required by Windows path are additionally escaped.

    cli_programmer -b c:\users\user\sdk\bsp\system\loaders\uartboot\Release\uartboot.bin --save-ini --gdb-cmd "\"C:\Program Files (x86)\SEGGER\JLink_V722b\JLinkGDBServerCL.exe\" -if SWD -device Cortex-M33 -singlerun -silent -speed auto"

Write OTP address 0x07f80128 with the following contents: B0:0x00, B1:0x01, B2:0x02, B3:0x03, B4:0x04, B5:0x05, B6:0x06, B7:0x07

     cli_programmer gdbserver write_otp 0x07f80128 2 0x03020100 0x07060504

Read OTP address 0x07f80128.

     cli_programmer gdbserver read_otp 0x07f80128 2

     If written with the contents from above write example, it should return
     0025   00 01 02 03 04 05 06 07   ........

## Building cli_programmer

- 'cli_programmer' makes use of the 'libprogrammer' library which implements the underlying 
functionality on the host side. 'cli_programmer' can be linked either statically or dynamically with
'libprogrammer'.

- 'cli_programmer' uses 'uartboot' application which acts as a secondary bootloader which 
cli_programmer downloads to the target for servicing the requested command's operations.

- The project and the relevant source code of the CLI are located under `utilities/cli_programmer/cli` folder
- The project and the relevant source code of the `uartboot` are located under `sdk/bsp/system/loaders/uartboot` folder

- Build configurations:

  * Release_static_linux    - Release version linked with a static version of libprogrammer - recommended for Linux.
                              It also builds uartboot project and includes it in cli_programmer executable.
  * Release_static_win32	- Release 32bit version for Windows linked with a static version of libprogrammer.
  * Release_static_win64	- Release 64bit version for Windows linked with a static version of libprogrammer.

- Build instructions:
  * Import `libprogrammer`, `cli_programmer` and `uartboot` into SmartSnippets Studio.
  * Build `libprogrammer` , `cli_programmer` and `uartboot`.
  * Run `cli_programmer` with proper parameters, as described in `Usage` and `Commands and arguments` sections.

> Notes:
> * A prebuilt version of `cli_programmer` can be found under SDK's `binaries` folder.
> * Building `cli_programmer` updates SDK's `binaries` folder (the new binaries are copied there).
> * Linux `cli_programmer` binaries built using the dynamic build configurations search for the library
file `libprogrammer.so` explicitly in the `binaries` folder.
