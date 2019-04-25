#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nrf.h"
#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "boards.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include <nrf_delay.h>

//spi
#define SPI2APP_CACHE  1
#define SPI2APP       1
#define CAP_ONE_TIME_BUF_LEN  (3000*2) //1952
#define MAX_SPI_BUF   50              //244
#define MAX_SPI2APP_CNT   (CAP_ONE_TIME_BUF_LEN / MAX_SPI_BUF )
#define SPI_INSTANCE  0 /**< SPI instance index. */

//ble
#define N_CAP_NODE    4
#define MAX_BLE_BUF   200 //MAX_SPI_BUF
#define MAX_BLE2APP_CNT  (CAP_ONE_TIME_BUF_LEN /MAX_BLE_BUF)

//sadc
#define ENABLE    1
#define DISABLE   0
#define SAMPLES_IN_BUFFER 4

typedef enum{
	LEVEL1=1,
	LEVEL2=2,
	LEVEL3=4,
	LEVEL4=8,
	ALL_LEVEL=0x3f
}tagCapLevel;

typedef enum{
	RESET_CAP,
	RESET_ALL,
	STOP_CAP,
	START_CAP,
	DISABLE_ADC,
	ENABLE_ADC,
}tagCapCmd;

typedef struct{
	nrf_drv_timer_t timer;
	nrf_saadc_value_t     buf[SAMPLES_IN_BUFFER];
	nrf_ppi_channel_t     ppiChannel;
	uint8_t               locked;
	uint8_t               levelEnable;
	int16_t *             pLevelData;
}TagAppAdc;
extern TagAppAdc  g_str_app_adc;
extern uint8_t  g_CurrentLevel ;
extern int16_t  g_levelData[2][N_CAP_NODE];
extern uint8_t g_save[];
void saadc_init(void);
int16_t saadc_getData(void);
void saadc_sampling_event_init(void);
void saadc_sampling_event_enable(void);
void saadc_sampling_event_disable(void);
extern void captrue_cmd(uint8_t type);
