#include <stdio.h>
#include <iostream>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/adc.h" 

using namespace std; 

//function that take in user input 
//same logic except now we use 16 bits 

uint16_t desired_pressure() //either that or uint8_t 
{
    int pressure; // Unit: ?? 
    cout << "Enter Pressure Value (Unit: ??): " << endl; 
    cin >> pressure; 


    // the pressure needs to be converted into a voltage. That voltage is sent as an input 
    // scale from 0 to 4096 where 0 is the minimum and 4096 is the max
    // scale for voltage is from 0 to 12  (these scales are related by a power of 2)


    //scale for pressure is from Y to X, X being min amount set and Y being the max amount 

    uint16_t pressure_8bit = pressure; 

    //convert the pressure value into a format redable by the dac 

    return pressure; 
}

int send_pressure(uint16_t msg, const uint cs_pin, spi_inst_t *spi, uint16_t number) // needs another input for the number from 
{
    int bytes; 
    // uint8_t msg[2]; 
    uint16_t num = 0x0FF; 
    uint16_t settings = 0x7; // 0101
    settings = settings << 12;
    uint16_t message_16bit = settings | msg; 




    

    // msg[0] = 0x3F;     //[0011 1111]
    // msg[1] = 0xFF;     //[11111111] 
    const uint16_t* src = &message_16bit; 
     


    //better idea: take the 15-12 bits. Shift to the left by 4. or them with the first 4 bits of the 12 bit guy. Shift the others to the right maybe; 
    gpio_put(cs_pin, 0); 

    printf("Sending data to DAC \n"); 
    printf("0x%X\n",*src);
    printf("NUM: %d\n",msg); 
    bytes = spi_write16_blocking(spi, src, 1); 

    gpio_put(cs_pin, 1); 

    return 0; 
    /*
    bit 15: ~w Read or write to the DAC  
            0 = Write to DAC register
            1 = Ignore this command
    bit 14 BUF: VREF Input Buffer Control bit
            1 = Buffered
            0 = Unbuffered
    bit 13 GA: Output Gain Selection bit
            1 = 1x (VOUT = VREF * D/4096)
            0 = 2x (VOUT = 2 * VREF * D/4096)
    bit 12 SHDN: Output Shutdown Control bit
            1 = Active mode operation. VOUT is available.
            0 = Shutdown the device. Analog output is not available. VOUT pin is connected to 500 ktypical)
    bit 11-0 D11:D0: DAC Input Data bits. Bit x is ignored 
*/

//input number is unint_16t b/c it is a 12 bit number 
//the goal is to modify number in the formqt that the MCP4921 accepts 

}

int read_pressure(const uint cs_pin, spi_inst_t *spi)
{
    uint8_t buffer[2]; 
    //uint16_t buffers[2];  
    uint16_t value; 
    uint16_t *dst = &value; 
    int half_word; 
    uint16_t result; 
    const float conversion_factor = 3.3f / (1 << 12);

    {
        gpio_put(cs_pin, 0); 
        //sleep_ms (2 * 1000);
        half_word = spi_read16_blocking(spi, 0, dst, 1);
        printf("Half Word Returned: %d", half_word); 
        // sleep_ms (2 * 1000);
        // printf("IS SPI Busy: %d \n", spi_is_busy(spi));
        // printf("Buffer 1 = %d ", buffer[0]);
        // printf("Buffer 1 = 0x%x \n", buffer[0]);
        // printf("Buffer 2 = %d ", buffer[1]);
        // printf("Buffer 2 = 0x%x \n", buffer[1]);
        printf("Buffer 2 = %d ", value);
        printf("Buffer 2 = 0x%x \n", value);
        gpio_put(cs_pin, 1); 
        printf("Voltage = %f \n",(value * conversion_factor));

        //need to drop the first bit from buffer[0]
        // result = (buffer[0] >> 1); //buffer is now 4 bits 
        // result = (buffer[0] << 8) | (buffer[1]); 
        // //concatenate both buffers to get the full 12 bit result from the mcp3301 
        // printf("Result = 0x%x \n",result); 
        // printf("Result = %d \n",result);
        printf("Voltage = %f \n",(value * conversion_factor));
        // printf("\n"); 
    }
    return 0; 
}

int main ()
{
    int val; 
    //uint16_t value = desired_pressure();
    uint16_t value = 0x88;
    
    const uint sck_pin = 2;  //gp 2
    const uint mosi_pin = 3; // gp 3
    const uint miso_pin = 4; //gp 4
    const uint cs_pin_dac = 1; //gp 5
    uint cs_pin = 5; //gp 6 
    spi_inst_t *spi = spi0;

    adc_init();
    adc_gpio_init(28);
    adc_select_input(2);
    uint16_t pot_val;
    

    

    stdio_init_all(); //Initialize all pins 
    sleep_ms(8500);
    printf("SPI protocal begin \n"); 


    // Setup SPI on Pico
    val = spi_init(spi, 90000); 
    sleep_ms(3000);


    spi_set_format(spi0,   // SPI instance
                16,      // Number of bits per transfer
                SPI_CPOL_1,      // Polarity (CPOL)
                SPI_CPHA_1,      // Phase (CPHA)
                SPI_MSB_FIRST);



    // Initalize Pins
    gpio_set_function(miso_pin,GPIO_FUNC_SPI);
    gpio_set_function(mosi_pin,GPIO_FUNC_SPI);
    gpio_set_function(sck_pin,GPIO_FUNC_SPI); 

    gpio_init(cs_pin); 
    gpio_set_dir(cs_pin, GPIO_OUT);
    gpio_put(cs_pin, 1); 

    gpio_init(cs_pin_dac); 
    gpio_set_dir(cs_pin_dac, GPIO_OUT); 
    gpio_put(cs_pin,1); 

    // blink_led(); //just to confirm the pico is working
    while(1)
    {
        pot_val = adc_read();
        send_pressure(pot_val,cs_pin_dac,spi,value);
    }
    read_pressure(cs_pin,spi); 
    return 0; 
}

//
