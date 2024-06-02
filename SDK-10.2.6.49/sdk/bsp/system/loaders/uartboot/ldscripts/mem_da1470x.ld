/* Linker script to configure memory regions.
 * Need modifying for a specific board.
 *   RAM.ORIGIN: starting address of read-write RAM area
 *   RAM.LENGTH: length of read-write RAM area
 *   BUFFER.ORIGIN: starting address of buffer area
 *   BUFFER.LENGTH: length of buffer area
 *   REBOOT.ORIGIN: place for function that moves buffer to beginning of RAM and starts binary
 *   REBOOT.LENGTH: length of reboot area
 */

MEMORY
{
        /* Access code + app data from CPU-C bus */
        RAM (rwx)    : ORIGIN = 0x00000000,     LENGTH = 256K
        /* Access buffer and reboot area from CPU-S bus */
        BUFFER (rw)  : ORIGIN = 0x20050000,     LENGTH = 892K
        REBOOT (rwx) : ORIGIN = 0x2012F000,     LENGTH = 4K
}
