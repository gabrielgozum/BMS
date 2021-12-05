/* AUTOGENERATED FILE. DO NOT EDIT. */
#ifndef _MOCKSPI_H
#define _MOCKSPI_H

#include "spi.h"

/* Ignore the following warnings, since we are copying code */
#if defined(__GNUC__) && !defined(__ICC) && !defined(__TMS470__)
#if __GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ > 6 || (__GNUC_MINOR__ == 6 && __GNUC_PATCHLEVEL__ > 0)))
#pragma GCC diagnostic push
#endif
#if !defined(__clang__)
#pragma GCC diagnostic ignored "-Wpragmas"
#endif
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wduplicate-decl-specifier"
#endif

void Mockspi_Init(void);
void Mockspi_Destroy(void);
void Mockspi_Verify(void);




#define SSI0_init_Ignore() SSI0_init_CMockIgnore()
void SSI0_init_CMockIgnore(void);
#define SSI0_init_Expect() SSI0_init_CMockExpect(__LINE__)
void SSI0_init_CMockExpect(UNITY_LINE_TYPE cmock_line);
#define SSI1_init_Ignore() SSI1_init_CMockIgnore()
void SSI1_init_CMockIgnore(void);
#define SSI1_init_Expect() SSI1_init_CMockExpect(__LINE__)
void SSI1_init_CMockExpect(UNITY_LINE_TYPE cmock_line);
#define spi_write_read_Ignore() spi_write_read_CMockIgnore()
void spi_write_read_CMockIgnore(void);
#define spi_write_read_Expect(tx_data, tx_len, rx_data, rx_len, base) spi_write_read_CMockExpect(__LINE__, tx_data, tx_len, rx_data, rx_len, base)
void spi_write_read_CMockExpect(UNITY_LINE_TYPE cmock_line, uint8_t* tx_data, uint8_t tx_len, uint8_t* rx_data, uint8_t rx_len, uint32_t base);
#define spi_write_array_Ignore() spi_write_array_CMockIgnore()
void spi_write_array_CMockIgnore(void);
#define spi_write_array_Expect(len, data, base) spi_write_array_CMockExpect(__LINE__, len, data, base)
void spi_write_array_CMockExpect(UNITY_LINE_TYPE cmock_line, uint8_t len, uint8_t* data, uint32_t base);
#define spiTransfer_IgnoreAndReturn(cmock_retval) spiTransfer_CMockIgnoreAndReturn(__LINE__, cmock_retval)
void spiTransfer_CMockIgnoreAndReturn(UNITY_LINE_TYPE cmock_line, uint32_t cmock_to_return);
#define spiTransfer_ExpectAndReturn(tx, base, cmock_retval) spiTransfer_CMockExpectAndReturn(__LINE__, tx, base, cmock_retval)
void spiTransfer_CMockExpectAndReturn(UNITY_LINE_TYPE cmock_line, uint8_t tx, uint32_t base, uint32_t cmock_to_return);

#if defined(__GNUC__) && !defined(__ICC) && !defined(__TMS470__)
#if __GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ > 6 || (__GNUC_MINOR__ == 6 && __GNUC_PATCHLEVEL__ > 0)))
#pragma GCC diagnostic pop
#endif
#endif

#endif
