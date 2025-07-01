/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


#ifndef NX_STM32_ETH_DRIVER_H
#define NX_STM32_ETH_DRIVER_H


#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {
#endif


/* Include ThreadX header file, if not already.  */

#ifndef TX_API_H
#include "tx_api.h"
#endif


/* Include NetX header file, if not already.  */

#ifndef NX_API_H
#include "nx_api.h"
#endif

/* Include NetX Link Layer header file, if not already.  */
#ifndef _NX_LINK_H_
#include "nx_link.h"
#endif

/* Determine if the driver's source file is being compiled. The constants and typdefs are only valid within
   the driver's source file compilation.  */

#ifdef NX_DRIVER_SOURCE

/****** DRIVER SPECIFIC ****** Start of part/vendor specific include area.  Include any such files here!  */
#include "nx_stm32_eth_config.h"
#ifdef MULTI_QUEUE_FEATURE
#include "nx_shaper.h"
#endif

/****** DRIVER SPECIFIC ****** End of part/vendor specific include file area!  */

#ifdef NX_DRIVER_ENABLE_PTP
#include "nxd_ptp_client.h"
typedef UINT (*TIMESTAMP_CALLBACK)(NX_PACKET *packet_ptr, UINT timestamp_type, NX_PTP_TIME* timestamp);
UINT nx_driver_set_timestamp_callback(USHORT type, TIMESTAMP_CALLBACK callback);

#endif /* NX_DRIVER_ENABLE_PTP */

/* Define generic constants and macros for all NetX Ethernet drivers.  */

#define NX_DRIVER_ETHERNET_IP                   0x0800
#define NX_DRIVER_ETHERNET_IPV6                 0x86dd
#define NX_DRIVER_ETHERNET_ARP                  0x0806
#define NX_DRIVER_ETHERNET_RARP                 0x8035

#define NX_DRIVER_ETHERNET_MTU                  1514
#define NX_DRIVER_ETHERNET_FRAME_SIZE           14

#ifdef MULTI_QUEUE_FEATURE
#define NX_DRIVER_DEFERRED_PACKET_RECEIVED_CH0      1
#define NX_DRIVER_DEFERRED_PACKET_RECEIVED_CH1      2
#define NX_DRIVER_DEFERRED_PACKET_TRANSMITTED_CH0   4
#define NX_DRIVER_DEFERRED_PACKET_TRANSMITTED_CH1   8
#else
#define NX_DRIVER_DEFERRED_PACKET_RECEIVED      1
#define NX_DRIVER_DEFERRED_DEVICE_RESET         2
#define NX_DRIVER_DEFERRED_PACKET_TRANSMITTED   4
#endif

#define NX_DRIVER_STATE_NOT_INITIALIZED         1
#define NX_DRIVER_STATE_INITIALIZE_FAILED       2
#define NX_DRIVER_STATE_INITIALIZED             3
#define NX_DRIVER_STATE_LINK_ENABLED            4

#define NX_DRIVER_ERROR                         90

#define NX_DRIVER_ETHERNET_HEADER_REMOVE(p)   \
{   \
    p -> nx_packet_prepend_ptr =  p -> nx_packet_prepend_ptr + NX_DRIVER_ETHERNET_FRAME_SIZE;  \
    p -> nx_packet_length =  p -> nx_packet_length - NX_DRIVER_ETHERNET_FRAME_SIZE;    \
}

/* Define PORT rate 10M / 100M / 1000M */
#define PORT_RATE 100

#ifdef NX_DRIVER_ENABLE_CBS
#define AVB_IDLE_SLOPE_CYCLE_FACTOR_100M 25000000 /* 1/40ns */
#define AVB_IDLE_SLOPE_CYCLE_FACTOR_1000M 125000000 /* 1/8ns */

#define AVB_HI_CREDIT 0x08000000
#define AVB_LO_CREDIT 0x18000000

#define AVB_MAX_PORT_TRANSIT_RATE_100M  (((uint64_t)100000000 * 1024) / AVB_IDLE_SLOPE_CYCLE_FACTOR_100M)
#define AVB_MAX_PORT_TRANSIT_RATE_1000M (((uint64_t)1000000000 * 1024) / AVB_IDLE_SLOPE_CYCLE_FACTOR_1000M)
#endif /* NX_DRIVER_ENABLE_CBS */

/**************************************************************************/
/* Cache maintenance. */
/**************************************************************************/
#if defined(__CORTEX_M)
#if (defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U))
#define invalidate_cache_by_addr(__ptr__, __size__)                  SCB_InvalidateDCache_by_Addr((void *)(__ptr__), (int32_t)(__size__))
#define clean_cache_by_addr(__ptr__, __size__)                       SCB_CleanDCache_by_Addr((uint32_t *)(__ptr__), (int32_t)(__size__))
#else
#define invalidate_cache_by_addr(__ptr__, __size__)
#define clean_cache_by_addr(__ptr__, __size__)
#endif
#else
#if defined(DATA_CACHE_ENABLE) && (DATA_CACHE_ENABLE == 1U)
__STATIC_FORCEINLINE void __invalidate_cache_by_addr(uint32_t start, uint32_t size)
{
  uint32_t current = start & ~31U;
  uint32_t end = (start + size + 31U) & ~31U;
  while (current < end)
  {
    L1C_CleanInvalidateDCacheMVA((void*)current); /* We clean also because buffers are not 32-byte aligned and read is done after this anyway. */
    current += 32U;
  }
}

__STATIC_FORCEINLINE void __clean_cache_by_addr(uint32_t start, uint32_t size)
{
  uint32_t current = start & ~31U;
  uint32_t end = (start + size + 31U) & ~31U;
  while (current < end)
  {
    L1C_CleanDCacheMVA((void*)current);
    current += 32U;
  }
}
#define invalidate_cache_by_addr(__ptr__, __size__) __invalidate_cache_by_addr((uint32_t)(__ptr__), (uint32_t)(__size__))
#define clean_cache_by_addr(__ptr__, __size__) __clean_cache_by_addr((uint32_t)(__ptr__), (uint32_t)(__size__))
#else
#define invalidate_cache_by_addr(__ptr__, __size__)
#define clean_cache_by_addr(__ptr__, __size__)
#endif
#endif

/****** DRIVER SPECIFIC ****** Start of part/vendor specific constants area.  Include any such constants here!  */

/* Define the number of descriptors and attached packets for transmit and receive operations. */

#ifndef NX_DRIVER_TX_DESCRIPTORS
#define NX_DRIVER_TX_DESCRIPTORS   ETH_TX_DESC_CNT
#endif

#ifndef NX_DRIVER_RX_DESCRIPTORS
#define NX_DRIVER_RX_DESCRIPTORS   ETH_RX_DESC_CNT
#endif

/* Define the number of packets to be transmitted before they are released.
* This value must be strictly below the number of packets in the smallest packet pool.
* It must not exceed the number of available TX descriptors */
#ifndef NX_DRIVER_TX_RELEASE_THRESHOLD
#define NX_DRIVER_TX_RELEASE_THRESHOLD (1U)
#endif

#ifndef PHY_LINK_TIMEOUT
#define PHY_LINK_TIMEOUT (5000U)
#endif

/****** DRIVER SPECIFIC ****** End of part/vendor specific constant area!  */
#ifndef NX_DRIVER_ENABLE_PTP
#define NX_DRIVER_CAPABILITY ( NX_INTERFACE_CAPABILITY_IPV4_TX_CHECKSUM   | \
                               NX_INTERFACE_CAPABILITY_IPV4_RX_CHECKSUM   | \
                               NX_INTERFACE_CAPABILITY_TCP_TX_CHECKSUM    | \
                               NX_INTERFACE_CAPABILITY_TCP_RX_CHECKSUM    | \
                               NX_INTERFACE_CAPABILITY_UDP_TX_CHECKSUM    | \
                               NX_INTERFACE_CAPABILITY_UDP_RX_CHECKSUM    | \
                               NX_INTERFACE_CAPABILITY_ICMPV4_TX_CHECKSUM   | \
                               NX_INTERFACE_CAPABILITY_ICMPV4_RX_CHECKSUM   | \
                               NX_INTERFACE_CAPABILITY_ICMPV6_TX_CHECKSUM | \
                               NX_INTERFACE_CAPABILITY_ICMPV6_RX_CHECKSUM )
#else
#define NX_DRIVER_CAPABILITY ( NX_INTERFACE_CAPABILITY_IPV4_TX_CHECKSUM   | \
                               NX_INTERFACE_CAPABILITY_IPV4_RX_CHECKSUM   | \
                               NX_INTERFACE_CAPABILITY_TCP_TX_CHECKSUM    | \
                               NX_INTERFACE_CAPABILITY_TCP_RX_CHECKSUM    | \
                               NX_INTERFACE_CAPABILITY_UDP_TX_CHECKSUM    | \
                               NX_INTERFACE_CAPABILITY_UDP_RX_CHECKSUM    | \
                               NX_INTERFACE_CAPABILITY_ICMPV4_TX_CHECKSUM   | \
                               NX_INTERFACE_CAPABILITY_ICMPV4_RX_CHECKSUM   | \
                               NX_INTERFACE_CAPABILITY_ICMPV6_TX_CHECKSUM | \
                               NX_INTERFACE_CAPABILITY_ICMPV6_RX_CHECKSUM | \
                               NX_INTERFACE_CAPABILITY_PTP_TIMESTAMP)
#endif /* NX_DRIVER_ENABLE_PTP */

/* Define basic Ethernet driver information typedef. Note that this typedefs is designed to be used only
   in the driver's C file. */

typedef struct NX_DRIVER_INFORMATION_STRUCT
{
    /* NetX IP instance that this driver is attached to.  */
    NX_IP               *nx_driver_information_ip_ptr;

    /* Driver's current state.  */
    ULONG               nx_driver_information_state ;

    /* Packet pool used for receiving packets. */
    NX_PACKET_POOL      *nx_driver_information_packet_pool_ptr;

    /* Define the driver interface association.  */
    NX_INTERFACE        *nx_driver_information_interface;

    /* Define the deferred event field. This will contain bits representing events
       deferred from the ISR for processing in the thread context.  */
    ULONG               nx_driver_information_deferred_events;


    /****** DRIVER SPECIFIC ****** Start of part/vendor specific driver information area.  Include any such constants here!  */

    /* Indices to current receive/transmit descriptors.  */
    UINT                nx_driver_information_receive_current_index;
    UINT                nx_driver_information_transmit_current_index;

    /* Transmit release index.  */
    UINT                nx_driver_information_transmit_release_index;

    /* Define the number of transmit buffers in use.  */
#ifdef ETH_MULTIQUEUE_SUPPORTED
    UINT                nx_driver_information_number_of_transmit_buffers_in_use[ETH_DMA_TX_CH_CNT];
#else
    UINT                nx_driver_information_number_of_transmit_buffers_in_use;
#endif
    /* Define the association between buffer descriptors and NetX packets.  */
    NX_PACKET           *nx_driver_information_transmit_packets[NX_DRIVER_TX_DESCRIPTORS];
    NX_PACKET           *nx_driver_information_receive_packets[NX_DRIVER_RX_DESCRIPTORS];

    /* Define the size of a rx buffer size.  */
    ULONG               nx_driver_information_rx_buffer_size;

    ULONG               nx_driver_information_multicast_count;

#ifdef NX_DRIVER_ENABLE_PTP
    NX_PTP_CLIENT     *nx_driver_ptp_ptr;
#endif /* NX_DRIVER_ENABLE_PTP */

    /****** DRIVER SPECIFIC ****** End of part/vendor specific driver information area.  */

}   NX_DRIVER_INFORMATION;

#endif


/****** DRIVER SPECIFIC ****** Start of part/vendor specific external function prototypes.  A typical NetX Ethernet driver
                               should expose its entry function as well as its interrupt handling function(s) here. All other
                               functions in the driver should have local scope, i.e., defined as static.  */

/* Define global driver entry function. */

VOID  nx_stm32_eth_driver(NX_IP_DRIVER *driver_req_ptr);

#ifdef NX_DRIVER_ENABLE_PTP
VOID  nx_driver_handle_ptp(NX_PACKET *release_packet);
UINT  nx_driver_ptp_clock_callback(NX_PTP_CLIENT *client_ptr, UINT operation,
                                             NX_PTP_TIME *time_ptr, NX_PACKET *packet_ptr,
                                             VOID *callback_data);
#endif /* NX_DRIVER_ENABLE_PTP */

#ifdef MULTI_QUEUE_FEATURE
UINT nx_driver_shaper_config(NX_SHAPER_DRIVER_PARAMETER* parameter, UCHAR shaper_capability);
#endif

#ifdef NX_DRIVER_ENABLE_CBS
UINT nx_driver_shaper_cbs_entry(NX_SHAPER_DRIVER_PARAMETER *parameter);
#endif

#ifdef NX_DRIVER_ENABLE_FPE
UINT nx_driver_shaper_fpe_entry(NX_SHAPER_DRIVER_PARAMETER *parameter);
#endif

#ifdef NX_DRIVER_ENABLE_TAS
#ifdef NX_DRIVER_ENABLE_FPE
void reconfig_fpe_gcl(NX_SHAPER_TAS_PARAMETER *parameter,UCHAR queue_bit);
#endif
UINT nx_driver_shaper_tas_entry(NX_SHAPER_DRIVER_PARAMETER *parameter);
#endif

#if (defined(MULTI_QUEUE_FEATURE) || defined(NX_DRIVER_ENABLE_PTP)) && !defined(NX_ENABLE_VLAN)
#error "VLAN is required for MULTI-QUEUE and PTP Features, NX_ENABLE_VLAN flag should be defined in nx_user.h"
#endif

#if defined(NX_DRIVER_ENABLE_CBS) && !defined(MULTI_QUEUE_FEATURE)
#error "MULTI_QUEUE_FEATURE is required for Credit Based Shaper, MULTI_QUEUE_FEATURE flag should be defined"
#endif

#if defined(NX_DRIVER_ENABLE_FPE) && !defined(MULTI_QUEUE_FEATURE)
#error "MULTI_QUEUE_FEATURE is required for Frame Preemption Shaper, MULTI_QUEUE_FEATURE flag should be defined"
#endif

#if defined(NX_DRIVER_ENABLE_TAS) && !(defined(NX_DRIVER_ENABLE_PTP) && defined(MULTI_QUEUE_FEATURE))
#error "MULTI-QUEUE and PTP Features are required for Time Aware Shaper, respective flags should be defined"
#endif
/****** DRIVER SPECIFIC ****** End of part/vendor specific external function prototypes.  */


#ifdef   __cplusplus
/* Yes, C++ compiler is present.  Use standard C.  */
    }
#endif
#endif /* NX_STM32_ETH_DRIVER_H */
