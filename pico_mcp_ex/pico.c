#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h" //Find where to get these or where they are 

#define SPI_PORT spi0

static const uint8_t REG_DAC_DATA = 0x8; //offset = 8h --> DAC Data Register 
int read_pressure(const uint cs_pin,spi_inst_t *spi); 

int read_pressure(const uint cs_pin, spi_inst_t *spi)
{
    uint8_t buffer[2]; 
    int bytes; 
    uint16_t result; 
    const float conversion_factor = 3.3f / (1 << 12);

    while(true)
    {
        gpio_put(cs_pin, 0); 
        //sleep_ms (2 * 1000);
        bytes = spi_read_blocking(spi, 0, buffer, 2);
        sleep_ms (2 * 1000);
        // printf("IS SPI Busy: %d \n", spi_is_busy(spi));
        printf("Buffer 1 = %d ", buffer[0]);
        printf("Buffer 1 = 0x%x \n", buffer[0]);
        printf("Buffer 2 = %d ", buffer[1]);
        printf("Buffer 2 = 0x%x \n", buffer[1]);
        gpio_put(cs_pin, 1); 

        //need to drop the first bit from buffer[0]
        result = (buffer[0] >> 1); //buffer is now 4 bits 
        result = (buffer[0] << 8) | (buffer[1]); 
        //concatenate both buffers to get the full 12 bit result from the mcp3301 
        printf("Result = 0x%x \n",result); 
        printf("Result = %d \n",result);
        printf("Voltage = %f \n",(result * conversion_factor));
        printf("\n"); 
    }
    return 0; 
}

int main ()
{
    int val; 
    
    const uint sck_pin = 2;  //gp 2
    const uint mosi_pin = 3; // gp 3
    const uint miso_pin = 4; //gp 4
    const uint cs_pin = 5; //gp 5
    spi_inst_t *spi = spi0;
    

    stdio_init_all(); //Initialize all pins 
    sleep_ms(8500);
    printf("SPI protocal begin \n"); 


    // Setup SPI on Pico
    val = spi_init(spi, 90000); 
    sleep_ms(3000);


    spi_set_format(spi0,   // SPI instance
                8,      // Number of bits per transfer
                1,      // Polarity (CPOL)
                1,      // Phase (CPHA)
                SPI_MSB_FIRST);



    // Initalize Pins
    gpio_set_function(miso_pin,GPIO_FUNC_SPI);
    gpio_set_function(mosi_pin,GPIO_FUNC_SPI);
    gpio_set_function(sck_pin,GPIO_FUNC_SPI); 

    gpio_init(cs_pin); 
    gpio_set_dir(cs_pin, GPIO_OUT);
    gpio_put(cs_pin, 1); 

    // blink_led(); //just to confirm the pico is working
    read_pressure(cs_pin,spi); 
    return 0; 
}

//mcp outputs extra zeroes for some reason (interesting?)