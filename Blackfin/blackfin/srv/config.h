#ifndef CONFIG_H_
#define CONFIG_H_

#include <cdefBF537.h>

#define CPU_BF537    37

#define BOARD_NAME          "SRV-1 Blackfin Camera Board"
#define CPU                 CPU_BF537
#define MASTER_CLOCK        22118000
#define SCLK_DIVIDER        4
#define VCO_MULTIPLIER      22
#define CCLK_DIVIDER        1

#define SDRAM_RECONFIG      1
//#define SDRAM_EBIU_SDRRC    0x03a0

//#define SENSOR_PORT         1  
//#define CONFIG_VIDEO        1

// PLL and clock definitions

// make sure, VCO stays below the nominal operation frequency of the core
// (normally 600 MHz)
//
// VCO = VCO_MULTIPLIER * MASTER_CLOCK / MCLK_DIVIDER
// where MCLK_DIVIDER = 1 when DF bit = 0,  (default)
//                      2               1
// CCLK = VCO / CCLK_DIVIDER
// SCLK = VCO / SCLK_DIVIDER

#define CORE_CLOCK (MASTER_CLOCK * VCO_MULTIPLIER / CCLK_DIVIDER)   // 486 596 000 Hz
#define PERIPHERAL_CLOCK  (CORE_CLOCK / SCLK_DIVIDER)               // 121 649 000 Hz

/* UART config */

// choose the mode dependent on UART connection. Note that they exclude each other
#define SERIAL_MODE
//#define WIFI_MODE

#ifdef SERIAL_MODE
	#define UART0_BAUDRATE 230400 //115200
	#define UART1_BAUDRATE 38400
#endif

#ifdef WIFI_MODE
	#define UART0_BAUDRATE 2304000
	#define HW_FLOW_CONTROL
	#define UART1_BAUDRATE 38400
#endif

//#define DEBUG

// must be power of 2!
#define FIFO_LENGTH  64
#define FIFO_MODULO_MASK (FIFO_LENGTH - 1)

#define UART0_DIVIDER   (MASTER_CLOCK * VCO_MULTIPLIER / SCLK_DIVIDER \
                        / 16 / UART0_BAUDRATE)
#define UART0_FAST_DIVIDER   (MASTER_CLOCK * VCO_MULTIPLIER / SCLK_DIVIDER \
                        / 16 / UART0_FAST_BAUDRATE)
#define UART_DIVIDER   UART0_DIVIDER
#define UART1_DIVIDER   (MASTER_CLOCK * VCO_MULTIPLIER / SCLK_DIVIDER \
                        / 16 / UART1_BAUDRATE)
// Blackfin environment memory map

#define L1_DATA_SRAM_A 0xff800000
#define FIFOLENGTH 0x100

/* general macros */
#define SSYNC    asm("ssync;")

/* camera defines */
#define DEFAULT_IMAGE_HEIGHT    240
#define DEFAULT_IMAGE_WIDTH		320

/* flash usage */
#define BOOT_SECTOR  0x00000000  // address in SPI flash of boot image
#define FLASH_SECTOR 0x00040000  // address in SPI flash of user flash sector

/* SDRAM allocation */
#define FLASH_BUFFER 0x00100000  // address in SDRAM for buffering flash and xmodem
#define FLASH_BUFFER_SIZE 0x00100000
#define C_HEAPSTART  0x00200000  // 512kB buffer for picoC
#define C_HEAPSIZE   0x00080000
#define SPI_BUFFER1  0x00280000  // 256kB buffer for transfer of data via SPI bus
#define SPI_BUFFER2  0x002C0000  // 256kB buffer for transfer of data via SPI bus
#define HTTP_BUFFER  0x00280000  // 256kB buffer also used for receiving and sending HTTP messages
#define HTTP_BUFFER_SIZE 0x00040000
#define HTTP_BUFFER2 0x002C0000  // additional 256kB buffer for HTTP content
#define HTTP_BUFFER2_SIZE 0x00040000
#define HEAPSTART    0x00300000  // put this above FLASH_BUFFER
#define HEAPSIZE     0x00C00000  // 12MB for now - leave 1MB for JPEG buffer
#define DMA_BUF1     0x01000000  // address in SDRAM for DMA transfer of frames from camera
#define DMA_BUF2     0x01280000  // second DMA buffer for double buffering
#define FRAME_BUF    0x01500000  // address in SDRAM for staging images for processing/jpeg
#define FRAME_BUF2   0x01780000  // second frame buffer for storing reference frame
#define FRAME_BUF3   0x01A00000  // third frame buffer for edge data or YUV planar data
#define FRAME_BUF4   0x01C80000  // fourth frame buffer
#define JPEG_BUF     0x00F00000  // address in SDRAM for JPEG compressed image
#define DISP_BUF     0x00F00000  // buffer used to send disparity data

/* video buf size 153600 bytes, 25800 in hex, so only up to 320 x 240 frames allowed */
#define VIDEO_BUF1   0x01F00000
#define VIDEO_BUF2	 0x01F25800
#define VIDEO_BUF3   0x01F4B000
#define VIDEO_BUF4	 0x01F70800
#define VIDEO_BUF5	 0x01F96000
#define VIDEO_BUF6   0x01FBB800
#define VIDEO_BUF7   0x01FE1000
#define VIDEO_BUF8	 0x02006800
#define VIDEO_BUF9	 0x0202C000
#define VIDEO_BUF10  0x02051800
#define VIDEO_BUF11  0x02077000
#define VIDEO_BUF12  0x0209C800
#define VIDEO_BUF13  0x020C2000
#define VIDEO_BUF14  0x020E7800
#define VIDEO_BUF15  0x0210D000
#define VIDEO_BUF16  0x02132800  // end at 0x02158000

#define VIDEO_BUF_COUNT 		8
#define VIDEO_BUF_ELEMENT_SIZE	153600  // bytes

/* Stack info */
#define STACK_TOP    0xFF904000
#define STACK_BOTTOM 0xFF900000

#endif
