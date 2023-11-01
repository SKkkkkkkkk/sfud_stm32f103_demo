#include <sfud.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include "main.h"
static char log_buf[256];

typedef struct
{
    SPI_HandleTypeDef *hspi;
    uint32_t lock;
    void (*cs_low)(void);
    void (*cs_high)(void);
} sfud_port_t;

void sfud_log_debug(const char *file, const long line, const char *format, ...);

static void spi_lock(const struct __sfud_spi *spi)
{
    // __disable_irq();
}

static void spi_unlock(const struct __sfud_spi *spi)
{
    // __enable_irq();
}

/**
 * SPI write data then read data
 */
static sfud_err spi_write_read(const sfud_spi *spi, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf,
                               size_t read_size)
{
    sfud_err result = SFUD_SUCCESS;
    sfud_port_t *sfud_port = (sfud_port_t *)(spi->user_data);
    // uint8_t send_data, read_data;

    /**
     * add your spi write and read code
     */
    sfud_port->cs_low();
    if (HAL_SPI_Transmit(sfud_port->hspi, (uint8_t *)write_buf, write_size, 3000) != HAL_OK)
    {
        // sfud_log_debug(__FILE__, __LINE__, "spi write fail.");
        result = SFUD_ERR_WRITE;
        goto ERR;
    }
    if ((read_buf != NULL) && (read_size != 0))
    {
        if (HAL_SPI_Receive(sfud_port->hspi, read_buf, read_size, 3000) != HAL_OK)
        {
            // sfud_log_debug(__FILE__, __LINE__, "spi read fail.");
            result = SFUD_ERR_READ;
        }
    }

    // return result;
ERR:
    sfud_port->cs_high();
    return result;
}

#ifdef SFUD_USING_QSPI
/**
 * read flash data by QSPI
 */
static sfud_err qspi_read(const struct __sfud_spi *spi, uint32_t addr, sfud_qspi_read_cmd_format *qspi_read_cmd_format,
                          uint8_t *read_buf, size_t read_size)
{
    sfud_err result = SFUD_SUCCESS;

    /**
     * add your qspi read flash data code
     */

    return result;
}
#endif /* SFUD_USING_QSPI */

static void retry_delay(void)
{
    HAL_Delay(1);
}

sfud_err sfud_spi_port_init(sfud_flash *flash)
{
    sfud_err result = SFUD_SUCCESS;
    sfud_port_t *sfud_port = NULL;
    /**
     * add your port spi bus and device object initialize code like this:
     * 1. rcc initialize
     * 2. gpio initialize
     * 3. spi device initialize
     * 4. flash->spi and flash->retry item initialize
     *    flash->spi.wr = spi_write_read; //Required
     *    flash->spi.qspi_read = qspi_read; //Required when QSPI mode enable
     *    flash->spi.lock = spi_lock;
     *    flash->spi.unlock = spi_unlock;
     *    flash->spi.user_data = &spix;
     *    flash->retry.delay = null;
     *    flash->retry.times = 10000; //Required
     */
    /*Configure GPIO pin Output Level */
    if (strcmp(flash->spi.name, "SPI1") == 0)
    {
        // MX_SPI1_Init();

        sfud_port = malloc(sizeof(sfud_port_t));
        if (sfud_port == NULL)
        {
            sfud_log_debug(__FILE__, __LINE__, "memory alloc fail.");
            goto ERR_MEMALLOC;
        }
        extern SPI_HandleTypeDef hspi1;
        extern void spi1_cs_low(void);
        extern void spi1_cs_high(void);
        sfud_port->hspi = &hspi1;
        sfud_port->cs_high = spi1_cs_high;
        sfud_port->cs_low = spi1_cs_low;

        flash->spi.wr = spi_write_read; // Required
        flash->spi.lock = spi_lock;
        flash->spi.unlock = spi_unlock;
        flash->spi.user_data = sfud_port;
        flash->retry.delay = retry_delay;
        flash->retry.times = 10000; // Required
    }
    else
    {
        result = SFUD_ERR_NOT_FOUND;
    }

    return result;

    free(sfud_port);
ERR_MEMALLOC:
    HAL_SPI_DeInit(sfud_port->hspi);

    return result;
}

/**
 * This function is print debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 */
void sfud_log_debug(const char *file, const long line, const char *format, ...)
{
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    printf("[SFUD](%s:%ld) ", file, line);
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    printf("%s\n", log_buf);
    va_end(args);
}

/**
 * This function is print routine info.
 *
 * @param format output format
 * @param ... args
 */
void sfud_log_info(const char *format, ...)
{
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    printf("[SFUD]");
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    printf("%s\n", log_buf);
    va_end(args);
}