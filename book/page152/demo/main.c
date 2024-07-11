
#include "functions.h"

#define WRITE_EN_CMD    0x06
#define WRITE_CMD       0x02
#define READ_CMD        0x03
#define READ_STATUS     0x05

#define LOW             0
#define HIGH            1
#define SET_SS_PIN(state) ( (state)?(PORTB |= (1 << PB2) ):(PORTB &= ~(1 << PB2) ) )

#define SIZE            10
uint8_t rom_contents[SIZE] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
uint8_t rx_buffer[SIZE];

// PB2: SS   --- OUT
// PB3: MOSI --- OUT
// PB4: MISO --- IN
// PB5: SCK  --- OUT
void init_spi_master(void)
{
    set_bits(DDRB, _BV(PB2) | _BV(PB3) | _BV(PB5));
    set_bits(PORTB, _BV(PB2) | _BV(PB3) | _BV(PB5));
    clear_bits(DDRB, _BV(PB4));
    clear_bits(PORTB, _BV(PB4));
    set_bits(SPCR, _BV(SPE) | _BV(MSTR) | _BV(SPR0));
    set_bits(SPSR, _BV(SPI2X)); 
}

void spi_tx_byte(uint8_t byte)
{
    SPDR = byte;
    loop_until_bit_is_set(SPSR, SPIF);
}

uint8_t spi_rx_byte(void)
{
    uint8_t rx_byte;
    spi_tx_byte(0);
    loop_until_bit_is_set(SPSR, SPIF);
    rx_byte = SPDR;
    return rx_byte;
}

void write_to_mem(uint8_t addr, uint8_t data)
{
    SET_SS_PIN(LOW);
    spi_tx_byte(WRITE_EN_CMD);
    SET_SS_PIN(HIGH);
    SET_SS_PIN(LOW);
    spi_tx_byte(WRITE_CMD);
    spi_tx_byte(addr);
    spi_tx_byte(data);
    SET_SS_PIN(HIGH);
}

uint8_t read_from_mem(uint8_t addr)
{
    uint8_t rx_data;
    SET_SS_PIN(LOW);
    spi_tx_byte(READ_CMD);
    spi_tx_byte(addr);
    rx_data = spi_rx_byte();
    SET_SS_PIN(HIGH);
    return rx_data;
}

uint8_t read_status(void)
{
    uint8_t status;
    SET_SS_PIN(LOW);
    spi_tx_byte(READ_STATUS);
    status = spi_rx_byte();
    SET_SS_PIN(HIGH);
    return status;
}

void eeprom_write(uint8_t start_addr, uint8_t size, uint8_t *data_ptr)
{
	int i = 0;
	for (i = 0; i < size; i++)
	{
		write_to_mem(start_addr + i, (*(data_ptr + i)));
		while (read_status() != 0);
	}
}

void eeprom_read(uint8_t start_addr, uint8_t size, uint8_t *data_ptr)
{
	int i = 0;
	for (i = 0; i < size; i++)
	{
		*(data_ptr + i) = read_from_mem(start_addr + i);
	}
}

int main(void)
{
    avr_uart_init();
	init_spi_master();
	eeprom_write(0x00, SIZE, rom_contents);
	eeprom_read(0x00, SIZE, rx_buffer);
	for (int j = 0; j < SIZE; j++)
	{
		xprintf("%u\t", rx_buffer[j]);
	}
    xprintf("\n");
	return 0;
}

