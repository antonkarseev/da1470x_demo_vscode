/**
 ****************************************************************************************
 *
 * @file cmd_handlers.c
 *
 * @brief Handling of CLI commands provided on command line
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <programmer.h>
#include "cli_common.h"

// NOTE: Currently set to 16 MB to match the AT25SL128 flash memory; set to 32 MB to match W25Q256JW
#define DEFAULT_SIZE 0x1000000//0x2000000

// NOTE: Currently set to 8 MB to match MX25U6432 and W25Q64JWIM flash memories; set to 128 MB to match MX66UM1G45G
#define OQSPI_DEFAULT_SIZE      0x1000000//0x8000000

/* Maximum size for an image */
#define MAX_IMAGE_SIZE 0x7F000


#define OQSPI_MEM1_VIRTUAL_BASE_ADDR    (0x00000000)
#define QSPI_MEM1_VIRTUAL_BASE_ADDR     (0x8000000UL)

#define ADESTO_ID       0x1F
#define GIGADEVICE_ID   0xC8
#define MACRONIX_ID     0xC2
#define WINBOND_ID      0xEF

/* Pointer will hold memory for executable application code */
static uint8_t *executable_bin;
/* Executable application size */
static size_t executable_size;

static int cmdh_write(int argc, char *argv[]);
static int cmdh_read(int argc, char *argv[]);
static int cmdh_write_qspi(int argc, char *argv[]);
static int cmdh_write_qspi_bytes(int argc, char *argv[]);
static int cmdh_read_qspi(int argc, char *argv[]);
static int cmdh_read_partition_table(int argc, char *argv[]);
static int cmdh_read_partition(int argc, char *argv[]);
static int cmdh_write_partition(int argc, char *argv[]);
static int cmdh_write_partition_bytes(int argc, char *argv[]);
static int cmdh_erase_qspi(int argc, char *argv[]);
static int cmdh_chip_erase_qspi(int argc, char *argv[]);
static int cmdh_copy_qspi(int argc, char *argv[]);
static int cmdh_is_empty_qspi(int argc, char *argv[]);
static int cmdh_write_otp(int argc, char *argv[]);
static int cmdh_read_otp(int argc, char *argv[]);
static int cmdh_write_otp_file(int argc, char *argv[]);
static int cmdh_write_otp_raw_file(int argc, char *argv[]);
static int cmdh_read_otp_file(int argc, char *argv[]);
static int cmdh_write_tcs(int argc, char *argv[]);
static int cmdh_boot(int argc, char *argv[]);
static int cmdh_run(int argc, char *argv[]);
static int cmdh_get_product_info(int argc, char *argvp[]);
static int cmdh_write_oqspi(int argc, char *argv[]);
static int cmdh_write_oqspi_bytes(int argc, char *argv[]);
static int cmdh_read_oqspi(int argc, char *argv[]);
static int cmdh_erase_oqspi(int argc, char *argv[]);
static int cmdh_chip_erase_oqspi(int argc, char *argv[]);
static int cmdh_copy_oqspi(int argc, char *argv[]);
static int cmdh_is_empty_oqspi(int argc, char *argv[]);
static int cmdh_read_flash_info(int argc, char *argv[]);

/**
 * \brief CLI command handler description
 *
 */
struct cli_command {
        const char *name;                       /**< name of command */
        int min_num_p;                          /**< minimum number of parameters */
        int (* func) (int argc, char *argv[]);  /**< handler function, return non-zero for success */
};

/**
 * \brief CLI command handlers
 *
 */
static struct cli_command cmds[] = {
        { "write",                 2, cmdh_write, },
        { "read",                  3, cmdh_read, },
        { "write_qspi",            2, cmdh_write_qspi, },
        { "write_qspi_bytes",      2, cmdh_write_qspi_bytes, },
        { "read_qspi",             3, cmdh_read_qspi, },
        { "erase_qspi",            2, cmdh_erase_qspi, },
        { "chip_erase_qspi",       0, cmdh_chip_erase_qspi, },
        { "read_partition_table",  0, cmdh_read_partition_table, },
        { "read_partition",        4, cmdh_read_partition, },
        { "write_partition",       3, cmdh_write_partition, },
        { "write_partition_bytes", 3, cmdh_write_partition_bytes, },
        { "copy_qspi",             3, cmdh_copy_qspi, },
        { "is_empty_qspi",         0, cmdh_is_empty_qspi, },
        { "write_otp",             2, cmdh_write_otp, },
        { "read_otp",              2, cmdh_read_otp, },
        { "write_otp_file",        1, cmdh_write_otp_file, },
        { "write_otp_raw_file",    2, cmdh_write_otp_raw_file, },
        { "read_otp_file",         1, cmdh_read_otp_file, },
        { "write_tcs",             3, cmdh_write_tcs },
        { "boot",                  1, cmdh_boot, },
        { "run",                   1, cmdh_run, },
        { "get_product_info",      0, cmdh_get_product_info, },
        { "write_oqspi",           2, cmdh_write_oqspi, },
        { "write_oqspi_bytes",     2, cmdh_write_oqspi_bytes, },
        { "read_oqspi",            3, cmdh_read_oqspi, },
        { "erase_oqspi",           2, cmdh_erase_oqspi, },
        { "chip_erase_oqspi",      0, cmdh_chip_erase_oqspi, },
        { "copy_oqspi",            3, cmdh_copy_oqspi, },
        { "is_empty_oqspi",        0, cmdh_is_empty_oqspi, },
        {"read_flash_info",        0, cmdh_read_flash_info, },
        /* end of table */
        { NULL, 0, NULL, }
};

static int get_filesize(const char *fname)
{
        struct stat st;

        if (stat(fname, &st) < 0) {
                return -1;
        }

        return st.st_size;
}

static bool check_otp_cell_address(uint32_t *addr)
{
        uint32_t _addr = *addr;
        uint32_t otp_base_mem = 0x10080000;
        uint32_t max_cell = 0x400;
        uint8_t  addr_to_cel_div = 2; /*4 bytes per cell */


/* convert mapped address to cell address, if possible */
        if ((_addr & otp_base_mem) == otp_base_mem) {
                _addr = (_addr & 0xFFFF) >> addr_to_cel_div;
        }

        if (_addr >= max_cell) {
                return false;
        }

        *addr = _addr;

        return true;
}

static int cmdh_write(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[1];
        unsigned int size = 0;
        int ret;
        const char *chip_rev;
        const prog_memory_sizes_t *sizes;

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (argc > 2) {
                if (!get_number(argv[2], &size) || !size) {
                        prog_print_err("invalid size\n");
                        return 0;
                }
        } else {
                int file_size = get_filesize(fname);

                if (file_size < 0) {
                        prog_print_err("could not open file\n");
                        return 0;
                }

                size = file_size;
        }

        prog_get_chip_rev(&chip_rev);
        prog_get_memory_sizes(chip_rev, &sizes);
        if (size > sizes->ram_size) {
                prog_print_err("invalid size exceeding RAM size\n");
                return 0;
        }

        ret = prog_write_file_to_ram(addr, fname, size);
        if (ret) {
                prog_print_err("write to RAM failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        return 1;
}

static int cmdh_read(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[1];
        unsigned int size = 0;
        uint8_t *buf = NULL;
        int ret;
        const char *chip_rev;
        const prog_memory_sizes_t *sizes;

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (!get_number(argv[2], &size) || !size) {
                prog_print_err("invalid size\n");
                return 0;
        }

        prog_get_chip_rev(&chip_rev);
        prog_get_memory_sizes(chip_rev, &sizes);
        if (size > sizes->ram_size) {
                prog_print_err("invalid size exceeding RAM size\n");
                return 0;
        }

        if (!strcmp(fname, "-") || !strcmp(fname, "--")) {
                buf = malloc(size);
                ret = prog_read_memory(addr, buf, size);
        } else {
                ret = prog_read_memory_to_file(addr, fname, size);
        }
        if (ret) {
                free(buf);
                prog_print_err("read from RAM failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        if (buf) {
                dump_hex(addr, buf, size, !strcmp(fname, "--") ? 32 : 16);
                free(buf);
        }

        return 1;
}

static int cmdh_write_qspi(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[1];
        unsigned int size = 0;
        unsigned int size_limit;
        int ret;
        const char *chip_rev;
        const prog_memory_sizes_t *sizes;
        const char *mem_name = "QSPI";

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (argc > 2) {
                if (!get_number(argv[2], &size) || !size) {
                        prog_print_err("invalid size\n");
                        return 0;
                }
        } else {
                int file_size = get_filesize(fname);

                if (file_size < 0) {
                        prog_print_err("could not open file\n");
                        return 0;
                }

                size = file_size;
        }

        prog_get_chip_rev(&chip_rev);
        prog_get_memory_sizes(chip_rev, &sizes);
        size_limit = sizes->qspi_size;


        if (size > size_limit) {
                prog_print_err("invalid size exceeding %s size\n", mem_name);
                return 0;
        }

        ret = prog_write_file_to_qspi(addr, fname, size);
        if (ret) {
                prog_print_err("write to %s failed: %s (%d)\n", mem_name, prog_get_err_message(ret),
                                                                                                ret);
                return 0;
        }

        return 1;
}

static int cmdh_write_qspi_bytes(int argc, char *argv[])
{
        unsigned int addr;
        int ret;
        int i;
        unsigned int b;
        uint8_t *buf = (uint8_t *) malloc(argc);

        if (buf == NULL) {
                return 0;
        }

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address %s\n", argv[0]);
                goto end;
        }

        for (i = 1; i < argc; ++i) {
                if (!get_number(argv[i], &b)) {
                        prog_print_err("invalid byte '%s'\n", argv[i]);
                        goto end;
                }
                buf[i - 1] = (uint8_t) b;
        }

        ret = prog_write_to_qspi(addr, buf, argc - 1);
        if (ret) {
                const char *mem_name = "QSPI";

                prog_print_err("write to %s failed: %s (%d)\n", mem_name, prog_get_err_message(ret),
                                                                                                ret);
                goto end;
        }
end:
        free(buf);

        return 1;
}


static int cmdh_read_qspi(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[1];
        unsigned int size = 0;
        unsigned int size_limit;
        uint8_t *buf = NULL;
        int ret;
        const char *chip_rev;
        const prog_memory_sizes_t *sizes;
        const char *mem_name = "QSPI";

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (!get_number(argv[2], &size) || !size) {
                prog_print_err("invalid size\n");
                return 0;
        }

        prog_get_chip_rev(&chip_rev);
        prog_get_memory_sizes(chip_rev, &sizes);
        size_limit = sizes->qspi_size;


        if (size > size_limit) {
                prog_print_err("invalid size exceeding %s size\n", mem_name);
                return 0;
        }

        if (!strcmp(fname, "-") || !strcmp(fname, "--")) {

                buf = malloc(size);
                if (!buf) {
                        ret = ERR_ALLOC_FAILED;
                } else {
                        ret = prog_read_qspi(addr, buf, size);
                }
        } else {
                ret = prog_read_qspi_to_file(addr, fname, size);
        }
        if (ret) {
                free(buf);
                prog_print_err("read from %s failed: %s (%d)\n", mem_name, prog_get_err_message(ret),
                                                                                                ret);
                return 0;
        }

        if (buf) {
                dump_hex(addr, buf, size, !strcmp(fname, "--") ? 32 : 16);
                free(buf);
        }

        return 1;
}

static int cmdh_erase_qspi(int argc, char *argv[])
{
        unsigned int addr;
        unsigned int size;
        int ret;

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (!get_number(argv[1], &size)) {
                prog_print_err("invalid size\n");
                return 0;
        }

        ret = prog_erase_qspi(addr, size);
        if (ret) {
                const char *mem_name = "QSPI";
                prog_print_err("erase %s failed: %s (%d)\n", mem_name, prog_get_err_message(ret),
                                                                                                ret);
                return 0;
        }

        return 1;
}

static int cmdh_read_partition_table(int argc, char *argv[])
{
        int ret = 0;
        unsigned int size = 0;
        uint8_t *buf = NULL;

        ret = prog_read_partition_table(&buf, &size);
        if (ret) {
                prog_print_err("read partition table failed: %s (%d)\n", prog_get_err_message(ret),
                                                                                                ret);
                goto done;
        }

        ret = dump_partition_table(buf, size);

done:
        free(buf);
        return (ret == 0);
}

static int cmdh_read_partition(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[2];
        unsigned int size = 0;
        uint8_t *buf = NULL;
        unsigned int id;
        int ret;

        if (!is_valid_partition_name(argv[0], &id)) {
                if (!get_number(argv[0], &id) || !is_valid_partition_id(id)) {
                        prog_print_err("invalid partition name/id or selected partition doesn't "
                                                                                        "exist\n");
                        return 0;
                }
        }

        if (!get_number(argv[1], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (!get_number(argv[3], &size) || !size) {
                prog_print_err("invalid size\n");
                return 0;
        }

        if (size > get_partition_size(id)) {
                prog_print_err("invalid size exceeding partition size\n");
                return 0;
        }

        if (!strcmp(fname, "-") || !strcmp(fname, "--")) {
                buf = malloc(size);
                if (!buf) {
                        ret = ERR_ALLOC_FAILED;
                } else {
                        ret = prog_read_partition(id, addr, buf, size);
                }
        } else {
                ret = prog_read_patrition_to_file(id, addr, fname, size);
        }

        if (ret) {
                free(buf);
                prog_print_err("read from partition failed (%d)\n", ret);
                return 0;
        }

        if (buf) {
                dump_hex(addr, buf, size, !strcmp(fname, "--") ? 32 : 16);
                free(buf);
        }

        return 1;
}

static int cmdh_write_partition(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[2];
        unsigned int size = 0;
        unsigned int id;
        int ret;

        if (!is_valid_partition_name(argv[0], &id)) {
                if (!get_number(argv[0], &id) || !is_valid_partition_id(id)) {
                        prog_print_err("invalid partition name/id\n");
                        return 0;
                }
        }

        if (!get_number(argv[1], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (argc > 3) {
                if (!get_number(argv[3], &size) || !size) {
                        prog_print_err("invalid size\n");
                        return 0;
                }
        } else {
                int file_size = get_filesize(fname);

                if (file_size < 0) {
                        prog_print_err("could not open file\n");
                        return 0;
                }

                size = file_size;
        }

        if (size > get_partition_size(id)) {
                prog_print_err("invalid size exceeding partition size\n");
                return 0;
        }

        ret = prog_write_file_to_partition(id, addr, fname, size);
        if (ret) {
                prog_print_err("write to partition failed: %s (%d)\n", prog_get_err_message(ret),
                                                                                                ret);
                return 0;
        }

        return 1;
}

static int cmdh_write_partition_bytes(int argc, char *argv[])
{
        unsigned int addr;
        unsigned int id;
        int ret = 0;
        int i;
        unsigned int b;
        uint8_t *buf = (uint8_t *) malloc(argc - 2);

        if (buf == NULL) {
                return ERR_ALLOC_FAILED;
        }

        if (!is_valid_partition_name(argv[0], &id)) {
                if (!get_number(argv[0], &id) || !is_valid_partition_id(id)) {
                        prog_print_err("invalid partition name/id\n");
                        goto end;
                }
        }

        if (!get_number(argv[1], &addr)) {
                prog_print_err("invalid address %s\n", argv[0]);
                goto end;
        }

        for (i = 2; i < argc; ++i) {
                if (!get_number(argv[i], &b)) {
                        prog_print_err("invalid byte '%s'\n", argv[i]);
                        goto end;
                }
                buf[i - 2] = (uint8_t) b;
        }

        ret = prog_write_partition(id, addr, buf, argc - 2);
        if (ret) {
                prog_print_err("write to partition failed: %s (%d)\n", prog_get_err_message(ret),
                                                                                                ret);
                ret = 0;
                goto end;
        }
        ret = 1;

end:
        free(buf);

        return ret;
}

static int cmdh_copy_qspi(int argc, char *argv[])
{
        unsigned int addr_ram;
        unsigned int addr_qspi;
        unsigned int size;
        int ret;

        if (!get_number(argv[0], &addr_ram)) {
                prog_print_err("invalid RAM address\n");
                return 0;
        }

        if (!get_number(argv[1], &addr_qspi)) {
                prog_print_err("invalid QSPI address\n");
                return 0;
        }

        if (!get_number(argv[2], &size)) {
                prog_print_err("invalid size\n");
                return 0;
        }

        ret = prog_copy_to_qspi(addr_ram, addr_qspi, size);
        if (ret) {
                const char *mem_name = "QSPI";
                prog_print_err("copy to %s failed: %s (%d)\n", mem_name, prog_get_err_message(ret),
                                                                                                ret);
                return 0;
        }

        return 1;
}

static int cmdh_is_empty_qspi(int argc, char *argv[])
{
        unsigned int size = DEFAULT_SIZE;
        unsigned int start_address = 0;
        int ret_number;
        int ret = 0;
        const char *mem_name = "QSPI";


        if (argc != 0 && argc != 2) {
                prog_print_err("invalid argument - function is_empty_qspi needs zero or two"
                                                                                "arguments\n");
                return 0;
        }

        if (argc == 2) {
                if (!get_number(argv[0], &start_address)) {
                        prog_print_err("invalid start address\n");
                        return 0;
                }

                if (!get_number(argv[1], &size) || size == 0) {
                        prog_print_err("invalid size\n");
                        return 0;
                }
        }

        ret = prog_is_empty_qspi(size, start_address, &ret_number);


        if (!ret) {
                if (ret_number <= 0) {
                        prog_print_log("%s flash region is not empty (byte at 0x%08x + 0x%08x is not "
                                        "0xFF).\n", mem_name, start_address, (-1 * ret_number));
                } else {
                        prog_print_log("%s flash region is empty (checked %u bytes).\n", mem_name,
                                                                                        ret_number);
                }
                ret = 1;
        } else {
                prog_print_err("check %s emptiness failed: %s (%d)\n", mem_name,
                                                                prog_get_err_message(ret), ret);
                ret = 0;
        }

        return ret;
}

static int cmdh_chip_erase_qspi(int argc, char *argv[])
{
        unsigned int addr = QSPI_MEM1_VIRTUAL_BASE_ADDR;
        int ret;

        if (argc != 0 && argc != 1) {
                prog_print_err("invalid argument - chip_erase_qspi takes zero or one argument\n");
                return 0;
        }

        if (argc == 1) {
                if (!get_number(argv[0], &addr)) {
                        prog_print_err("invalid start address\n");
                        return 0;
                }

                if (*argv[0] != QSPI_MEM1_VIRTUAL_BASE_ADDR) {
                        prog_print_err("wrong start address - use 0x%lx instead\n", QSPI_MEM1_VIRTUAL_BASE_ADDR);
                        return 0;
                }
        }

        ret = prog_chip_erase_qspi_by_addr(addr);

        if (ret) {
                prog_print_err("chip erase QSPI failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        return 1;
}

static int cmdh_write_otp(int argc, char *argv[])
{
        unsigned int addr;
        unsigned int length = 0;
        uint32_t *buf = NULL;
        int i;
        int ret = 0;
        const char *chip_rev;
        const prog_memory_sizes_t *sizes;

        if (!get_number(argv[0], &addr) || !check_otp_cell_address(&addr)) {
                prog_print_err("invalid address\n");
                goto done;
        }

        if (!get_number(argv[1], &length) || !length) {
                prog_print_err("invalid length\n");
                goto done;
        }

        argc -= 2;
        argv += 2;

        prog_get_chip_rev(&chip_rev);
        prog_get_memory_sizes(chip_rev, &sizes);
        if ((length + addr) > sizes->otp_size >> 2) {
                prog_print_err("invalid length exceeding OTP size\n");
                goto done;
        }

        if ((buf = (uint32_t *) calloc(length, sizeof(*buf))) == NULL) {
                goto done;
        }

        for (i = 0; i < argc && i < length; i++) {
                if (!get_number(argv[i], &buf[i])) {
                        prog_print_err("invalid data (#%d)\n", i + 1);
                        goto done;
                }
        }

        ret = prog_write_otp(addr, buf, length);
        if (ret) {
                prog_print_err("write to OTP failed: %s (%d)\n", prog_get_err_message(ret), ret);
                ret = 0;
                goto done;
        }

        ret = 1;

done:
        if (buf) {
                free(buf);
        }
        return ret;
}

static int cmdh_read_otp(int argc, char *argv[])
{
        unsigned int addr;
        unsigned int length = 0;
        uint32_t *buf = NULL;
        int ret = 0;
        const char *chip_rev;
        const prog_memory_sizes_t *sizes;

        if (!get_number(argv[0], &addr) || !check_otp_cell_address(&addr)) {
                prog_print_err("invalid address\n");
                goto done;
        }

        if (!get_number(argv[1], &length) || !length) {
                prog_print_err("invalid length\n");
                goto done;
        }

        prog_get_chip_rev(&chip_rev);
        prog_get_memory_sizes(chip_rev, &sizes);
        if ((length + addr) > sizes->otp_size >> 2) {
                prog_print_err("invalid length exceeding OTP size\n");
                goto done;
        }

        if ((buf = (uint32_t *) calloc(length, sizeof(*buf))) == NULL) {
                goto done;
        }

        ret = prog_read_otp(addr, buf, length);
        if (ret) {
                prog_print_err("read from OTP failed: %s (%d)\n", prog_get_err_message(ret), ret);
                goto done;
        }

        dump_otp(addr, buf, length);
        ret = 1;
done:
        if (buf) {
                free(buf);
        }
        return ret;
}
static bool write_otp_file_value_cb(uint32_t addr, uint32_t size, uint64_t value)
{
        uint8_t *buf;
        unsigned int i;
        int ret;

        buf = calloc(1, size);
        if (!buf) {
                return false;
        }

        memcpy(buf, &value, size < sizeof(value) ? size : sizeof(value));

        prog_print_log("write_otp %04x %d ", addr, size);
        for (i = 0; i < size; i++) {
                prog_print_log("%02X", buf[i]);
        }

        ret = prog_write_otp(addr, (void *) buf, size / 4);
        if (ret < 0) {
                prog_print_log(" (FAILED: %s (%d))\n", prog_get_err_message(ret), ret);
                free(buf);
                return false;
        }

        free(buf);
        prog_print_log(" (OK)\n");

        return true;
}

int cmdh_write_otp_file(int argc, char *argv[])
{
        return parse_otp_file(argv[0], write_otp_file_value_cb);
}

static int cmdh_write_otp_raw_file(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[1];
        unsigned int size = 0;
        int ret;

        if (!get_number(argv[0], &addr) || !check_otp_cell_address(&addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (argc > 2) {
                if (!get_number(argv[2], &size)) {
                        prog_print_err("invalid size\n");
                        return 0;
                }
        } else {
                int file_size = get_filesize(fname);

                if (file_size < 0) {
                        prog_print_err("could not open file\n");
                        return 0;
                }

                size = file_size;
        }

        ret = prog_write_file_to_otp(addr, fname, size);
        if (ret) {
                prog_print_err("write to OTP failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        return 1;
}

static bool read_otp_file_value_cb(uint32_t addr, uint32_t size, uint64_t value)
{
        uint8_t *buf;
        int ret;

        buf = calloc(1, size);
        if (!buf) {
                return false;
        }

        prog_print_log("read_otp %04x %d ", addr, size);

        ret = prog_read_otp(addr, (void *) buf, size / 4);
        if (ret < 0) {
                prog_print_log(" (FAILED: %s (%d))\n", prog_get_err_message(ret), ret);
                free(buf);
                return false;
        }

        prog_print_log(" (OK)\n");

        dump_otp(addr, (void *) buf, size / 4);

        free(buf);

        return true;
}

int cmdh_read_otp_file(int argc, char *argv[])
{
        return parse_otp_file(argv[0], read_otp_file_value_cb);
}

static int cmdh_write_tcs(int argc, char *argv[])
{
        unsigned int length;
        uint32_t *buf = NULL;
        unsigned int i;
        uint32_t address;
        int ret = 0;

        if (!get_number(argv[0], &length) || !length) {
                prog_print_err("invalid length\n");
                goto done;
        }

        argc -= 1;
        argv += 1;

        if (argc != length) {
                prog_print_err("invalid length. provided data does not match length\n");
                goto done;
        }
        if (length & 0x01) {
                prog_print_err("invalid length. TCS entries need to be in pairs\n");
                goto done;
        }
        if (length > TCS_WORD_SIZE) {
                prog_print_err("invalid length. length is bigger than TCS size\n");
                goto done;
        }

        length <<= 1;

        if ((buf = (uint32_t *) calloc(length, sizeof(*buf))) == NULL) {
                goto done;
        }

        /*Create data + complement data for TCS*/
        for (i = 0; i < (unsigned int)argc; i++) {
                if (!get_number(argv[i], &buf[2*i])) {
                        prog_print_err("invalid data (#%d)\n", i + 1);
                        goto done;
                }
                buf[2*i+1]=~buf[2*i];//calculate the complement
        }

        ret = prog_write_tcs(&address, buf, length);
        if (ret) {
                prog_print_err("write to OTP TCS failed: %s (%d)\n", prog_get_err_message(ret), ret);
                goto done;
        }
        prog_print_log("TCS contents written: \n");
        dump_otp(address, buf, length);
        ret = 1;
done:
        if (buf) {
                free(buf);
        }
        return ret;
}

static int set_executable_from_file(const char *file_name)
{
        FILE *f;
        struct stat st;
        int err = 0;

        if (file_name == NULL || stat(file_name, &st) < 0) {
                return ERR_FILE_OPEN;
        }

        f = fopen(file_name, "rb");
        if (f == NULL) {
                return ERR_FILE_OPEN;
        }

        executable_bin = realloc(executable_bin, st.st_size);
        if (executable_bin == NULL) {
                err = ERR_ALLOC_FAILED;
                goto end;
        }

        if (st.st_size != fread(executable_bin, 1, st.st_size, f)) {
                err = ERR_FILE_READ;
                goto end;
        }

        if (st.st_size > 0) {
                executable_size = st.st_size;
        } else {
                err = ERR_FILE_EMPTY;
        }
end:
        if (f != NULL) {
                fclose(f);
        }

        return err;
}

int cmdh_boot(int argc, char *argv[])
{
        int ret;

        ret = set_executable_from_file(argv[0]);
        if (ret < 0) {
                prog_print_err("failed to set executable file: %s (%d)\n",
                                                                prog_get_err_message(ret), ret);
                return ret;
        }

        ret = prog_boot(executable_bin, executable_size);
        if (ret < 0) {
                prog_print_err("failed to boot executable: %s (%d)\n",
                                                                prog_get_err_message(ret), ret);
                return ret;
        }

        return 1;
}

int cmdh_run(int argc, char *argv[])
{
        int ret;

        ret = set_executable_from_file(argv[0]);
        if (ret < 0) {
                prog_print_err("failed to set executable file: %s (%d)\n",
                                                                prog_get_err_message(ret), ret);
                return ret;
        }

        ret = prog_run(executable_bin, executable_size);
        if (ret < 0) {
                prog_print_err("failed to run executable: %s (%d)\n",
                                                                prog_get_err_message(ret), ret);
                return ret;
        }

        return 1;
}

/* Example of product information output:
         * Device classification attributes:
         * Device family: DA1469x
         * Device chip ID: D2522
         * Device variant: DA14695
         * Device version (revision|step): AB
         *
         * Production layout information:
         * Package = VFBGA86
         *
         * Production testing information:
         * Timestamp = 0x12DCED46
         */
int cmdh_get_product_info(int argc, char *argv[])
{
        int ret = 0;
        unsigned int size = 0;
        uint8_t *buf = NULL;

        ret = prog_get_product_info(&buf, &size);
        if (ret) {
                prog_print_err("get product info failed: %s (%d)\n", prog_get_err_message(ret), ret);
                goto done;
        }

        ret = dump_product_info(buf, size);

done:
        free(buf);
        return (ret == 0);
}


static int cmdh_write_oqspi(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[1];
        unsigned int size = 0;
        unsigned int size_limit;
        int ret;
        const char *chip_rev;
        const prog_memory_sizes_t *sizes;
        const char *mem_name = "OQSPI";

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (argc > 2) {
                if (!get_number(argv[2], &size) || !size) {
                        prog_print_err("invalid size\n");
                        return 0;
                }
        } else {
                int file_size = get_filesize(fname);

                if (file_size < 0) {
                        prog_print_err("could not open file\n");
                        return 0;
                }

                size = file_size;
        }

        prog_get_chip_rev(&chip_rev);
        prog_get_memory_sizes(chip_rev, &sizes);
        size_limit = sizes->oqspi_size;

        if (size > size_limit) {
                prog_print_err("invalid size exceeding %s size\n", mem_name);
                return 0;
        }

        ret = prog_write_file_to_oqspi(addr, fname, size);
        if (ret) {
                prog_print_err("write to %s failed: %s (%d)\n", mem_name, prog_get_err_message(ret),
                                                                                                ret);
                return 0;
        }

        return 1;
}

static int cmdh_write_oqspi_bytes(int argc, char *argv[])
{
        unsigned int addr;
        int ret;
        int i;
        unsigned int b;
        uint8_t *buf = (uint8_t *) malloc(argc);

        if (buf == NULL) {
                return 0;
        }

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address %s\n", argv[0]);
                goto end;
        }

        for (i = 1; i < argc; ++i) {
                if (!get_number(argv[i], &b)) {
                        prog_print_err("invalid byte '%s'\n", argv[i]);
                        goto end;
                }
                buf[i - 1] = (uint8_t) b;
        }

        ret = prog_write_to_oqspi(addr, buf, argc - 1);
        if (ret) {
                const char *mem_name = "OQSPI";

                prog_print_err("write to %s failed: %s (%d)\n", mem_name, prog_get_err_message(ret),
                                                                                                ret);
                goto end;
        }
end:
        free(buf);

        return 1;
}

static int cmdh_read_oqspi(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[1];
        unsigned int size = 0;
        unsigned int size_limit;
        uint8_t *buf = NULL;
        int ret;
        const char *chip_rev;
        const prog_memory_sizes_t *sizes;
        const char *mem_name = "OQSPI";

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (!get_number(argv[2], &size) || !size) {
                prog_print_err("invalid size\n");
                return 0;
        }

        prog_get_chip_rev(&chip_rev);
        prog_get_memory_sizes(chip_rev, &sizes);
        size_limit = sizes->oqspi_size;

        if (size > size_limit) {
                prog_print_err("invalid size exceeding %s size\n", mem_name);
                return 0;
        }

        if (!strcmp(fname, "-") || !strcmp(fname, "--")) {

                buf = malloc(size);
                if (!buf) {
                        ret = ERR_ALLOC_FAILED;
                } else {
                        ret = prog_read_oqspi(addr, buf, size);
                }
        } else {
                ret = prog_read_oqspi_to_file(addr, fname, size);
        }
        if (ret) {
                free(buf);
                prog_print_err("read from %s failed: %s (%d)\n", mem_name, prog_get_err_message(ret),
                                                                                                ret);
                return 0;
        }

        if (buf) {
                dump_hex(addr, buf, size, !strcmp(fname, "--") ? 32 : 16);
                free(buf);
        }

        return 1;
}

static int cmdh_erase_oqspi(int argc, char *argv[])
{
        unsigned int addr;
        unsigned int size;
        int ret;

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (!get_number(argv[1], &size)) {
                prog_print_err("invalid size\n");
                return 0;
        }

        ret = prog_erase_oqspi(addr, size);
        if (ret) {
                const char *mem_name = "OQSPI";
                prog_print_err("erase %s failed: %s (%d)\n", mem_name, prog_get_err_message(ret),
                                                                                                ret);
                return 0;
        }

        return 1;
}

static int cmdh_copy_oqspi(int argc, char *argv[])
{
        unsigned int addr_ram;
        unsigned int addr_oqspi;
        unsigned int size;
        int ret;

        if (!get_number(argv[0], &addr_ram)) {
                prog_print_err("invalid RAM address\n");
                return 0;
        }

        if (!get_number(argv[1], &addr_oqspi)) {
                prog_print_err("invalid OQSPI address\n");
                return 0;
        }

        if (!get_number(argv[2], &size)) {
                prog_print_err("invalid size\n");
                return 0;
        }

        ret = prog_copy_to_oqspi(addr_ram, addr_oqspi, size);
        if (ret) {
                const char *mem_name = "OQSPI";
                prog_print_err("copy to %s failed: %s (%d)\n", mem_name, prog_get_err_message(ret),
                                                                                                ret);
                return 0;
        }

        return 1;
}

static int cmdh_is_empty_oqspi(int argc, char *argv[])
{
        unsigned int size = OQSPI_DEFAULT_SIZE;
        unsigned int start_address = 0;
        int ret_number;
        int ret = 0;
        const char *mem_name = "OQSPI";


        if (argc != 0 && argc != 2) {
                prog_print_err("invalid argument - function is_empty_qspi needs zero or two"
                                                                                "arguments\n");
                return 0;
        }

        if (argc == 2) {
                if (!get_number(argv[0], &start_address)) {
                        prog_print_err("invalid start address\n");
                        return 0;
                }

                if (!get_number(argv[1], &size) || size == 0) {
                        prog_print_err("invalid size\n");
                        return 0;
                }
        }

        ret = prog_is_empty_oqspi(size, start_address, &ret_number);

        if (!ret) {
                if (ret_number <= 0) {
                        prog_print_log("%s flash region is not empty (byte at 0x%08x + 0x%08x is not "
                                        "0xFF).\n", mem_name, start_address, (-1 * ret_number));
                } else {
                        prog_print_log("%s flash region is empty (checked %u bytes).\n", mem_name,
                                                                                        ret_number);
                }
                ret = 1;
        } else {
                prog_print_err("check %s emptiness failed: %s (%d)\n", mem_name,
                                                                prog_get_err_message(ret), ret);
                ret = 0;
        }

        return ret;
}

static int cmdh_chip_erase_oqspi(int argc, char *argv[])
{
        unsigned int addr = OQSPI_MEM1_VIRTUAL_BASE_ADDR;
        int ret;

        if (argc != 0 && argc != 1) {
                prog_print_err("invalid argument - chip_erase_oqspi takes zero or one argument\n");
                return 0;
        }

        if (argc == 1) {
                if (!get_number(argv[0], &addr)) {
                        prog_print_err("invalid start address\n");
                        return 0;
                }

                if (*argv[0] != OQSPI_MEM1_VIRTUAL_BASE_ADDR) {
                        prog_print_err("wrong start address - use 0x%lx instead\n", OQSPI_MEM1_VIRTUAL_BASE_ADDR);
                        return 0;
                }
        }

        ret = prog_chip_erase_oqspi_by_addr(addr);

        if (ret) {
                prog_print_err("chip erase OQSPI failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        return 1;
}

static int cmdh_read_flash_info(int argc, char *argv[])
{
        int ret;

        if (argc != 0) {
                prog_print_err("invalid argument - read_flash_info takes no argument\n");
                return 0;
        }

        const char *man, *model;
        unsigned int size;

        flash_info_t flash_info = {
                .qspic_id         = 0,
                .qspi_flash_info  = {
                        .driver_configured = false,
                        .man_id            = 0x0,
                        .type              = 0x0,
                        .density           = 0x0
                },
                .oqspi_flash_info = {
                        .driver_configured = false,
                        .man_id            = 0x0,
                        .type              = 0x0,
                        .density           = 0x0
                }
        };

        ret = prog_read_flash_info(&flash_info);

        if (ret < 0) {
                prog_print_err("failed to read flash mem info: %s (%d)\n", prog_get_err_message(ret), ret);

                return 0;
        }

        if (flash_info.qspi_flash_info.driver_configured == true) {
                prog_print_log("QSPI flash mem info:\n");

                switch (flash_info.qspi_flash_info.man_id) {
                case ADESTO_ID:
                        man = "Adesto";

                        if ((flash_info.qspi_flash_info.type == 0x42) &&
                                        (flash_info.qspi_flash_info.density == 0x18)) {
                                model = "AT25SL128";
                                size  = 16; // 128 megabits
                        } else {
                                model = "N/A";
                                size  = 0;
                        }

                        break;

                case WINBOND_ID:
                        man = "Winbond";

                        if ((flash_info.qspi_flash_info.type == 0x80) &&
                                        (flash_info.qspi_flash_info.density == 0x19)) {
                                model = "W25Q256JW";
                                size  = 32; // 256 megabits
                        } else {
                                model = "N/A";
                                size  = 0;
                        }

                        break;

                default:
                        man   = "N/A";
                        model = "N/A";
                        size  = 0;
                        break;
                }

                prog_print_log("Manufacturer = %s\n", man);
                prog_print_log("Device model = %s\n", model);
                prog_print_log("Device size = %u MB\n", size);
        } else {
                prog_print_log("QSPI flash NOT present\n");
        }

        prog_print_log("\n");

        if (flash_info.oqspi_flash_info.driver_configured == true) {
                prog_print_log("OQSPI flash mem info:\n");

                switch (flash_info.oqspi_flash_info.man_id) {
                case GIGADEVICE_ID:
                        man = "GigaDevice";

                        if ((flash_info.oqspi_flash_info.type == 0x60) &&
                                        (flash_info.oqspi_flash_info.density == 0x17)) {
                                model = "GD25LQ64C/GD25LE64E";
                                size  = 8; // 64 megabits
                        } else {
                                model = "N/A";
                                size  = 0;
                        }

                        break;

                case MACRONIX_ID:
                        man = "Macronix";

                        if ((flash_info.oqspi_flash_info.type == 0x25) &&
                                (flash_info.oqspi_flash_info.density == 0x37)) {
                                model = "MX25U6432";
                                size  = 8; // 64 megabits
                        } else if ((flash_info.oqspi_flash_info.type == 0x80) &&
                                        (flash_info.oqspi_flash_info.density == 0x3B)) {
                                model = "MX66UM1G45G";
                                size  = 128; // 1 gigabit
                        } else {
                                model = "N/A";
                                size  = 0;
                        }

                        break;

                case WINBOND_ID:
                        man = "Winbond";

                        if ((flash_info.oqspi_flash_info.type == 0x80) &&
                                        (flash_info.oqspi_flash_info.density == 0x17)) {
                                model = "W25Q64JWIM";
                                size = 8; // 64 megabits
                        } else {
                                model = "N/A";
                                size  = 0;
                        }

                        break;
                case ADESTO_ID:
                        man = "Adesto";

                        if ((flash_info.qspi_flash_info.type == 0x42) &&
                                        (flash_info.qspi_flash_info.density == 0x18)) {
                                model = "AT25SL128";
                                size  = 16; // 128 megabits
                        } else {
                                model = "N/A";
                                size  = 0;
                        }

                        break;

                default:
                        man   = "N/A";
                        model = "N/A";
                        size  = 0;
                        break;
                }

                prog_print_log("Manufacturer = %s\n", man);
                prog_print_log("Device model = %s\n", model);
                prog_print_log("Device size = %u MB\n", size);
        } else {
                prog_print_log("OQSPI flash NOT present\n");
        }

        return 1;
}

int handle_command(char *cmd, int argc, char *argv[])
{
        struct cli_command *cmdh = cmds;

        /* lookup command handler */
        while (cmdh->name && strcmp(cmdh->name, cmd)) {
                cmdh++;
        }

        /* handlers table is terminated by empty entry, so name == NULL means no handler found */
        if (!cmdh->name) {
                prog_print_err("invalid command\n");
                return 1;
        }

        if (argc < cmdh->min_num_p) {
                prog_print_err("not enough parameters\n");
                return 1;
        }

        /*
         * return value from handler (0=failure) will be used as exit code so need to change it
         * here to have exit code 0 on success
         */
        return !cmdh->func(argc, argv);
}
