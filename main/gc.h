#ifndef GC_H
#define GC_H

//#include "driver/rmt.h"
#include <inttypes.h>
#include "driver/gpio.h"
#include "rom/gpio.h"
#include "rmt_gc.h"
#include "soc/rmt_struct.h"
#include "hal/clk_gate_ll.h"
#include "soc/io_mux_reg.h"
#include "soc/gpio_periph.h"
#include "driver/rmt.h"
#include "esp_log.h"

#define JB_RMT_GPIO     21
#define JB_TX_CHANNEL   0


#define RMT_RX_FREQDIV  20     
#define RMT_TX_FREQDIV  10     

#define RMT_TX_CHANNEL          RMT_CHANNEL_0
#define RMT_RX_CHANNEL          RMT_CHANNEL_1

#define JB_0L_TICKS     12
#define JB_0H_TICKS     4
#define JB_1L_TICKS     4
#define JB_1H_TICKS     12
#define JB_STOP_TICKS   8

#define JB_IDLE_TICKS   32

#define JB_LOW      (rmt_item32_t) {{{ JB_0L_TICKS,      0, JB_0H_TICKS, 1 }}}
#define JB_HIGH     (rmt_item32_t) {{{ JB_1L_TICKS,      0, JB_1H_TICKS, 1 }}}
#define JB_STOP     (rmt_item32_t) {{{ JB_STOP_TICKS,    0, 0,           1 }}}

#define JB_RMT_0X0  JB_LOW, JB_LOW, JB_LOW, JB_LOW
#define JB_RMT_0X9  JB_HIGH, JB_LOW, JB_LOW, JB_HIGH
#define JB_RMT_0X3  JB_LOW, JB_LOW, JB_HIGH, JB_HIGH
#define JB_RMT_0X8  JB_HIGH, JB_LOW, JB_LOW, JB_LOW
#define JB_RMT_0XF  JB_HIGH, JB_HIGH, JB_HIGH, JB_HIGH

#define JB_ZERO     (rmt_item32_t) {{{0,0,0,0}}}

// For ESP32 S3
#if CONFIG_IDF_TARGET_ESP32S3

#define JB_RX_CHANNEL   4

// FIFO Mem Access Bit Ptr
#define JB_RMT_FIFO_PTR     RMT.sys_conf.apb_fifo_mask

// Start transmission Ptr
// TX Channesl are CHN. RX are CHM
#define JB_TX_BEGIN_PTR     RMT.chnconf0[0].tx_start_chn
// Clock divider Ptr
#define JB_TX_CLKDIV_PTR    RMT.chnconf0[0].div_cnt_chn
// Block memory size Ptr
#define JB_TX_MEMSIZE_PTR   RMT.chnconf0[0].mem_size_chn
// Continuous Transmission Enable Ptr
#define JB_TX_CONT_PTR      RMT.chnconf0[0].tx_conti_mode_chn
// Carrier Enable Ptr
#define JB_TX_CARRIER_PTR   RMT.chnconf0[0].carrer_en_chn
// Memory owner Ptr
//#define JB_TX_MEMOWNER_PTR  UNUSED FOR TX ON ESP32S3
// Idle level Ptr
#define JB_TX_IDLELVL_PTR   RMT.chnconf0[0].idle_out_lv_chn
// Idle level enable Ptr
#define JB_TX_IDLEEN_PTR    RMT.chnconf0[0].idle_out_en_chn
// Transmission complete interrupt enable Ptr
#define JB_TX_COMPLETEISR_PTR   RMT.int_ena.ch0_tx_end_int_ena
// Memory RD Reset Ptr
#define JB_TX_RDRST_PTR     RMT.chnconf0[0].mem_rd_rst_chn
// Enable carrier Ptr
#define JB_TX_CARRIER_PTR   RMT.chnconf0[0].carrier_en_chn
// Memory access Ptr
#define JB_TX_MEM           RMTMEM.chan[0].data32

// Start receive Ptr
// TX Channesl are CHN. RX are CHM
#define JB_RX_EN_PTR        RMT.chmconf[0].conf1.rx_en_chm
// Clock divider Ptr
#define JB_RX_CLKDIV_PTR    RMT.chmconf[0].conf0.div_cnt_chm
// Block memory size Ptr
#define JB_RX_MEMSIZE_PTR   RMT.chmconf[0].conf0.mem_size_chm
// Memory owner Ptr
#define JB_RX_MEMOWNER_PTR  RMT.chmconf[0].conf1.mem_owner_chm
// Idle level Ptr
#define JB_RX_IDLETHRESH_PTR    RMT.chmconf[0].conf0.idle_thres_chm
// Receive complete interrupt enable Ptr
#define JB_RX_COMPLETEISR_PTR   RMT.int_ena.ch4_rx_end_int_ena
// Memory RD Reset Ptr
#define JB_RX_RDRST_PTR     RMT.chmconf[0].conf1.mem_wr_rst_chm
// Memory access Ptr
#define JB_RX_MEM           RMTMEM.chan[4].data32

// For ESP32 standard
#elif CONFIG_IDF_TARGET_ESP32

#define JB_RX_CHANNEL   1
// FIFO Mem Access Bit Ptr
#define JB_RMT_FIFO_PTR     RMT.apb_conf.fifo_mask
// Start transmission Ptr
// TX Channesl are CHN. RX are CHM
#define JB_TX_BEGIN_PTR     RMT.conf_ch[0].conf1.tx_start
// Clock divider Ptr
#define JB_TX_CLKDIV_PTR    RMT.conf_ch[0].conf0.div_cnt
// Block memory size Ptr
#define JB_TX_MEMSIZE_PTR   RMT.conf_ch[0].conf0.mem_size
// Continuous Transmission Enable Ptr
#define JB_TX_CONT_PTR      RMT.conf_ch[0].conf1.tx_conti_mode
// Carrier Enable Ptr
#define JB_TX_CARRIER_PTR   RMT.conf_ch[0].conf0.carrier_en
// Memory owner Ptr
#define JB_TX_MEMOWNER_PTR  RMT.conf_ch[0].conf1.mem_owner
// Idle level Ptr
#define JB_TX_IDLELVL_PTR   RMT.conf_ch[0].conf1.idle_out_lv
// Idle level enable Ptr
#define JB_TX_IDLEEN_PTR    RMT.conf_ch[0].conf1.idle_out_en
// Transmission complete interrupt enable Ptr
#define JB_TX_COMPLETEISR_PTR   RMT.int_ena.ch0_tx_end
// Memory RD Reset Ptr
#define JB_TX_RDRST_PTR     RMT.conf_ch[0].conf1.mem_rd_rst
// Memory access Ptr
#define JB_TX_MEM           RMTMEM.chan[0].data32

// Start receive Ptr
// TX Channesl are CHN. RX are CHM
#define JB_RX_EN_PTR        RMT.conf_ch[1].conf1.rx_en
// Clock divider Ptr
#define JB_RX_CLKDIV_PTR    RMT.conf_ch[1].conf0.div_cnt
// Block memory size Ptr
#define JB_RX_MEMSIZE_PTR   RMT.conf_ch[1].conf0.mem_size
// Memory owner Ptr
#define JB_RX_MEMOWNER_PTR  RMT.conf_ch[1].conf1.mem_owner
// Idle level Ptr
#define JB_RX_IDLETHRESH_PTR    RMT.conf_ch[0].conf0.idle_thres
// Receive complete interrupt enable Ptr
#define JB_RX_COMPLETEISR_PTR   RMT.int_ena.ch1_rx_end
// Memory RD Reset Ptr
#define JB_RX_RDRST_PTR     RMT.conf_ch[1].conf1.mem_rd_rst
// Memory access Ptr
#define JB_RX_MEM           RMTMEM.chan[1].data32

#endif

esp_err_t gamecube_reader_start();

#endif