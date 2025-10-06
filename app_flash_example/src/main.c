

#include <xs1.h>
#include "platform.h"
#include <string.h>
#include <stdio.h> // for snprintf used to format messages into a buffer

#include <quadflash.h>

// ---- Select or adapt the fwk_io UART include for your SDK revision ---- // Newer SDKs typically provide a UART transport under fwk_io. // If your SDK uses a different header name, adjust this include and init below.

#include "uart.h" // Adjust if your SDK uses a different path/name
#define TEST_OFFSET_BYTES (0x00100000) // 1 MiB safe offset (adjust if needed)
#define TEST_SECTOR_SIZE (4096) // 4 KiB sector
#define TEST_LEN (256)
#define UART_TX_PORT (XS1_PORT_1N)

uart_tx_t *uart;

// uart_tx_t should be initiated in main
static void uart_puts(const char* s)
{

    for (const char *p = s; *p != '\0'; ++p) {

        if (*p == '\n') {

            uart_tx(uart, (uint8_t)'\r');

        }

            uart_tx(uart, (uint8_t)*p);

    }

}

static void uart_printf(const char *fmt, ...) { char buf[160]; va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof buf, fmt, ap); va_end(ap); uart_puts(buf); }

static void flash_test(void)
{ // Init QSPI flash using board-provided resources qspi_flash_init( &flash_ctx, FLASH_IO_QSPI, FLASH_CLKBLK, FLASH_CS_PORT);

    
    fl_QSPIPorts qspi = {
        PORT_SQI_CS,
        PORT_SQI_SCLK,
        PORT_SQI_SIO,
        XS1_CLKBLK_2
    };

    fl_connect(&qspi);
    if (fl_getFlashType() == 0) {
    } else {
        uart_puts("Warning: could not read JEDEC ID\r\n");
    }

 

    // Prepare test data
    unsigned char * scratch; // suspect this needs to be able to hold 1 page
    scratch = malloc(fl_getWriteScratchSize(TEST_OFFSET_BYTES,TEST_LEN));
    uint8_t tx[TEST_LEN];
    uint8_t rx[TEST_LEN];

    for (unsigned i = 0; i < TEST_LEN; ++i) {
        tx[i] = (uint8_t)i;
        rx[i] = 0x00;
    }


    // Erase
    uart_printf("Erasing sector at 0x%08X\r\n", TEST_OFFSET_BYTES);

    if (fl_eraseDataSector(TEST_OFFSET_BYTES) != 0) {
        uart_puts("Erase failed\r\n");
        return;
    }


    // Write
    uart_printf("Writing %u bytes at 0x%08X\r\n", (unsigned)TEST_LEN, TEST_OFFSET_BYTES);

    if (fl_writeData(TEST_OFFSET_BYTES, TEST_LEN, tx, scratch) != 0) {
        uart_puts("Write failed\r\n");
        return;
    }


    // Read back
    if (fl_readData(TEST_OFFSET_BYTES, TEST_LEN, rx) != 0) {
        uart_puts("Read failed\r\n");
        return;
    }


    // Verify
    if (memcmp(tx, rx, TEST_LEN) == 0) {
        uart_printf("Flash test PASSED at 0x%08X (len=%u)\r\n", TEST_OFFSET_BYTES, (unsigned)TEST_LEN);
    } else {
        uart_puts("Flash test FAILED: data mismatch\r\n");
        for (unsigned i = 0; i < TEST_LEN; ++i) {
            if (tx[i] != rx[i]) {
                uart_printf("Mismatch at %u: wrote %02X, read %02X\r\n", i, tx[i], rx[i]);
                break;
            }
        }
    }
}

int main()
{
    uart_tx_blocking_init(
        uart,
        UART_TX_PORT,
        115200,
        8,
        0,
        1,
        0);

    flash_test();
    return 0;
}