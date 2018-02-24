/*******************************************************************************
 * (c) Copyright 2012 Microsemi SoC Products Group.  All rights reserved.
 *
 *  Smartfusion2 system configuration. This file is automatically generated
 *  by the Libero tools.
 *
 */
#ifndef MSS_SYSTEM_CONFIGURATION
#define MSS_SYSTEM_CONFIGURATION

/*==============================================================================
 * Clock configuration
 */
#include "sys_config_mss_clocks.h"
 
/*==============================================================================
 * Memory remapping configuration
 */
/* TBD */

/*==============================================================================
 * FACC_INIT (Cortex-M3 runs the FACC INIT procedure)
 * Only set to 1 for design targeting the M2S050T_ES device
 */
#define MSS_SYS_FACC_INIT_BY_CORTEX         0

/*==============================================================================
 * MDDR configuration
 */
#define MSS_SYS_MDDR_CONFIG_BY_CORTEX       0

/*==============================================================================
 * FDDR configuration
 */
#define MSS_SYS_FDDR_CONFIG_BY_CORTEX       0

/*==============================================================================
 * SERDES Interface configuration
 */
#define MSS_SYS_SERDES_0_CONFIG_BY_CORTEX   0
#if MSS_SYS_SERDES_0_CONFIG_BY_CORTEX
#include "sys_config_SERDESIF_0.h"
#endif

#define MSS_SYS_SERDES_1_CONFIG_BY_CORTEX   0
#if MSS_SYS_SERDES_1_CONFIG_BY_CORTEX
#include "sys_config_SERDESIF_1.h"
#endif

#define MSS_SYS_SERDES_2_CONFIG_BY_CORTEX   0
#if MSS_SYS_SERDES_2_CONFIG_BY_CORTEX
#include "sys_config_SERDESIF_2.h"
#endif

#define MSS_SYS_SERDES_3_CONFIG_BY_CORTEX   0
#if MSS_SYS_SERDES_3_CONFIG_BY_CORTEX
#include "sys_config_SERDESIF_3.h"
#endif

/*==============================================================================
 * Cache configuration
 */
#define MSS_SYS_CACHE_CONFIG_BY_CORTEX      0

#endif  /* MSS_SYSTEM_CONFIGURATION */


