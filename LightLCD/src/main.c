/**
 * \file
 *
 * \brief Display the light sensor reading as a graph
 *
 * This application uses the ADC to convert the signal from the 
 * light sensor and displays it as a graph on the LCD.  
 *
 * Interrupts are used with the ADC and adc_handler() 
 * is called when a value is ready. The value is then output to the LCD screen as a pixel 
 * that has been positioned according to it value.
 * (0,0) has been moved from its normal top left position to the bottom left
 * 
   Once the pixel reaches the right-hand-side of the LCD screen, the display
   starts again from the left-hand-side.
 
 * ASF Workflow
   1) First of all I looked for existing examples for the A3BU-XPLAINED board.
      When I couldn't find any examples I googled for what I was looking for.  The XPLAINED board
      didn't show any results and also searching for xmega didn't return too many.  As these are
      new devices I do expect this to change.
   2) When I couldn't find any existing examples I then checked the source code to the ASF for the
      board and then the device/service.
   3) If I found something that was applicable - or even if I didn't - I would then look at the ASF
      documentation for that device/service.
   4) The next step would be to look at the datasheet for the peripheral.  Although, as a beginner
      and not coming from an AVR background, this was often the first step for me.
      
  * What would make my ASF design process easier.
    I am very impressed with the way AVR allows quick access to documentation, whether it's on
    the web or locally.  I would like an option for when say right click on a ASF function or 
    structure, it would take me to the API documentation for that function/structure.
    
  * What key features would I like added to ASF
    - I was very interested in the event system of the xmega but I couldn't find any ASF API for it.
    - It would be nice if some features (even a small subset) could be ported to other AVR devices.
   
  * What are my general comments on the ASF
   - A very professional, well thought out and documented library.  It is nicely integrated into AVR Studio
     makes it a pleasure to use.
       
 */

#include <asf.h>

static const uint16_t SCALED_LCD_HEIGHT = (4096 / GFX_MONO_LCD_HEIGHT);

static struct adc_config adc_config;
static struct adc_channel_config adcch_config;
static volatile uint8_t scaled_light_reading;

static uint8_t x = 0;

static void adc_handler(ADC_t *adc, uint8_t ch_mask, adc_result_t result);
static void setup_lcd(void);
static void setup_adc(void);
static void update_graph_display(void);

/**
 * \internal
 * \brief Handler for ADC conversion result
 *
 * Callback function for for when an ADC conversion is ready
 * In this handler we create a scaled light reading according
 * to the height of the LCD screen
 */
static void adc_handler(ADC_t *adc, uint8_t ch_mask, adc_result_t result)
{
    scaled_light_reading = GFX_MONO_LCD_HEIGHT - (result / SCALED_LCD_HEIGHT);
    adc_start_conversion(adc, ch_mask);
}

/**
 * \internal
 * \brief Initialize the LCD display
 */
static void setup_lcd(void)
{
    gfx_mono_init();
    ioport_set_pin_high(NHD_C12832A1Z_BACKLIGHT);
    st7565r_set_contrast(ST7565R_DISPLAY_CONTRAST_MIN);
}

/**
 * \internal
 * \brief Initialize the ADC
 */
static void setup_adc(void)
{	
    adc_read_configuration(&ADCA, &adc_config);
    adcch_read_configuration(&ADCA, ADC_CH0, &adcch_config);
	
    adc_set_conversion_parameters(&adc_config, ADC_SIGN_OFF, ADC_RES_12, ADC_REF_BANDGAP);
	
    adc_set_clock_rate(&adc_config, 200000UL);
    adc_set_conversion_trigger(&adc_config, ADC_TRIG_MANUAL, 0, 0);
	
    adc_write_configuration(&ADCA, &adc_config);
    adc_set_callback(&ADCA, &adc_handler);
	
    adcch_set_input(&adcch_config, LIGHT_SENSOR_ADC_INPUT, ADCCH_NEG_NONE, 1);
    adcch_set_interrupt_mode(&adcch_config, ADCCH_MODE_COMPLETE);
    adcch_enable_interrupt(&adcch_config);
    adcch_write_configuration(&ADCA, ADC_CH0, &adcch_config);	
}

/**
 * \internal
 * \brief Updates the display with the scaled lcd reading
 */
static void update_graph_display(void)
{
    // Clear just ahead of the pixel position
    gfx_mono_draw_filled_rect(x, 0, 6, GFX_MONO_LCD_HEIGHT, GFX_PIXEL_CLR);
            
    // Draw the pixel 
    gfx_mono_draw_pixel(x, scaled_light_reading, GFX_PIXEL_SET);
    
    // Update the x-axis position, move to LHS when at RHS of display
    x = (x >=  GFX_MONO_LCD_WIDTH) ? 0 : ++x;
}

int main(void)
{
    // Initialize the board
    board_init();
    sysclk_init();
    pmic_init();
    cpu_irq_enable();
		
    // Setup the LCD and ADC
    setup_lcd();
    setup_adc();

    // Enable the ADC and start converting
    adc_enable(&ADCA);
    adc_start_conversion(&ADCA, ADC_CH0);
		
    // Main loop
    for(;;)
    {
        update_graph_display();
    }    	
}
