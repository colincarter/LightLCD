/**
 * \file
 *
 * \brief Display the light sensor reading as a graph
 *
 */

#include <asf.h>

enum Modes
{
    MODE_GRAPH,
    MODE_TEXT
};

static const uint16_t SCALED_LCD_HEIGHT = (4096 / GFX_MONO_LCD_HEIGHT);

static struct adc_config adc_config;
static struct adc_channel_config adcch_config;
static volatile uint8_t scaled_light_reading;

static volatile uint8_t mode =  MODE_GRAPH;

static uint8_t x = 0;

static void clear_lcd(void)
{
    gfx_mono_draw_filled_rect(0, 0, GFX_MONO_LCD_WIDTH, GFX_MONO_LCD_HEIGHT, GFX_PIXEL_CLR);
}

static void setup_graph_display(void)
{
    clear_lcd();
	x = 0;
}

static void setup_text_display(void)
{
    clear_lcd();
}

static void adc_handler(ADC_t *adc, uint8_t ch_mask, adc_result_t result)
{
    scaled_light_reading = GFX_MONO_LCD_HEIGHT - (result / SCALED_LCD_HEIGHT);
    adc_start_conversion(adc, ch_mask);
}

static void setup_lcd(void)
{
    gfx_mono_init();
    ioport_set_pin_high(NHD_C12832A1Z_BACKLIGHT);
    st7565r_set_contrast(ST7565R_DISPLAY_CONTRAST_MIN);
}

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

void update_graph_display(void)
{
    // Clear just ahead of the pixel position
    gfx_mono_draw_filled_rect(x, 0, 6, GFX_MONO_LCD_HEIGHT, GFX_PIXEL_CLR);
            
    // Draw the pixel 
    gfx_mono_draw_pixel(x, scaled_light_reading, GFX_PIXEL_SET);
    
    x = (x >=  GFX_MONO_LCD_WIDTH) ? 0 : ++x;
}

struct button
{
	uint8_t button;
	uint8_t direction;
};

void update_text_display(void)
{
    
}

void set_mode(struct button *b)
{
	if (gpio_pin_is_low(GPIO_PUSH_BUTTON_0))
	{
		
	    mode = MODE_GRAPH;	
		setup_graph_display();
	}
	
	if (gpio_pin_is_low(GPIO_PUSH_BUTTON_1))
	{
		mode = MODE_TEXT;
		setup_text_display();
	}
}

int main(void)
{
	struct button button;
		
    // Initialize the board
    board_init();
    sysclk_init();
    pmic_init();
    cpu_irq_enable();
		
    // Setup the LCD and ADC
    setup_lcd();
    setup_adc();

    adc_enable(&ADCA);
    adc_start_conversion(&ADCA, ADC_CH0);
		
    while (1) 
    {
		set_mode(&button);
		
        if (mode == MODE_GRAPH)
            update_graph_display();
        else if (mode == MODE_TEXT)
            update_text_display();
    }    	
}
