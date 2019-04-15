#include "app_getData.h"

//static const nrf_drv_timer_t m_timer = NRF_DRV_TIMER_INSTANCE(1);
//static nrf_saadc_value_t     m_buffer_pool[2][SAMPLES_IN_BUFFER];
//static nrf_ppi_channel_t     m_ppi_channel;
//static uint32_t              m_adc_evt_counter;
extern uint8_t g_CapFlag;

//sadc预设值和实际值
int16_t  g_levelData[2][N_CAP_NODE] ={{0},{0}};
uint8_t  g_CurrentLevel = 0;
TagAppAdc  g_str_app_adc = {NRF_DRV_TIMER_INSTANCE(1),{0},NRF_PPI_CHANNEL0,0,0, &g_levelData[0][0]};

void timer_handler(nrf_timer_event_t event_type, void * p_context)
{
}
void saadc_sampling_event_init(void)
{
    ret_code_t err_code;

    err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
    err_code = nrf_drv_timer_init(&g_str_app_adc.timer, &timer_cfg, timer_handler);
    APP_ERROR_CHECK(err_code);

    /* setup m_timer for compare event every 400ms */
    uint32_t ticks = nrf_drv_timer_ms_to_ticks(&g_str_app_adc.timer, 40); //400
    nrf_drv_timer_extended_compare(&g_str_app_adc.timer,
                                   NRF_TIMER_CC_CHANNEL0,
                                   ticks,
                                   NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
                                   false);
    //nrf_drv_timer_enable(&g_str_app_adc.timer);   //by aizj

    uint32_t timer_compare_event_addr = nrf_drv_timer_compare_event_address_get(&g_str_app_adc.timer,
                                                                                NRF_TIMER_CC_CHANNEL0);
    uint32_t saadc_sample_task_addr   = nrf_drv_saadc_sample_task_get();

    /* setup ppi channel so that timer compare event is triggering sample task in SAADC */
    err_code = nrf_drv_ppi_channel_alloc(&g_str_app_adc.ppiChannel);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_ppi_channel_assign(g_str_app_adc.ppiChannel,
                                          timer_compare_event_addr,
                                          saadc_sample_task_addr);
    APP_ERROR_CHECK(err_code);
}


void saadc_sampling_event_enable(void)
{
    ret_code_t err_code = nrf_drv_ppi_channel_enable(g_str_app_adc.ppiChannel);

    APP_ERROR_CHECK(err_code);
}
void saadc_sampling_event_disable(void)
{
    ret_code_t err_code = nrf_drv_ppi_channel_disable(g_str_app_adc.ppiChannel);

    APP_ERROR_CHECK(err_code);
}

void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE && !g_str_app_adc.locked)  //采集完成
    {
        ret_code_t err_code;
		

        err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
        APP_ERROR_CHECK(err_code);
		
		//calc level
		int16_t aver = p_event->data.done.p_buffer[0]; //+ p_event->data.done.p_buffer[1]) >>1;
		int16_t  tmp;
		for(int i=0; i<N_CAP_NODE;i++){
			if( !(g_str_app_adc.levelEnable & 1<<i)){
				tmp = g_levelData[0][i];
				if( aver >=( g_levelData[0][i] - (tmp/(2<<i))) &&  aver <= (g_levelData[0][i] + (tmp/(2<<i))) ){
					captrue_cmd(START_CAP);
					captrue_cmd(DISABLE_ADC);
					g_str_app_adc.levelEnable |= (1<<i);
					g_levelData[1][i] = aver;
					g_CurrentLevel  = i;
				//	NRF_LOG_INFO("cap level[%d]:%d",i,aver);
					printf("cap level[%d]:%d",i,aver);
				}
			}
        }
		//NRF_LOG_INFO("adc: %d",aver);
	 }
  
}
void saadc_init(void)
{
    ret_code_t err_code;
    
    nrf_saadc_channel_config_t channel_config =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN6);

    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(g_str_app_adc.buf, SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);

   // err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[1], SAMPLES_IN_BUFFER);
   // APP_ERROR_CHECK(err_code);  // mask by aizj

}
void saadc_init2(void)
{
    ret_code_t err_code;
  
    nrf_saadc_channel_config_t channel_config =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN6);
    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);
}
int16_t saadc_getData(void)
{
//	nrfx_saadc_sample_convert(NRF_SAADC_INPUT_AIN6,g_levelData[0][0]);
	return g_levelData[0][0];
}
