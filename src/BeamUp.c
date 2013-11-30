/**
	* Beam Up Pebble Watchface
	* Author: Chris Lewis
	* Date: 8th November 2013
	*/

#include <pebble.h>

#define DEBUG false

#define INV_LAYER_WIDTH 30
#define INV_LAYER_HEIGHT 101
#define HTX -12 
#define HUX 22
#define MTX 64
#define MUX 98
#define OFFSET 14

//Prototypes
void getTimeDigits(struct tm *t);
void setTimeDigits();
void predictNextDigits(struct tm *t);
void animateLayer(PropertyAnimation *animation, Layer *input, GRect startLocation, GRect finishLocation, int duration, int delay);
void setupTextLayer(TextLayer *layer, GRect location, GColor b_colour, GColor t_colour, ResHandle f_handle, GTextAlignment alignment);

//Globals
static Window *window;
static TextLayer *HTLayer, *HULayer, *colonLayer, *MTLayer, *MULayer;
static InverterLayer *HTInvLayer, *HUInvLayer, *MTInvLayer, *MUInvLayer, *bottomInvLayer;
static PropertyAnimation *HTUpAnim, *HTDownAnim, *HUUpAnim, *HUDownAnim, 
									*MTUpAnim, *MTDownAnim, *MUUpAnim, *MUDownAnim, 
									*HTInvUpAnim, *HTInvDownAnim, *HUInvUpAnim, *HUInvDownAnim, 
									*MTInvUpAnim, *MTInvDownAnim, *MUInvUpAnim, *MUInvDownAnim,
									*bottomInvRightAnim, *bottomInvLeftAnim;
static int HTDigit = 0, HTprev = 0, 
					 HUDigit = 0, HUprev = 0, 
					 MTDigit = 0, MTprev = 0,
					 MUDigit = 0, MUprev = 0;
static char hourText[] = "00", minuteText[] = "00",
						HTText[] = "0", HUText[] = "0",
						colonText[] = ":",
						MTText[] = "0", MUText[] = "0";

/**
    * Handle tick function
    */
static void handle_tick(struct tm *t, TimeUnits units_changed) 
{    
    //Get the time
    int seconds = t->tm_sec;
     
    //Bottom suface
    if(seconds == 15)
    {
        animateLayer(bottomInvRightAnim, inverter_layer_get_layer(bottomInvLayer), GRect(0, 105, 0, 5), GRect(0, 105, 36, 5), 500, 0);
    }
    else if(seconds == 30)  
    {
        animateLayer(bottomInvRightAnim, inverter_layer_get_layer(bottomInvLayer), GRect(0, 105, 36, 5), GRect(0, 105, 72, 5), 500, 0);
    }
    else if(seconds == 45)  
    {
        animateLayer(bottomInvRightAnim, inverter_layer_get_layer(bottomInvLayer), GRect(0, 105, 72, 5), GRect(0, 105, 108, 5), 500, 0);
    }
    else if(seconds == 58)  
    {
        animateLayer(bottomInvRightAnim, inverter_layer_get_layer(bottomInvLayer), GRect(0, 105, 108, 5), GRect(0, 105, 144, 5), 500, 1000);
    }
     
    //Animations and time change
    else if(seconds == 59) 
    {
        //Predict next changes
        predictNextDigits(t); //CALLS GETTIMEDIGITS()
         
        //Only change minutes units if its changed
        if((MUDigit != MUprev) || (DEBUG)) 
        {
            animateLayer(MUInvDownAnim, inverter_layer_get_layer(MUInvLayer), GRect(MUX+OFFSET, 0, INV_LAYER_WIDTH, 0), GRect(MUX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), 600, 0);
            animateLayer(MUUpAnim, text_layer_get_layer(MULayer), GRect(MUX, 53, 50, 60), GRect(MUX, -50, 50, 60), 200, 700);
        }
         
        //Only change minutes tens if its changed
        if((MTDigit != MTprev) || (DEBUG)) 
        {
            animateLayer(MTInvDownAnim, inverter_layer_get_layer(MTInvLayer), GRect(MTX+OFFSET, 0, INV_LAYER_WIDTH, 0), GRect(MTX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), 600, 0);
            animateLayer(MTUpAnim, text_layer_get_layer(MTLayer), GRect(MTX, 53, 50, 60), GRect(MTX, -50, 50, 60), 200, 700);
        }
         
        //Only change hours units if its changed
        if((HUDigit != HUprev) || (DEBUG)) 
        {
            animateLayer(HUInvDownAnim, inverter_layer_get_layer(HUInvLayer), GRect(HUX+OFFSET, 0, INV_LAYER_WIDTH, 0), GRect(HUX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), 600, 0);
            animateLayer(HUUpAnim, text_layer_get_layer(HULayer), GRect(HUX, 53, 50, 60), GRect(HUX, -50, 50, 60), 200, 700);
        }
         
        //Only change hours tens if its changed
        if((HTDigit != HTprev) || (DEBUG)) 
        {
            animateLayer(HTInvDownAnim, inverter_layer_get_layer(HTInvLayer), GRect(HTX+OFFSET, 0, INV_LAYER_WIDTH, 0), GRect(HTX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), 600, 0);
            animateLayer(HTUpAnim, text_layer_get_layer(HTLayer), GRect(HTX, 53, 50, 60), GRect(HTX, -50, 50, 60), 200, 700);
        }      
    } 
    else if(seconds == 0) 
    {       
        getTimeDigits(t);
         
        //Set the time off screen
        setTimeDigits(); 
 
        //Animate stuff back into place
        if((MUDigit != MUprev) || (DEBUG)) 
        {       
            animateLayer(MUDownAnim, text_layer_get_layer(MULayer), GRect(MUX, -50, 50, 60), GRect(MUX, 53, 50, 60), 200, 100);
            animateLayer(MUInvUpAnim, inverter_layer_get_layer(MUInvLayer), GRect(MUX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), GRect(MUX+OFFSET, 0, INV_LAYER_WIDTH, 0), 500, 500);
            MUprev = MUDigit;   //reset the thing
        }
        if((MTDigit != MTprev) || (DEBUG)) 
        {
            animateLayer(MTDownAnim, text_layer_get_layer(MTLayer), GRect(MTX, -50, 50, 60), GRect(MTX, 53, 50, 60), 200, 100);
            animateLayer(MTInvUpAnim, inverter_layer_get_layer(MTInvLayer), GRect(MTX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), GRect(MTX+OFFSET, 0, INV_LAYER_WIDTH, 0), 500, 500);
            MTprev = MTDigit;   
        }
        if((HUDigit != HUprev) || (DEBUG)) 
        {       
            animateLayer(HUDownAnim, text_layer_get_layer(HULayer), GRect(HUX, -50, 50, 60), GRect(HUX, 53, 50, 60), 200, 100);
            animateLayer(HUInvUpAnim, inverter_layer_get_layer(HUInvLayer), GRect(HUX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), GRect(HUX+OFFSET, 0, INV_LAYER_WIDTH, 0), 500, 500);
            HUprev = HUDigit;   
        }
        if((HTDigit != HTprev) || (DEBUG)) 
        {
            animateLayer(HTDownAnim, text_layer_get_layer(HTLayer), GRect(HTX, -50, 50, 60), GRect(HTX, 53, 50, 60), 200, 100);
            animateLayer(HTInvUpAnim, inverter_layer_get_layer(HTInvLayer), GRect(HTX+OFFSET, 0, INV_LAYER_WIDTH, INV_LAYER_HEIGHT), GRect(HTX+OFFSET, 0, INV_LAYER_WIDTH, 0), 500, 500);
            HTprev = HTDigit;   
        }
         
        //Bottom surface down
        animateLayer(bottomInvLeftAnim, inverter_layer_get_layer(bottomInvLayer), GRect(0, 105, 144, 5), GRect(0, 105, 0, 5), 500, 500);
    } 
}

/*
 * Load window members
 */
static void window_load(Window *window) {
	window_set_background_color(window, GColorBlack);
	
	//Get Font
	ResHandle f_handle = resource_get_handle(RESOURCE_ID_FONT_IMAGINE_48);
	
	//Allocate text layers
	HTLayer = text_layer_create(GRect(HTX, 53, 50, 60));
	text_layer_set_background_color(HTLayer, GColorClear);
	text_layer_set_text_color(HTLayer, GColorWhite);
	text_layer_set_font(HTLayer, fonts_load_custom_font(f_handle));
	text_layer_set_text_alignment(HTLayer, GTextAlignmentRight);
	layer_add_child(window_get_root_layer(window), (Layer*) HTLayer);
	
	HULayer = text_layer_create(GRect(HUX, 53, 50, 60));
	text_layer_set_background_color(HULayer, GColorClear);
	text_layer_set_text_color(HULayer, GColorWhite);
	text_layer_set_font(HULayer, fonts_load_custom_font(f_handle));
	text_layer_set_text_alignment(HULayer, GTextAlignmentRight);
	layer_add_child(window_get_root_layer(window), (Layer*) HULayer);
	
	colonLayer = text_layer_create(GRect(69, 53, 50, 60));
	text_layer_set_background_color(colonLayer, GColorClear);
	text_layer_set_text_color(colonLayer, GColorWhite);
	text_layer_set_font(colonLayer, fonts_load_custom_font(f_handle));
	text_layer_set_text_alignment(colonLayer, GTextAlignmentLeft);
	layer_add_child(window_get_root_layer(window), (Layer*) colonLayer);
	
	MTLayer = text_layer_create(GRect(MTX, 53, 50, 60));
	text_layer_set_background_color(MTLayer, GColorClear);
	text_layer_set_text_color(MTLayer, GColorWhite);
	text_layer_set_font(MTLayer, fonts_load_custom_font(f_handle));
	text_layer_set_text_alignment(MTLayer, GTextAlignmentRight);
	layer_add_child(window_get_root_layer(window), (Layer*) MTLayer);
	
	MULayer = text_layer_create(GRect(MUX, 53, 50, 60));
	text_layer_set_background_color(MULayer, GColorClear);
	text_layer_set_text_color(MULayer, GColorWhite);
	text_layer_set_font(MULayer, fonts_load_custom_font(f_handle));
	text_layer_set_text_alignment(MULayer, GTextAlignmentRight);
	layer_add_child(window_get_root_layer(window), (Layer*) MULayer);
	
//	setupTextLayer(HTLayer, GRect(HTX, 53, 50, 60), GColorClear, GColorWhite, f_handle, GTextAlignmentRight);	//One day!
//	setupTextLayer(HULayer, GRect(HUX, 53, 50, 60), GColorClear, GColorWhite, f_handle, GTextAlignmentRight);
//	setupTextLayer(colonLayer, GRect(69, 53, 50, 60), GColorClear, GColorWhite, f_handle, GTextAlignmentLeft);
//	setupTextLayer(MTLayer, GRect(MTX, 53, 50, 60), GColorClear, GColorWhite, f_handle, GTextAlignmentRight);
//	setupTextLayer(MULayer, GRect(MUX, 53, 50, 60), GColorClear, GColorWhite, f_handle, GTextAlignmentRight);
	
	//Allocate inverter layers
	HTInvLayer = inverter_layer_create(GRect(0, 0, INV_LAYER_WIDTH, 0));	
	HUInvLayer = inverter_layer_create(GRect(0, 0, INV_LAYER_WIDTH, 0));	
	bottomInvLayer = inverter_layer_create(GRect(0, 0, 144, 0));
	MTInvLayer = inverter_layer_create(GRect(0, 0, INV_LAYER_WIDTH, 0));	
	MUInvLayer = inverter_layer_create(GRect(0, 0, INV_LAYER_WIDTH, 0));

	layer_add_child(window_get_root_layer(window), (Layer*) HTInvLayer);	
	layer_add_child(window_get_root_layer(window), (Layer*) HUInvLayer);
	layer_add_child(window_get_root_layer(window), (Layer*) bottomInvLayer);
	layer_add_child(window_get_root_layer(window), (Layer*) MTInvLayer);		
	layer_add_child(window_get_root_layer(window), (Layer*) MUInvLayer);	

	
	//Make sure the face is not blank
	struct tm *t;
	
	time_t temp;	
	temp = time(NULL);	
	t = localtime(&temp);	
	getTimeDigits(t);
	
	//Stop 'all change' on first minute
	MUprev = MUDigit;
	MTprev = MTDigit;
	HUprev = HUDigit;
	HTprev = HTDigit;
	
	setTimeDigits();
}

/*
 * Unload window members
 */
static void window_unload(Window *window) {	
	//Free text layers
	text_layer_destroy(HTLayer);
	text_layer_destroy(HULayer);
	text_layer_destroy(colonLayer);
	text_layer_destroy(MTLayer);
	text_layer_destroy(MULayer);
	
	//Free inverter layers
	inverter_layer_destroy(HTInvLayer);
	inverter_layer_destroy(HUInvLayer);
	inverter_layer_destroy(bottomInvLayer);
	inverter_layer_destroy(MTInvLayer);
	inverter_layer_destroy(MUInvLayer);
	
	//Unsubscribe from events
	tick_timer_service_unsubscribe();
}

/*
 * Init app
 */
static void init(void) {
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
    });
	
	//Subscribe to events
	tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
	
	//Finally
  window_stack_push(window, true);
}

/*
 * De-init app
 */
static void deinit(void) {
	window_destroy(window);
}

/*
 * Entry point
 */
int main(void) {
	init();
	app_event_loop();
	deinit();
}

/*
 * Function definitions
 ***************************************************************************8
 */
/*
void setupTextLayer(TextLayer *layer, GRect location, GColor b_colour, GColor t_colour, ResHandle f_handle, GTextAlignment alignment)
{
	layer = text_layer_create(location);
	text_layer_set_background_color(layer, b_colour);
	text_layer_set_text_color(layer, t_colour);
	text_layer_set_font(layer, fonts_load_custom_font(f_handle));
	text_layer_set_text_alignment(layer, alignment);
	layer_add_child(window_get_root_layer(window), (Layer*) layer);
} */

/**
	* Function to get time digits
	*/
void getTimeDigits(struct tm *t) 
{
	//Hour string
	if(clock_is_24h_style())
		strftime(hourText, sizeof(hourText), "%H", t);
	else
		strftime(hourText, sizeof(hourText), "%I", t);

	//Minute string
	strftime(minuteText, sizeof(minuteText), "%M", t);
	
	//Get digits
	MUDigit = minuteText[1] - '0';
	MTDigit = minuteText[0] - '0';
	HUDigit = hourText[1] - '0';
	HTDigit = hourText[0] - '0';
}

/**
    * Function to set the time and date digits on the TextLayers
    */
void setTimeDigits() 
{
    //Copy digits
    HTText[0] = '0' + HTDigit;
    HUText[0] = '0' + HUDigit;
    MTText[0] = '0' + MTDigit;
    MUText[0] = '0' + MUDigit;
     
    //Fix digits for debugging purposes
    if(DEBUG) 
    {
        HTText[0] = '2';
        HUText[0] = '3';
        MTText[0] = '5';
        MUText[0] = '9';
    }
    
    //Set digits in TextLayers
    text_layer_set_text(HTLayer, HTText);
    text_layer_set_text(HULayer, HUText);
    text_layer_set_text(colonLayer, colonText);
    text_layer_set_text(MTLayer, MTText);
    text_layer_set_text(MULayer, MUText);
}

/**
    * Function to predict digit changes to make the digit change mechanic fire correctly.
    * If the values change at seconds == 0, then the animations depending on MUDigit != MUprev
    * will not fire! 
    * The solution is to advance the ones about to change pre-emptively
    */
void predictNextDigits(struct tm *t) {
    //These will be for the previous minute at 59 seconds
    getTimeDigits(t);
     
    //Fix hours tens
    if
    (
       ((HTDigit == 0) && (HUDigit == 9) && (MTDigit == 5) && (MUDigit == 9)) //09:59 --> 10:00
    || ((HTDigit == 1) && (HUDigit == 9) && (MTDigit == 5) && (MUDigit == 9))   //19:59 --> 20:00
    || ((HTDigit == 2) && (HUDigit == 3) && (MTDigit == 5) && (MUDigit == 9))   //23:59 --> 00:00
    )
    {
        HTDigit++;
    }
 
    //Fix hours units
    if((MTDigit == 5) && (MUDigit == 9))
    {
        HUDigit++;
    }
 
    //Fix minutes tens
    if(MUDigit == 9)    //Minutes Tens about to change
    {
        MTDigit++;
    }
         
    //Finally fix minutes units
    MUDigit++;
}

/**
	* Destory an animation to conserve memory
	* Source: https://github.com/fuzzie360/pebble-noms/blob/master/src/noms.c
	*/
void destroy_property_animation(PropertyAnimation *prop_animation) {
    if (prop_animation == NULL) {
        return;
    }

    if (animation_is_scheduled((Animation*) prop_animation)) {
        animation_unschedule((Animation*) prop_animation);
    }

    property_animation_destroy(prop_animation);
    prop_animation = NULL;
}

/**
	* Animation handlers
	*/
void animation_started(Animation *animation, void *data) {
    (void)animation;
    (void)data;
}

void animation_stopped(Animation *animation, void *data) {
    (void)data;
	
	//Teardown
	destroy_property_animation((PropertyAnimation*) animation);
}

/**
	* Function to linearly animate any layer between two GRects
	*/
void animateLayer(PropertyAnimation *animation, Layer *input, GRect startLocation, GRect finishLocation, int duration, int delay) 
{
	animation = property_animation_create_layer_frame(input, &startLocation, &finishLocation);
	animation_set_duration((Animation*) animation, duration);
	animation_set_delay((Animation*) animation, delay);
	animation_set_curve((Animation*) animation, AnimationCurveEaseInOut);
	animation_set_handlers((Animation*) animation, (AnimationHandlers) {
            .started = (AnimationStartedHandler) animation_started,
            .stopped = (AnimationStoppedHandler) animation_stopped
    }, 0);
	while(!animation_is_scheduled((Animation*) animation)) 
	{
		animation_schedule((Animation*) animation);
	}
}

