#define F_CPU 32000000UL

#define ADC_REF_VOLTAGE 1
#define ADC_BIT_RES 4096

//Main Power rail ADC
#define MP_ON_THRESHOLD_V 0.5
#define MP_FAIL_THRESHOLD_V 0.5
#define MP_R_SERIES 12
#define MP_R_MEASURE 1
#define MP_ON_ADC_THRES ( ((float)MP_R_MEASURE / (float)(MP_R_SERIES + MP_R_MEASURE) ) * (ADC_BIT_RES / (float)ADC_REF_VOLTAGE) * (float)MP_ON_THRESHOLD_V )
#define MP_FAIL_ADC_THRES ( ((float)MP_R_MEASURE / (float)(MP_R_SERIES + MP_R_MEASURE) ) * (ADC_BIT_RES / (float)ADC_REF_VOLTAGE) * (float)MP_FAIL_THRESHOLD_V )

typedef enum {low = 0, high = 1, toggle} io_level_t;

//Output function

void enableCoreVoltage (io_level_t level);

void enable1_8V (io_level_t level);

void enable1_8VIO (io_level_t level);

void enable5V (io_level_t level);

//Input functions

int16_t readPGoodCore (void);

int16_t readPGood1_8V (void);

int16_t readPGood1_8VIO (void);

int16_t readPGood5V (void);

//ADC functions

void ADC_init(void); // Init ADCs

uint16_t MP_ADC_read(void); // Read Main Power Voltage ADC
