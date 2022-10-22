#ifndef RMT_GC_H
#define RMT_GC_H

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_err.h"
#include "soc/soc_caps.h"
#include "hal/rmt_types.h"
#include "hal/rmt_hal.h"
#include "hal/dma_types.h"
#include "esp_intr_alloc.h"
#include "esp_heap_caps.h"
#include "esp_pm.h"
#include "esp_attr.h"
#include "esp_private/gdma.h"
#include "driver/rmt_types_legacy.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "driver/gpio.h"
#include "rom/gpio.h"
#include "rmt_gc.h"
#include "soc/rmt_struct.h"
#include "hal/clk_gate_ll.h"
#include "soc/io_mux_reg.h"
#include "soc/gpio_periph.h"
#include "driver/rmt.h"
#include "esp_log.h"
#include "hal/rmt_ll.h"

typedef struct {
    struct {
        volatile rmt_item32_t data32[SOC_RMT_MEM_WORDS_PER_CHANNEL];
    } chan[SOC_RMT_CHANNELS_PER_GROUP];
} legacy_rmt_mem_t;

// RMTMEM address is declared in <target>.peripherals.ld
extern legacy_rmt_mem_t RMTMEM;

#define JB_P1_GPIO      GPIO_NUM_5
#define JB_P2_GPIO      GPIO_NUM_6
#define JB_P3_GPIO      GPIO_NUM_7
#define JB_P4_GPIO      GPIO_NUM_8

#define RMT_TX_CHANNEL          RMT_CHANNEL_0
#define RMT_RX_CHANNEL          RMT_CHANNEL_4

#define JB_0L_TICKS     15
#define JB_0H_TICKS     5
#define JB_1L_TICKS     5
#define JB_1H_TICKS     15
#define JB_STOP_TICKS   7

#define JB_IDLE_TICKS   100

#define JB_LOW      (rmt_item32_t) {{{ JB_0L_TICKS,      0, JB_0H_TICKS, 1 }}}
#define JB_HIGH     (rmt_item32_t) {{{ JB_1L_TICKS,      0, JB_1H_TICKS, 1 }}}
#define JB_STOP     (rmt_item32_t) {{{ JB_STOP_TICKS,    0, 0,           1 }}}

#define JB_RMT_0X0  JB_LOW, JB_LOW, JB_LOW, JB_LOW
#define JB_RMT_0X1  JB_LOW, JB_LOW, JB_LOW, JB_HIGH 
#define JB_RMT_0X4  JB_LOW, JB_HIGH, JB_LOW, JB_LOW
#define JB_RMT_0X9  JB_HIGH, JB_LOW, JB_LOW, JB_HIGH
#define JB_RMT_0X3  JB_LOW, JB_LOW, JB_HIGH, JB_HIGH
#define JB_RMT_0X8  JB_HIGH, JB_LOW, JB_LOW, JB_LOW
#define JB_RMT_0XF  JB_HIGH, JB_HIGH, JB_HIGH, JB_HIGH

#define JB_ZERO     (rmt_item32_t) {{{0,0,0,0}}}

// FIFO Mem Access Bit Ptr
#define JB_RMT_FIFO     RMT.sys_conf.apb_fifo_mask
// RMT Clock Select
#define JB_RMT_CLKSEL   RMT.sys_conf.sclk_sel
// Clock Enable Ptr
#define JB_TX_CLKEN     RMT.sys_conf.sclk_active

// Start transmission Ptr
// TX Channesl are CHN. RX are CHM
#define JB_TX_BEGIN         RMT.chnconf0[RMT_TX_CHANNEL].tx_start_chn
// Clock divider Ptr
#define JB_TX_CLKDIV        RMT.chnconf0[RMT_TX_CHANNEL].div_cnt_chn

// Block memory size Ptr
#define JB_TX_MEMSIZE       RMT.chnconf0[RMT_TX_CHANNEL].mem_size_chn
// Continuous Transmission Enable Ptr
#define JB_TX_CONT          RMT.chnconf0[RMT_TX_CHANNEL].tx_conti_mode_chn
// Carrier Enable Ptr
#define JB_TX_CARRIER       RMT.chnconf0[RMT_TX_CHANNEL].carrier_en_chn
#define JB_TX_CARRIER2      RMT.chnconf0[RMT_TX_CHANNEL].carrier_eff_en_chn

// Write Config Data Sync Bit
#define JB_TX_SYNC          RMT.chnconf0[RMT_TX_CHANNEL].conf_update_chn

// Idle level Ptr
#define JB_TX_IDLELVL       RMT.chnconf0[RMT_TX_CHANNEL].idle_out_lv_chn
// Idle level enable Ptr
#define JB_TX_IDLEEN        RMT.chnconf0[RMT_TX_CHANNEL].idle_out_en_chn

// Memory RD Reset Ptr
#define JB_TX_RDRST         RMT.chnconf0[RMT_TX_CHANNEL].mem_rd_rst_chn
// Memory WR Reset Ptr
#define JB_TX_WRRST         RMT.chnconf0[RMT_TX_CHANNEL].apb_mem_rst_chn
// Memory access Ptr
#define JB_TX_MEM           RMTMEM.chan[RMT_TX_CHANNEL].data32

// Transmission complete interrupt enable Ptr
#define JB_TX_ENAISR        RMT.int_ena.ch0_tx_end_int_ena
// Transmission complete interrupt clear Ptr
#define JB_TX_CLEARISR      RMT.int_clr.ch0_tx_end_int_clr
// Transmission complete interrupt status Ptr
#define JB_TX_STATISR       RMT.int_st.ch0_tx_end_int_st

// Start receive Ptr
// TX Channesl are CHN. RX are CHM
#define JB_RX_BEGIN         RMT.chmconf[RMT_RX_CHANNEL-4].conf1.rx_en_chm
// Clock divider Ptr
#define JB_RX_CLKDIV        RMT.chmconf[RMT_RX_CHANNEL-4].conf0.div_cnt_chm
// Block memory size Ptr
#define JB_RX_MEMSIZE       RMT.chmconf[RMT_RX_CHANNEL-4].conf0.mem_size_chm
// Memory owner Ptr
#define JB_RX_MEMOWNER      RMT.chmconf[RMT_RX_CHANNEL-4].conf1.mem_owner_chm
// Idle level Ptr
#define JB_RX_IDLETHRESH    RMT.chmconf[RMT_RX_CHANNEL-4].conf0.idle_thres_chm
// Filter Enable Ptr
#define JB_RX_FILTEREN      RMT.chmconf[RMT_RX_CHANNEL-4].conf1.rx_filter_en_chm
// Filter thresh Ptr
#define JB_RX_FILTERTHR     RMT.chmconf[RMT_RX_CHANNEL-4].conf1.rx_filter_thres_chm
// Memory RD Reset Ptr
#define JB_RX_RDRST         RMT.chmconf[RMT_RX_CHANNEL-4].conf1.mem_wr_rst_chm
// RX Carrier Demodulate Enable
#define JB_RX_CARRIER       RMT.chmconf[RMT_RX_CHANNEL-4].conf0.carrier_en_chm

// Memory access Ptr
#define JB_RX_MEM           RMTMEM.chan[RMT_RX_CHANNEL].data32
#define JB_RX_MEM2          RMTMEM.chan[RMT_RX_CHANNEL+1].data32

// RX received interrupt enable Ptr
#define JB_RX_ENAISR        RMT.int_ena.ch4_rx_end_int_ena
// RX complete interrupt clear Ptr
#define JB_RX_CLEARISR      RMT.int_clr.ch4_rx_end_int_clr
// RX complete interrupt status Ptr
#define JB_RX_STATISR       RMT.int_st.ch4_rx_end_int_st

// Write Config Data Sync Bit
#define JB_RX_SYNC          RMT.chmconf[RMT_RX_CHANNEL-4].conf1.conf_update_chm

#endif
