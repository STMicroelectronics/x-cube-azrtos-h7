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

/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Component                                                        */
/**                                                                       */
/**   User Specific                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  PORT SPECIFIC C INFORMATION                            RELEASE        */
/*                                                                        */
/*    nx_user.h                                           PORTABLE C      */
/*                                                           6.1.11       */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains user defines for configuring NetX in specific    */
/*    ways. This file will have an effect only if the application and     */
/*    NetX library are built with NX_INCLUDE_USER_DEFINE_FILE defined.    */
/*    Note that all the defines in this file may also be made on the      */
/*    command line when building NetX library and application objects.    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  08-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            supported TCP/IP offload,   */
/*                                            resulting in version 6.1.8  */
/*  04-25-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/

#ifndef NX_USER_H
#define NX_USER_H

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/* Define various build options for the NetX Duo port. The application should
   either make changes here by commenting or un-commenting the conditional
   compilation defined OR supply the defines though the compiler's equivalent
   of the -D option.  */

/* Override various options with default values already assigned in nx_api.h
   or nx_port.h. Please also refer to nx_port.h for descriptions on each of
   these options.  */

/*****************************************************************************/
/************** Configuration options for NetXDuo Core ***********************/
/*****************************************************************************/

/* NX_MAX_PHYSICAL_INTERFACES defines the number physical network interfaces
   present to NetX Duo IP layer.  Physical interface does not include
   loopback interface. By default there is at least one physical interface
   in the system. */
/*
#define NX_MAX_PHYSICAL_INTERFACES              1
*/

/* If defined, the link driver is able to specify extra capability, such as
   checksum offloading features. */
/*
#define NX_ENABLE_INTERFACE_CAPABILITY
*/

/* NX_PHYSICAL_HEADER Specifies the size in bytes of the physical header of
   the frame. */
/*
#define NX_PHYSICAL_HEADER            16
*/

/* NX_PHYSICAL_TRAILER specifies the size in bytes of the physical packet
   trailer and is typically used to reserve storage for things like Ethernet
   CRCs, etc. */
/* Define the max string length. The default value is 1024.  */
/*
#define NX_PHYSICAL_TRAILER                     4
*/

/* Defined, this option bypasses the basic NetX error checking. This define
   is typically used after the application is fully debugged.
*/
/*
#define NX_DISABLE_ERROR_CHECKING
*/

/* Defined, this option enables deferred driver packet handling. This allows
   the driver to place a raw packet on the IP instance and have the driver's
   real processing routine called from the NetX internal IP helper thread. */
/*
#define NX_DRIVER_DEFERRED_PROCESSING
*/

/* Defined, the source address of incoming packet is checked. The default is
   disabled. */
/*
#define NX_ENABLE_SOURCE_ADDRESS_CHECK
*/

/* Defined, ASSERT is disabled. The default is enabled. */
/*
#define NX_DISABLE_ASSERT
*/

/* Defined, ASSERT is disabled. The default is enabled. */
/*
#define NX_ASSERT_FAIL              for (;;) {tx_thread_sleep(NX_WAIT_FOREVER); }
*/

/* Define the max string length. The default value is 1024.  */
/*
#define NX_MAX_STRING_LENGTH                    1024
*/

/* Defined, enables the optional debug packet dumping available in the RAM
   Ethernet network driver.  */
/*
#define NX_DEBUG_PACKET
*/

/* Defined, enables the optional print debug information available from the RAM
   Ethernet network driver. */
/*
#define NX_DEBUG
*/

/* Defined, the extended notify support is enabled.  This feature adds additional
   callback/notify services to NetX Duo API for notifying the application of
   socket events, such as TCP connection and disconnect completion.
   These extended notify functions are mainly used by the BSD wrapper.
   The default is this feature is disabled.  */
/*
#define NX_ENABLE_EXTENDED_NOTIFY_SUPPORT
*/

/* Defined, NetX Duo is built with NAT process. This option is defined when the
   component Addons NAT is selected and NX_DISABLE_IPV4 is not selected. */
/*
#define NX_NAT_ENABLE
*/

/* Defined, allows the stack to use two packet pools, one with large payload
   size and one with smaller payload size. By default this option is not
   enabled. */
/*
#define NX_ENABLE_DUAL_PACKET_POOL
*/

/*****************************************************************************/
/***************** Configuration options for Packet **************************/
/*****************************************************************************/

/* Defined, packet header and payload are aligned automatically by the value.
   The default value is sizeof(ULONG). */
/*
#define NX_PACKET_ALIGNMENT               sizeof(ULONG)
*/

/* Defined, packet debug information is enabled.  */
/*
#define NX_ENABLE_PACKET_DEBUG_INFO
*/

/* Defined, disables packet pool information gathering. */
/*
#define NX_DISABLE_PACKET_INFO
*/

/* Defined, enables NetX Duo packet pool low watermark feature. Application
   sets low watermark value. On receiving TCP packets, if the packet pool
   low watermark is reached, NetX Duo silently discards the packet by releasing
   it, preventing the packet pool from starvation. By default this feature is
   not enabled. */
/*
#define NX_ENABLE_LOW_WATERMARK
*/

/*****************************************************************************/
/************* Configuration options for Neighbor Cache **********************/
/*****************************************************************************/

/* Define the length of time, in milliseconds, between re-transmitting
   Neighbor Solicitation (NS) packets. */
/*
#define NX_RETRANS_TIMER                        1000
*/

/* Defined, this option disables Duplicate Address Detection (DAD) during IPv6
   address assignment. Addresses are set either by manual configuration or
   through Stateless Address Auto Configuration. */
/*
#define NX_DISABLE_IPV6_DAD
*/

/* Defined, this option prevents NetX Duo from removing stale (old) cache table
   entries whose timeout has not expired so are otherwise still valid) to make
   room for new entries when the table is full. Static and router entries are
   not purged.  */
/*
#define NX_DISABLE_IPV6_PURGE_UNUSED_CACHE_ENTRIES
*/

/* Specifies the number of Neighbor Solicitation messages to be sent before
   NetX Duo marks an interface address as valid. If NX_DISABLE_IPV6_DAD is
   defined (DAD disabled), setting this option has no effect. Alternatively,
   a value of zero (0) turns off DAD but leaves the DAD functionality in
   NetX Duo. Defined in nx_api.h, the default value is 3.  */
/*
#define NX_IPV6_DAD_TRANSMITS               3
*/

/* Specifies the number of entries in the IPv6 Neighbor Cache table. Defined
   in nx_nd_cache.h, the default value is 16. */
/*
#define NX_IPV6_NEIGHBOR_CACHE_SIZE         16
*/

/* Specifies the delay in seconds before the first solicitation is sent out for
   a cache entry in the STALE state. Defined in nx_nd_cache.h, the default
   value is 5. */
/*
#define NX_DELAY_FIRST_PROBE_TIME           5
*/

/* Specifies the number of Neighbor Solicitation messages NetX Duo transmits as
   part of the IPv6 Neighbor Discovery protocol when mapping between IPv6
   address and MAC address is required. Defined in nx_nd_cache.h, the default
   value is 3. */
/*
#define NX_MAX_MULTICAST_SOLICIT            3
*/

/* Specifies the number of Neighbor Solicitation messages NetX Duo transmits
   to determine a specific neighbor's reachability. Defined in nx_nd_cache.h,
   the default value is 3. */
/*
#define NX_MAX_UNICAST_SOLICIT              3
*/

/* This defines specifies the maximum number of packets that can be queued while waiting for a
   Neighbor Discovery to resolve an IPv6 address. The default value is 4.  */
/*
#define NX_ND_MAX_QUEUE_DEPTH               4
*/

/* Specifies the time out in seconds for a cache entry to exist in the REACHABLE
   state with no packets received from the cache destination IPv6 address.
   Defined in nx_nd_cache.h, the default value is 30. */
/*
#define NX_REACHABLE_TIME                   30
*/

/*****************************************************************************/
/********************* Configuration options for IP **************************/
/*****************************************************************************/

/* Defined, this option disables NetX Duo support on the 127.0.0.1 loopback
   interface. 127.0.0.1 loopback interface is enabled by default.
   Uncomment out the follow code to disable the loopback interface. */

/*
#define NX_DISABLE_LOOPBACK_INTERFACE
*/

/* Defined, this option disables the addition size checking on received packets. */
/*
#define NX_DISABLE_RX_SIZE_CHECKING
*/

/* The symbol "NX_IP_PERIODIC_RATE" specifies the number of ThreadX timer ticks in one second.
   The value should be derived from TX_TIMER_TICKS_PER_SECOND, which is defined in ThreadX port.
   If TX_TIMER_TICKS_PER_SECOND is not defined, NX_IP_PERIODIC_RATE defaults to 100 ticks per second. */

/*
#ifdef TX_TIMER_TICKS_PER_SECOND
#define NX_IP_PERIODIC_RATE                     TX_TIMER_TICKS_PER_SECOND
#else
#define NX_IP_PERIODIC_RATE                     100
#endif
*/

/* Defined, NX_ENABLE_IP_RAW_PACKET_FILTER allows an application to install a
   filter for incoming raw packets. This feature is disabled by default. */
/*
#define NX_ENABLE_IP_RAW_PACKET_FILTER
*/

/* This define specifies the maximum number of RAW packets can be queued for
   receive. The default value is 20.  */
/*
#define NX_IP_RAW_MAX_QUEUE_DEPTH             20
*/

/* Defined, this option enables IP static routing feature. By default IP static
   routing feature is not compiled in. */
/*
#define NX_ENABLE_IP_STATIC_ROUTING
*/

/* This define specifies the maximum time of IP reassembly.  The default value
   is 60. By default this option is not defined.  */
/*
#define NX_IP_MAX_REASSEMBLY_TIME       60
*/

/* Symbol that controls maximum time allowed to reassemble IPv4 fragment.
   Note the value defined in NX_IP_MAX_REASSEMBLY_TIME overwrites this value.
   The default value is 15. */
/*
#define NX_IPV4_MAX_REASSEMBLY_TIME       15
*/

/* This define specifies the maximum time of IPv6 reassembly. The default value
   is 60. Note that if NX_IP_MAX_REASSEMBLY_TIME is defined, this option is
   automatically defined as 60. By default this option is not defined. */
/*
#define NX_IPV6_MAX_REASSEMBLY_TIME 60
*/

/* Defined, this option disables checksum logic on received IP packets.
   This is useful if the link-layer has reliable checksum or CRC logic. */
/*
#define NX_DISABLE_IP_RX_CHECKSUM
*/

/* Defined, this option disables checksum logic on transmitted IP packets. */
/*
#define NX_DISABLE_IP_TX_CHECKSUM
*/

/* Defined, IP information gathering is disabled.  */
/*
#define NX_DISABLE_IP_INFO
*/

/* This define IP fast timer rate. The default value is 10. */
/*
#define NX_IP_FAST_TIMER_RATE               10
*/

/* Define the amount of time to sleep in nx_ip_(interface_)status_check.
   The default value is 1. */
/*
#define NX_IP_STATUS_CHECK_WAIT_TIME          1
*/

/* Defined, IP packet filter is enabled.  */
/*
#define NX_ENABLE_IP_PACKET_FILTER
*/

/* Defined, disables both IPv4 and IPv6 fragmentation and reassembly logic. */
/*
#define NX_DISABLE_FRAGMENTATION
*/

/* Defined, this option process IP fragmentation immediately.  */
/*
#define NX_FRAGMENT_IMMEDIATE_ASSEMBLY
*/

/*****************************************************************************/
/******************** Configuration options for IPV 4 ************************/
/*****************************************************************************/

/* Defined, disables IPv4 functionality. This option can be used to build NetX
   Duo to support IPv6 only. By default this option is not defined. */
/*
#define NX_DISABLE_IPV4
*/

/*****************************************************************************/
/******************** Configuration options for IPV 6 ************************/
/*****************************************************************************/

/* Disables IPv6 functionality when the NetX Duo library is built.
   For applications that do not need IPv6, this avoids pulling in code and
   additional storage space needed to support IPv6. */
/*
#define NX_DISABLE_IPV6
*/

/* Defined, enable IPV6 features. */
/*
#define FEATURE_NX_IPV6
*/

/* Specifies the number of entries in the IPv6 address pool. During interface
   configuration, NetX Duo uses IPv6 entries from the pool. It is defaulted to
   (NX_MAX_PHYSICAL_INTERFACES * 3) to allow each interface to have at least
   one link local address and two global addresses. Note that all interfaces
   share the IPv6 address pool. */
/*
#ifdef NX_MAX_PHYSICAL_INTERFACES
#define NX_MAX_IPV6_ADDRESSES           (NX_MAX_PHYSICAL_INTERFACES * 3)
#endif
*/

/* If defined, application is able to control whether or not to perform IPv6
   stateless address auto-configuration with nxd_ipv6_stateless_address_autoconfig_enable()
   or nxd_ipv6_stateless_address_autoconfig_disable() service. If defined, the
   system starts with IPv6 stateless address auto-configuration enabled.
   This feature is disabled by default. */
/*
#define NX_IPV6_STATELESS_AUTOCONFIG_CONTROL
*/

/* If enabled, application is able to install a callback function to get
   notified when an interface IPv6 address is changed. By default this
   feature is disabled. */
/*
#define NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
*/

/* This define enables simple IPv6 multicast group join/leave function.
   By default the IPv6 multicast join/leave function is not enabled. */
/*
#define NX_ENABLE_IPV6_MULTICAST
*/

/* Defined, Minimum Path MTU Discovery feature is enabled.  */
/*
#define NX_ENABLE_IPV6_PATH_MTU_DISCOVERY
*/

/* Define wait interval in seconds to reset the path MTU for a destination
   table entry after decreasing it in response to a packet too big error message.
   RFC 1981 Section 5.4 states the minimum time to wait is 5 minutes and
   recommends 10 minutes. */
/*
#define NX_PATH_MTU_INCREASE_WAIT_INTERVAL          600
*/

/* Specifies the number of entries in the IPv6 destination table. This stores
   information about next hop addresses for IPv6 addresses. Defined in nx_api.h,
   the default value is 8. */
/*
#define NX_IPV6_DESTINATION_TABLE_SIZE          8
*/

/* Specifies the size of the prefix table. Prefix information is obtained from
   router advertisements and is part of the IPv6 address configuration. Defined
   in nx_api.h, the default value is 8. */
/*
#define NX_IPV6_PREFIX_LIST_TABLE_SIZE      8
*/

/* Specifies the number of entries in the IPv6 routing table. At least one
   entry is needed for the default router. Defined in nx_api.h, the default
   value is 8. */
/*
#define NX_IPV6_DEFAULT_ROUTER_TABLE_SIZE       8
*/

/*****************************************************************************/
/******************** Configuration options for ICMP *************************/
/*****************************************************************************/

/* Defined, NetX Duo does not send ICMPv4 Error Messages in response to error
   conditions such as improperly formatted IPv4 header. By default this option
   is not defined. */
/*
#define NX_DISABLE_ICMPV4_ERROR_MESSAGE
*/

/* Defined, this option disables checksum logic on received ICMPv4 or ICMPv6 packets.
   Note that if NX_DISABLE_ICMP_RX_CHECKSUM is defined, NX_DISABLE_ICMPV4_RX_CHECKSUM
   and NX_DISABLE_ICMPV6_RX_CHECKSUM are automatically defined. */
/*
#define NX_DISABLE_ICMP_RX_CHECKSUM
*/

/* Defined, this option disables checksum logic on received ICMPv4 packets.
   Note that if NX_DISABLE_ICMP_RX_CHECKSUM is defined, this option is
   automatically defined. By default this option is not defined. */
/*
#define NX_DISABLE_ICMPV4_RX_CHECKSUM
*/

/* Defined, this option disables checksum logic on received ICMPv6 packets.
   Note that if NX_DISABLE_ICMP_RX_CHECKSUM is defined, this option is
   automatically defined. By default this option is not defined. */
/*
#define NX_DISABLE_ICMPV6_RX_CHECKSUM
*/

/* Defined, this option disables checksum logic on transmitted ICMPv4 or ICMPv6
   packets. Note that if NX_DISABLE_ICMP_TX_CHECKSUM is defined,
   NX_DISABLE_ICMPV4_TX_CHECKSUM and NX_DISABLE_ICMPV6_TX_CHECKSUM are
   automatically defined. */
/*
#define NX_DISABLE_ICMP_TX_CHECKSUM
*/

/* Defined, this option disables checksum logic on transmitted ICMPv4 packets.
   Note that if NX_DISABLE_ICMP_TX_CHECKSUM is defined, this option is
   automatically defined. By default this option is not defined.*/
/*
#define NX_DISABLE_ICMPV4_TX_CHECKSUM
*/

/* Defined, this option disables checksum logic on transmitted ICMPv6 packets.
   Note that if NX_DISABLE_ICMP_TX_CHECKSUM is defined, this option is
   automatically defined. By default this option is not defined.*/
/*
#define NX_DISABLE_ICMPV6_TX_CHECKSUM
*/

/* Defined, ICMP information gathering is disabled. */
/*
#define NX_DISABLE_ICMP_INFO
*/

/* Defined, the destination address of ICMP packet is checked. The default
   is disabled. An ICMP Echo Request destined to an IP broadcast or IP
   multicast address will be silently discarded. */
/*
#define NX_ENABLE_ICMP_ADDRESS_CHECK
*/

/* Defined, disables NetX Duo from sending an ICMPv6 error message in response
   to a problem packet (e.g., improperly formatted header or packet header
   type is deprecated) received from another host. */
/*
#define NX_DISABLE_ICMPV6_ERROR_MESSAGE
*/

/* Defined, disables ICMPv6 redirect packet processing. NetX Duo by default
   processes redirect messages and updates the destination table with next hop
   IP address information. */
/*
#define NX_DISABLE_ICMPV6_REDIRECT_PROCESS
*/

/* Defined, disables NetX Duo from processing information received in IPv6
   router advertisement packets. */
/*
#define NX_DISABLE_ICMPV6_ROUTER_ADVERTISEMENT_PROCESS
*/

/* Defined, disables NetX Duo from sending IPv6 router solicitation messages
   at regular intervals to the router. */
/*
#define NX_DISABLE_ICMPV6_ROUTER_SOLICITATION
*/

/* Define the max number of router solicitations a host sends until a router
   response is received. If no response is received, the host concludes no
   router is present. The default value is 3. */
/*
#define NX_ICMPV6_MAX_RTR_SOLICITATIONS         3
*/

/* Specifies the interval between two router solicitation messages.
   The default value is 4. */
/*
#define NX_ICMPV6_RTR_SOLICITATION_INTERVAL     4
*/

/* Specifies the maximum delay for the initial router solicitation in seconds. */
/*
#define NX_ICMPV6_RTR_SOLICITATION_DELAY        1
*/

/*****************************************************************************/
/********************* Configuration options for TCP *************************/
/*****************************************************************************/

/* Specifies how the number of system ticks (NX_IP_PERIODIC_RATE) is divided
   to calculate the timer rate for the TCP delayed ACK processing.
   The default value is 5, which represents 200ms, and is defined in nx_tcp.h.
   The application can override the default by defining the value before
   nx_api.h is included. */
/*
#define NX_TCP_ACK_TIMER_RATE             5
*/

/* Specifies how the number of NetX Duo internal ticks (NX_IP_PERIODIC_RATE)
   is divided to calculate the fast TCP timer rate. The fast TCP timer is used
   to drive the various TCP timers, including the delayed ACK timer.
   The default value is 10, which represents 100ms assuming the ThreadX timer
   is running at 10ms. This value is defined in nx_tcp.h. The application can
   override the default by defining the value before nx_api.h is included. */
/*
#define NX_TCP_FAST_TIMER_RATE            10
*/

/* Specifies how the number of system ticks (NX_IP_PERIODIC_RATE) is divided to
   calculate the timer rate for the TCP transmit retry processing.
   The default value is 1, which represents 1 second, and is defined in nx_tcp.h.
   The application can override the default by defining the value before nx_api.h
   is included. */
/*
#define NX_TCP_TRANSMIT_TIMER_RATE        1
*/

/* Specifies the number of seconds of inactivity before the keepalive timer
   activates. The default value is 7200, which represents 2 hours, and is
   defined in nx_tcp.h. The application can override the default by defining
   the value before nx_api.h is included. */
/*
#define NX_TCP_KEEPALIVE_INITIAL          7200
*/

/* Specifies the number of seconds between retries of the keepalive timer
   assuming the other side of the connection is not responding. The default
   value is 75, which represents 75 seconds between retries, and is defined
   in nx_tcp.h. The application can override the default by defining the value
   before nx_api.h is included. */
/*
#define NX_TCP_KEEPALIVE_RETRY            75
*/

/* Symbol that defines the maximum number of out-of-order TCP packets can be
   kept in the TCP socket receive queue. This symbol can be used to limit the
   number of packets queued in the TCP receive socket, preventing the packet
   pool from being starved. By default this symbol is not defined, thus there
   is no limit on the number of out of order packets being queued in the TCP
   socket. */
#define NX_TCP_MAX_OUT_OF_ORDER_PACKETS       8

/* Defined, enables the optional TCP keepalive timer. The default settings is
   not enabled. */
/*
#define NX_ENABLE_TCP_KEEPALIVE
*/

/* Defined, enables the optional TCP immediate ACK response processing.
   Defining this symbol is equivalent to defining NX_TCP_ACK_EVERY_N_PACKETS
   to be 1. */
/*
#define NX_TCP_IMMEDIATE_ACK
*/

/* Specifies the number of TCP packets to receive before sending an ACK.
   Note if NX_TCP_IMMEDIATE_ACK is enabled but NX_TCP_ACK_EVERY_N_PACKETS is
   not, this value is automatically set to 1 for backward compatibility. */
/*
#define NX_TCP_ACK_EVERY_N_PACKETS        2
*/

/* Automatically define NX_TCP_ACK_EVERY_N_PACKETS to 1 if NX_TCP_IMMEDIATE_ACK is defined.
   This is needed for backward compatibility. */
#if (defined(NX_TCP_IMMEDIATE_ACK) && !defined(NX_TCP_ACK_EVERY_N_PACKETS))
#define NX_TCP_ACK_EVERY_N_PACKETS 1
#endif

/* Specifies how many data transmit retries are allowed before the connection
   is deemed broken. The default value is 10, which represents 10 retries, and
   is defined in nx_tcp.h. The application can override the default by defining
   the value before nx_api.h is included. */
/*
#define NX_TCP_MAXIMUM_RETRIES            10
*/

/* Specifies the maximum depth of the TCP transmit queue before TCP send
   requests are suspended or rejected. The default value is 20, which means
   that a maximum of 20 packets can be in the transmit queue at any given time.
   Note packets stay in the transmit queue until an ACK that covers some or all
   of the packet data is received from the other side of the connection.
   This constant is defined in nx_tcp.h. The application can override the
   default by defining the value before nx_api.h is included. */
/*
#define NX_TCP_MAXIMUM_TX_QUEUE           20
*/

/* Specifies how the retransmit timeout period changes between retries.
   If this value is 0, the initial retransmit timeout is the same as subsequent
   retransmit timeouts. If this value is 1, each successive retransmit is twice
   as long. If this value is 2, each subsequent retransmit timeout is four
   times as long. The default value is 0 and is defined in nx_tcp.h.
   The application can override the default by defining the value before nx_api.h
   is included. */
/*
#define NX_TCP_RETRY_SHIFT                0x0
*/

/* Specifies how many keepalive retries are allowed before the connection is
   deemed broken. The default value is 10, which represents 10 retries, and is
   defined in nx_tcp.h. The application can override the default by defining
   the value before nx_api.h is included. */
/*
#define NX_TCP_KEEPALIVE_RETRIES          10
*/

/* Enables the window scaling option for TCP applications. If defined, window
   scaling option is negotiated during TCP connection phase, and the
   application is able to specify a window size larger than 64K. The default
   setting is not enabled (not defined). */
/*
#define NX_ENABLE_TCP_WINDOW_SCALING
*/

/* Defined, disables the reset processing during disconnect when the timeout
   value supplied is specified as NX_NO_WAIT. */
/*
#define NX_DISABLE_RESET_DISCONNECT
*/

/* Defined, enables the verification of minimum peer MSS before accepting a TCP
   connection. To use this feature, the symbol NX_ENABLE_TCP_MSS_MINIMUM must
   be defined. By default, this option is not enabled. */
/*
#define NX_ENABLE_TCP_MSS_CHECK
*/

/* Defined, allows the application to install a callback function that is
   invoked when the TCP transmit queue depth is no longer at maximum value.
   This callback serves as an indication that the TCP socket is ready to
   transmit more data. By default this option is not enabled. */
/*
#define NX_ENABLE_TCP_QUEUE_DEPTH_UPDATE_NOTIFY
*/

/* Defined, disables checksum logic on received TCP packets. This is only
   useful in situations in which the link-layer has reliable checksum or CRC
   processing, or the interface driver is able to verify the TCP checksum in
   hardware, and the application does not use IPsec. */
/*
#define NX_DISABLE_TCP_RX_CHECKSUM
*/

/* Defined, disables checksum logic for sending TCP packets. This is only
   useful in situations in which the receiving network node has received TCP
   checksum logic disabled or the underlying network driver is capable of
   generating the TCP checksum, and the application does not use IPsec. */
/*
#define NX_DISABLE_TCP_TX_CHECKSUM
*/

/* Defined, disables TCP information gathering. */
/*
#define NX_DISABLE_TCP_INFO
*/

/* Number of seconds for maximum segment lifetime, the default is 2 minutes
  (120s) */
/*
#define NX_TCP_MAXIMUM_SEGMENT_LIFETIME       120
*/

/* Specifies the maximum number of server listen requests. The default value is
   10 and is defined in nx_api.h. The application can override the default by
   defining the value before nx_api.h is included. */
/*
#define NX_MAX_LISTEN_REQUESTS            10
*/

/*****************************************************************************/
/********************* Configuration options for UDP *************************/
/*****************************************************************************/

/* Defined, disables the UDP checksum computation on incoming UDP packets.
   This is useful if the network interface driver is able to verify UDP header
   checksum in hardware, and the application does not enable IPsec or IP
   fragmentation logic. */
/*
#define NX_DISABLE_UDP_RX_CHECKSUM
*/

/* Defined, disables the UDP checksum computation on outgoing UDP packets.
   This is useful if the network interface driver is able to compute UDP header
   checksum and insert the value in the IP head before transmitting the data,
   and the application does not enable IPsec or IP fragmentation logic. */
/*
#define NX_DISABLE_UDP_TX_CHECKSUM
*/

/* Defined, disables UDP information gathering. */
/*
#define NX_DISABLE_UDP_INFO
*/

/*****************************************************************************/
/********************* Configuration options for IGMP ************************/
/*****************************************************************************/

/* Defined, disables IGMP information gathering. */
/*
#define NX_DISABLE_IGMP_INFO
*/

/* Defined, disables IGMPv2 support, and NetX Duo supports IGMPv1 only.
   By default this option is not set and is defined in nx_api.h. */
/*
#define NX_DISABLE_IGMPV2
*/

/* Specifies the maximum number of multicast groups that can be joined.
   The default value is 7 and is defined in nx_api.h. The application can
   override the default by defining the value before nx_api.h is included. */
/*
#define NX_MAX_MULTICAST_GROUPS           7
*/

/*****************************************************************************/
/******************* Configuration options for ARP/RARP **********************/
/*****************************************************************************/

/* Defined, allows NetX Duo to defend its IP address by sending an ARP
   response. */
/*
#define NX_ARP_DEFEND_BY_REPLY
*/

/* Specifies the number of seconds ARP entries remain valid. The default value
   of zero disables expiration or aging of ARP entries and is defined in
   nx_api.h. The application can override the default by defining the value
   before nx_api.h is included. */
/*
#define NX_ARP_EXPIRATION_RATE            0
*/

/* Specifies the number of seconds between ARP retries. The default value is 10,
   which represents 10 seconds, and is defined in nx_api.h. The application can
   override the default by defining the value before nx_api.h is included. */
/*
#define NX_ARP_UPDATE_RATE                10
*/

/* Specifies the maximum number of ARP retries made without an ARP response.
   The default value is 18 and is defined in nx_api.h. The application can
   override the default by defining the value before nx_api.h is included. */
/*
#define NX_ARP_MAXIMUM_RETRIES            18
*/

/* Specifies the maximum number of packets that can be queued while waiting for
   an ARP response. The default value is 4 and is defined in nx_api.h. */
/*
#define NX_ARP_MAX_QUEUE_DEPTH            4
*/

/* Defines the interval, in seconds, the ARP module sends out the next defend
   packet in response to an incoming ARP message that indicates an address in
   conflict. */
/*
#define NX_ARP_DEFEND_INTERVAL          10
*/

/* Defined, disables entering ARP request information in the ARP cache. */
/*
#define NX_DISABLE_ARP_AUTO_ENTRY
*/

/* Defined, allows ARP to invoke a callback notify function on detecting the
   MAC address is updated. */
/*
#define NX_ENABLE_ARP_MAC_CHANGE_NOTIFICATION
*/

/* Defined, disables ARP information gathering. */
/*
#define NX_DISABLE_ARP_INFO
*/

/* Defined, disables RARP information gathering. */
/*
#define NX_DISABLE_RARP_INFO
*/

/*****************************************************************************/
/********************* Configuration options for DHCP ************************/
/*****************************************************************************/

/* Type of service required for the DHCP UDP requests. By default, this value
   is defined as NX_IP_NORMAL to indicate normal IP packet service. */
/*
#define NX_DHCP_TYPE_OF_SERVICE                 NX_IP_NORMAL
*/

/* Fragment enable for DHCP UDP requests. By default, this value is
   NX_DONT_FRAGMENT to disable DHCP UDP fragmenting. */
/*
#define NX_DHCP_FRAGMENT_OPTION                 NX_DONT_FRAGMENT
*/

/* Specifies the number of routers this packet can pass before it is discarded.
   The default value is set to 0x80. */
/*
#define NX_DHCP_TIME_TO_LIVE                    0x80
*/

/* Specifies the number of maximum depth of receive queue. The default value
   is set to 4. */
/*
#define NX_DHCP_QUEUE_DEPTH                 4
*/

/*****************************************************************************/
/****************** Configuration options for DHCP Client ********************/
/*****************************************************************************/

/* Defined, this option enables the BOOTP protocol instead of DHCP.
   By default this option is disabled. */
/*
#define NX_DHCP_ENABLE_BOOTP
*/

/* If defined, this enables the DHCP Client to save its current DHCP Client
   license 'state' including time remaining on the lease, and restore this
   state between DHCP Client application reboots.
   The default value is disabled. */
/*
#define NX_DHCP_CLIENT_RESTORE_STATE
*/

/* If set, the DHCP Client will not create its own packet pool. The host
   application must use the nx_dhcp_packet_pool_set service to set the DHCP
   Client packet pool. The default value is disabled. */
/*
#define NX_DHCP_CLIENT_USER_CREATE_PACKET_POOL
*/

/* Defined, this enables the DHCP Client to send an ARP probe after IP
   address assignment to verify the assigned DHCP address is not owned by
   another host. By default, this option is disabled. */
/*
#define NX_DHCP_CLIENT_SEND_ARP_PROBE
*/

/* Defines the number of ARP probes sent for determining if the IP address
   assigned by the DHCP server is already in use. The value is defaulted to
   3 probes. */
/*
#define NX_DHCP_ARP_PROBE_NUM               3
*/

/* Specifies the maximum number of interface records to save to the DHCP Client
   instance. A DHCP Client interface record is a record of the DHCP Client
   running on a specific interface. The default value is set as physical
   interfaces count (NX_MAX_PHYSICAL_INTERFACES). */
/*
#define NX_DHCP_CLIENT_MAX_RECORDS          (NX_MAX_PHYSICAL_INTERFACES)
*/

/* Defined, this enables the DHCP Client to send maximum DHCP message size
   option. By default, this option is disabled. */
/*
#define NX_DHCP_CLIENT_SEND_MAX_DHCP_MESSAGE_OPTION
*/

/* Defined, this enables the DHCP Client to check the input host name in the
   nx_dhcp_create call for invalid characters or length. By default, this
   option is disabled. */
/*
#define NX_DHCP_CLIENT_ENABLE_HOST_NAME_CHECK
*/

/* Priority of the DHCP thread. By default, this value specifies that the DHCP
   thread runs at priority 3. */
/*
#define NX_DHCP_THREAD_PRIORITY             3
*/

/* Size of the DHCP thread stack. By default, the size is 4096 bytes. */
/*
#define NX_DHCP_THREAD_STACK_SIZE           (4096)
*/

/* Size of DHCP options buffer. By default, this value is 312 bytes. */
/*
#define NX_DHCP_OPTIONS_BUFFER_SIZE         312
*/

/* Specifies the size in bytes of the DHCP Client packet payload.
   The default value is NX_DHCP_MINIMUM_IP_DATAGRAM + physical header size.
   The physical header size in a wireline network is usually the Ethernet frame
   size. */
/*
#define NX_DHCP_PACKET_PAYLOAD              (NX_DHCP_MINIMUM_IP_DATAGRAM + NX_PHYSICAL_HEADER)
*/

/* Specifies the size of the DHCP Client packet pool. The default value is
  (5 *NX_DHCP_PACKET_PAYLOAD) which will provide four packets plus room for
  internal packet pool overhead. */
/*
#define NX_DHCP_PACKET_POOL_SIZE            (5 * NX_DHCP_PACKET_PAYLOAD)
*/

/*****************************************************************************/
/****************** Configuration options for DHCP Server ********************/
/*****************************************************************************/

/* This option specifies the priority of the DHCP Server thread. By default,
   this value specifies that the DHCP thread runs at priority 2. */
/*
#define NX_DHCP_SERVER_THREAD_PRIORITY          2
*/

/* This is the subnet mask the DHCP Client should be configured with.
   The default value is set to 0xFFFFFF00. */
/*
#define NX_DHCP_SUBNET_MASK                     0xFFFFFF00UL
*/

/* This is timeout period in timer ticks for the DHCP Server fast timer to check
   on session time remaining and handle sessions that have timed out. */
/*
#define NX_DHCP_FAST_PERIODIC_TIME_INTERVAL     10
*/

/* This is timeout period in timer ticks for the DHCP Server slow timer to
   check on IP address lease time remaining and handle lease that have
   timed out. */
/*
#define NX_DHCP_SLOW_PERIODIC_TIME_INTERVAL   1000
*/

/* This is IP Address lease time in seconds assigned to the DHCP Client, and
   the basis for computing the renewal and rebind times also assigned to the
   Client.
   The default value is set to (10 * NX_DHCP_SLOW_PERIODIC_TIME_INTERVAL). */
/*
#define  NX_DHCP_DEFAULT_LEASE_TIME             (10 * NX_DHCP_SLOW_PERIODIC_TIME_INTERVAL)
*/

/* This is size of the DHCP Server array for holding available IP addresses for
   assigning to the Client. The default value is 20. */
/*
#define NX_DHCP_IP_ADDRESS_MAX_LIST_SIZE        20
*/

/* This is size of the DHCP Server array for holding Client records.
   The default value is 50. */
/*
#define NX_DHCP_CLIENT_RECORD_TABLE_SIZE        50
*/

/* This is size of the array in the DHCP Client instance for holding the all
   the requested options in the parameter request list in the current session.
   The default value is 12. */
/*
#define NX_DHCP_CLIENT_OPTIONS_MAX              12
*/

/* This is size of the buffer for holding the Server host name.
   The default value is 32. */
/*
#define NX_DHCP_SERVER_HOSTNAME_MAX             32
*/

/* This is size of the buffer for holding the Client host name in the current
   DHCP Server Client session. The default value is 32. */
/*
#define NX_DHCP_CLIENT_HOSTNAME_MAX             32
*/

/* Define the DHCP server thread stack size. */
/*
#define NX_DHCP_SERVER_THREAD_STACK_SIZE           1024
*/

/* Define name of DHCP server. */
/*
#define NX_DHCP_SERVER_NAME       "NetX DHCP Server"
*/

/*****************************************************************************/
/****************** Configuration options for DHCP IPV6 **********************/
/*****************************************************************************/

/* Time interval in seconds at which the session timer updates the length of
   time the Client has been in session communicating with the Server.
   By default, this value is 1. */
/*
#define NX_DHCPV6_SESSION_TIMER_INTERVAL        1
*/

/* Time out in seconds for allocating a packet from the Client packet pool.
   The default value is 3 seconds. */
/*
#define NX_DHCPV6_PACKET_TIME_OUT               (3 * NX_DHCPV6_TICKS_PER_SECOND)
*/

/* This defines the type of service for UDP packet transmission from the DHCPv6
   Client socket. The default value is NX_IP_NORMAL. */
/*
#define NX_DHCPV6_TYPE_OF_SERVICE               NX_IP_NORMAL
*/

/* The number of times a Client packet is forwarded by a network router before
   the packet is discarded. The default value is 0x80. */
/*
#define NX_DHCPV6_TIME_TO_LIVE                  0x80
*/

/* Specifies the number of packets to keep in the Client UDP socket receive
   queue before NetX Duo discards packets. The default value is 5. */
/*
#define NX_DHCPV6_QUEUE_DEPTH                   5
*/

/*****************************************************************************/
/*************** Configuration options for DHCP Client IPV6 ******************/
/*****************************************************************************/

/* Priority of the Client thread. By default, this value specifies that the
   Client thread runs at priority 2. */
/*
#define NX_DHCPV6_THREAD_PRIORITY               2
*/

/* Time out option for obtaining an exclusive lock on a DHCPv6 Client mutex.
   The default value is TX_WAIT_FOREVER. */
/*
#define NX_DHCPV6_MUTEX_WAIT                    TX_WAIT_FOREVER
*/

/* Time interval in seconds at which the IP lifetime timer updates the length
   of time the current IP address has been assigned to the Client.
   By default, this value is 1. */
/*
#define NX_DHCPV6_IP_LIFETIME_TIMER_INTERVAL    1
*/

/* The maximum number of IA addresses that can be added to the Client record.
   The default value is 1. */
/*
#define NX_DHCPV6_MAX_IA_ADDRESS                1
*/

/* Number of DNS servers to store to the client record.
   The default value is 2. */
/*
#define NX_DHCPV6_NUM_DNS_SERVERS               2
*/

/* Number of time servers to store to the client record.
   The default value is 1. */
/*
#define NX_DHCPV6_NUM_TIME_SERVERS              1
*/

/* Size of the buffer in the Client record to hold the client's network domain
   name. The default value is 32. */
/*
#define NX_DHCPV6_DOMAIN_NAME_BUFFER_SIZE       32
*/

/* Size of the buffer in the Client record to hold the Client's time zone.
   The default value is 16. */
/*
#define NX_DHCPV6_TIME_ZONE_BUFFER_SIZE         16
*/

/* Size of the buffer in the Client record to hold the option status message
   in a Server reply. The default value is 100 bytes. */
/*
#define NX_DHCPV6_MAX_MESSAGE_SIZE              100
*/

/*****************************************************************************/
/*************** Configuration options for DHCP Server IPV6 ******************/
/*****************************************************************************/

/* This defines the size of the DHCPv6 thread stack. By default, the size is
   4096 bytes which is more than enough for most NetX Duo applications. */
/*
#define NX_DHCPV6_SERVER_THREAD_STACK_SIZE      4096
*/

/* This defines the DHCPv6 Server thread priority. This should be lower than the
   DHCPv6 Server's IP thread task priority. The default value is 2. */
/*
#define NX_DHCPV6_SERVER_THREAD_PRIORITY        2
*/

/* Timer interval in seconds when the lease timer entry function is called by
   the ThreadX scheduler. The entry function sets a flag for the DHCPv6 Server
   to increment all Clients' accrued time on their lease by the timer interval.
   By default, this value is 60. */
/*
#define NX_DHCPV6_IP_LEASE_TIMER_INTERVAL       (60)
*/

/* Create a Server DUID with a vendor assigned ID. Note the DUID type must be
   set NX_DHCPV6_DUID_TYPE_VENDOR_ASSIGNED. */
/*
#define NX_DHCPV6_SERVER_DUID_VENDOR_ASSIGNED_ID    "abcdeffghijklmnopqrstuvwxyz"
*/

/* Sets the upper limit on the Vendor assigned ID. The default value is 48. */
/*
#define NX_DHCPV6_SERVER_DUID_VENDOR_ASSIGNED_LENGTH    48
*/

/* Sets the enterprise type of the DUID to private vendor type. */
/*
#define NX_DHCPV6_SERVER_DUID_VENDOR_PRIVATE_ID     0x12345678
*/

/* This defines the preference option value between 0 and 255, where the higher
   the value the higher the preference, in the DHCPv6 option of the same name.
   This tells the Client what preference to place on this Server's offer where
   multiple DHCPv6 Servers are available to assign IP addresses.
   A value of 255 instructs the Client to choose this server. A value of zero
   indicates the Client is free to choose. The default value is zero. */
/*
#define NX_DHCPV6_PREFERENCE_VALUE              0
*/

/* This defines the maximum number of option requests in a Client request that
   can be saved to a Client record. The default value is 6. */
/*
#define NX_DHCPV6_MAX_OPTION_REQUEST_OPTIONS    6
*/

/* The time in seconds assigned by the Server on a Client address lease for
   when the Client should begin renewing its IP address. The default value is
   2000 seconds. */
/*
#define  NX_DHCPV6_DEFAULT_T1_TIME              (2000)
*/

/* The time in seconds assigned by the Server on a Client address lease for
   when the Client should begin rebinding its IP address assuming its attempt
   to renew failed. The default value is 3000 seconds. */
/*
#define  NX_DHCPV6_DEFAULT_T2_TIME              (3000)
*/

/* This defines the time in seconds assigned bythe Server for when an assigned
   Client IP address lease is deprecated.
   The default value is 2 NX_DHCPV6_DEFAULT_T1_TIME. */
/*
#define  NX_DHCPV6_DEFAULT_PREFERRED_TIME       (2 * NX_DHCPV6_DEFAULT_T1_TIME)
*/

/* This defines the time expiration in seconds assigned by the Server on an
   assigned Client IP address lease. After this time expires, the Client IP
   address is invalid.
   The default value is 2 NX_DHCPV6_DEFAULT_PREFERRED_TIME. */
/*
#define NX_DHCPV6_DEFAULT_VALID_TIME          (2 * NX_DHCPV6_DEFAULT_PREFERRED_TIME)
*/

/* Defines the maximum size of the Server message in status option message field.
   The default value is 100 bytes. */
/*
#define NX_DHCPV6_STATUS_MESSAGE_MAX            100
*/

/* Defines the size of the Server's IP lease table (e.g. the max number of IPv6
   address available to lease that can be stored).
   By default, this value is 100. */
/*
#define NX_DHCPV6_MAX_LEASES                    100
*/

/* Defines the size of the Server's Client record table (e.g. max number of
   Clients that can be stored). This value should be less than or equal to the
   value NX_DHCPV6_MAX_LEASES.By default, this value is 120. */
/*
#define NX_DHCPV6_MAX_CLIENTS                   120
*/

/* This defines the packet payload of the Server packet pool packets.
   The default value is 500 bytes. */
/*
#define NX_DHCPV6_PACKET_SIZE                   500
*/

/* Defines the Server packet pool size for packets the Server will allocate to
   send DHCPv6 messages out.
   The default value is (10 * NX_DHCPV6_PACKET_SIZE) */
/*
#define NX_DHCPV6_PACKET_POOL_SIZE              (10 * NX_DHCPV6_PACKET_SIZE)
*/

/* This defines the Server socket fragmentation option.
   The default value is NX_DONT_FRAGMENT. */
/*
#define NX_DHCPV6_FRAGMENT_OPTION               NX_DONT_FRAGMENT
*/

/* Define the session time out in seconds. This is the timer for how long the
   server has not received a client response.
   The default value is set to 20. */
/*
#define NX_DHCPV6_SESSION_TIMEOUT               (20)
*/

/* This defines the status option message for : NX_DHCPV6_STATUS_MESSAGE_SUCCESS.
   The default value is "IA OPTION GRANTED". */
/*
#define NX_DHCPV6_STATUS_MESSAGE_SUCCESS        "IA OPTION GRANTED"
*/

/* This defines the status option message for : NX_DHCPV6_STATUS_MESSAGE_NO_ADDRS_AVAILABLE.
   The default value is "IA OPTION NOT GRANTED-NO ADDRESSES AVAILABLE". */
/*
#define NX_DHCPV6_STATUS_MESSAGE_NO_ADDRS_AVAILABLE     "IA OPTION NOT GRANTED-NO ADDRESSES AVAILABLE"
*/

/* This defines the status option message for : NX_DHCPV6_STATUS_MESSAGE_NO_BINDING.
   The default value is "IA OPTION NOT GRANTED-INVALID CLIENT REQUEST". */
/*
#define NX_DHCPV6_STATUS_MESSAGE_NO_BINDING             "IA OPTION NOT GRANTED-INVALID CLIENT REQUEST"
*/

/* This defines the status option message for : NX_DHCPV6_STATUS_MESSAGE_NOT_ON_LINK.
   The default value is "IA OPTION NOT GRANTED-INVALID CLIENT REQUEST". */
/*
#define NX_DHCPV6_STATUS_MESSAGE_NOT_ON_LINK            "IA OPTION NOT GRANTED-CLIENT NOT ON LINK"
*/

/* This defines the status option message for : NX_DHCPV6_STATUS_MESSAGE_USE_MULTICAST.
   The default value is "IA OPTION NOT GRANTED-CLIENT MUST USE MULTICAST". */
/*
#define NX_DHCPV6_STATUS_MESSAGE_USE_MULTICAST          "IA OPTION NOT GRANTED-CLIENT MUST USE MULTICAST"
*/

/*****************************************************************************/
/********************* Configuration options for DNS *************************/
/*****************************************************************************/

/* Type of service required for the DNS UDP requests. By default, this value
   is defined as NX_IP_NORMAL for normal IP packet service. */
/*
#define NX_DNS_TYPE_OF_SERVICE              NX_IP_NORMAL
*/

/* Specifies the maximum number of routers a packet can pass before it is
   discarded. The default value is 0x80. */
/*
#define NX_DNS_TIME_TO_LIVE                     0x80
*/

/* Sets the socket property to allow or disallow fragmentation of outgoing
   packets. The default value is NX_DONT_FRAGMENT. */
/*
#define NX_DNS_FRAGMENT_OPTION              NX_DONT_FRAGMENT
*/

/* Sets the maximum number of packets to store on the socket receive queue.
   The default value is 5. */
/*
#define NX_DNS_QUEUE_DEPTH                      5
*/

/* Specifies the maximum number of DNS Servers in the Client server list.
   The default value is 5. */
/*
#define NX_DNS_MAX_SERVERS                      5
*/

/* Specifies the maximum number of DNS Servers in the Client server list.
   The default value is 5. */
/*
#define NX_DNS_MESSAGE_MAX                      512
*/

/* If not defined, the size of the Client packet payload which includes the
   Ethernet, IP (or IPv6), and UDP headers plus the maximum DNS message size
   specified by NX_DNS_MESSAGE_MAX. Regardless if defined, the packet payload
   is the 4-byte aligned and stored in NX_DNS_PACKET_PAYLOAD.
   The default value is (NX_UDP_PACKET + NX_DNS_MESSAGE_MAX). */
/*
#define NX_DNS_PACKET_PAYLOAD_UNALIGNED         (NX_UDP_PACKET + NX_DNS_MESSAGE_MAX)
*/

/* Size of the Client packet pool for sending DNS queries if
   NX_DNS_CLIENT_USER_CREATE_PACKET_POOL is not defined. The default value is
   large enough for 16 packets of payload size defined by NX_DNS_PACKET_PAYLOAD,
   and is 4-byte aligned.
   The default value is (16 * (NX_DNS_PACKET_PAYLOAD + sizeof(NX_PACKET))). */
/*
#define NX_DNS_PACKET_POOL_SIZE                 (16 * (NX_DNS_PACKET_PAYLOAD + sizeof(NX_PACKET)))
*/

/* The maximum number of times the DNS Client will query the current DNS server
   before trying another server or aborting the DNS query.
   The default value is 3. */
/*
#define NX_DNS_MAX_RETRIES                      3
*/

/* If defined and the Client IPv4 gateway address is non zero, the DNS Client
   sets the IPv4 gateway as the Client's primary DNS server. The default value
   is disabled. */
/*
#define NX_DNS_IP_GATEWAY_AND_DNS_SERVER
*/

/* This enables the DNS Client to let the application create and set the DNS
   Client packet pool. By default this option is disabled, and the DNS Client
   creates its own packet pool in nx_dns_create. */
/*
#define NX_DNS_CLIENT_USER_CREATE_PACKET_POOL
*/

/* This enables the DNS Client to clear old DNS messages off the receive queue
   before sending a new query. Removing these packets from previous DNS queries
   prevents the DNS Client socket queue from overflowing and dropping valid
   packets. */
/*
#define NX_DNS_CLIENT_CLEAR_QUEUE
*/

/* This enables the DNS Client to query on additional DNS record types in
   (e.g. CNAME, NS, MX, SOA, SRV and TXT). */
/*
#define NX_DNS_ENABLE_EXTENDED_RR_TYPES
*/

/* This enables the DNS Client to store the answer records into DNS cache. */
/*
#define NX_DNS_CACHE_ENABLE
*/

/*****************************************************************************/
/********************* Configuration options for MQTT ************************/
/*****************************************************************************/

/* Defined, MQTT Client is built with TLS support. Defining this symbol
   requires NetX Secure TLS module to be installed. NX_SECURE_ENABLE is not
   enabled by default. */
/*
#define NX_SECURE_ENABLE
*/

/* Defined, application must use TLS to connect to MQTT broker. This feature
   requires NX_SECURE_ENABLE defined. By default, this symbol is not
   defined. */
/*
#define NXD_MQTT_REQUIRE_TLS
*/

/* Defines the time out in the TCP socket disconnect call when disconnecting
   from the MQTT server in timer ticks. The default value is NX_WAIT_FOREVER. */
/*
#define NXD_MQTT_SOCKET_TIMEOUT                 NX_WAIT_FOREVER
*/

/* Defined, enable MQTT over cloud option. */
/*
#define NXD_MQTT_CLOUD_ENABLE
*/

/* Define memcpy function used internal. */
/*
#define NXD_MQTT_SECURE_MEMCPY                  memcpy
*/

/* Define memcmp function used internal. */
/*
#define NXD_MQTT_SECURE_MEMCMP                  memcmp
*/

/* Define memset function used internal. */
/*
#define NXD_MQTT_SECURE_MEMSET                  memset
*/

/* Define memmove function used internal. */
/*
#define NXD_MQTT_SECURE_MEMMOVE                 memmove
*/

/* Define the default TCP socket window size. */
/*
#define NXD_MQTT_CLIENT_SOCKET_WINDOW_SIZE      8192
*/

/* Define the default MQTT Thread time slice. */
/*
#define NXD_MQTT_CLIENT_THREAD_TIME_SLICE       2
*/

/*****************************************************************************/
/********************* Configuration options for HTTP ************************/
/*****************************************************************************/

/* Type of service required for the HTTP TCP requests. By default, this value
   is defined as NX_IP_NORMAL to indicate normal IP packet service. */
/*
#define NX_HTTP_TYPE_OF_SERVICE               NX_IP_NORMAL
*/

/* Fragment enable for HTTP TCP requests. By default, this value is
   NX_DONT_FRAGMENT to disable HTTP TCP fragmenting. */
/*
#define NX_HTTP_FRAGMENT_OPTION               NX_DONT_FRAGMENT
*/

/* Specifies the number of bytes allowed in a client supplied resource name.
   The default value is set to 40. */
/*
#define NX_HTTP_MAX_RESOURCE                  40
*/

/* Specifies the number of bytes allowed in a client supplied username.
   The default value is set to 20. */
/*
#define NX_HTTP_MAX_NAME                      20
*/

/* Specifies the number of bytes allowed in a client supplied password.
   The default value is set to 20. */
/*
#define NX_HTTP_MAX_PASSWORD                  20
*/

/*****************************************************************************/
/***************** Configuration options for HTTP Client *********************/
/*****************************************************************************/

/* Specifies the minimum size of the packets in the pool specified at Client
   creation. The minimum size is needed to ensure the complete HTTP header can
   be contained in one packet. The default value is set to 600. */
/*
#define NX_HTTP_CLIENT_MIN_PACKET_SIZE        600
*/

/*****************************************************************************/
/***************** Configuration options for HTTP Server *********************/
/*****************************************************************************/

/* The number of timer ticks the Server thread is allowed to run before yielding
   to threads of the same priority. The default value is 2. */
/*
#define NX_HTTP_SERVER_THREAD_TIME_SLICE      2
*/

/* Specifies the maximum size of the HTTP header field. The default value is 256. */
/*
#define NX_HTTP_MAX_HEADER_FIELD              256
*/

/* If defined, enables HTTP Server to support multipart HTTP requests. */
/*
#define NX_HTTP_MULTIPART_ENABLE
*/

/* Specifies the number of connections that can be queued for the HTTP Server.
   The default value is set to 5. */
/*
#define NX_HTTP_SERVER_MAX_PENDING            5
*/

/* This specifies the maximum number of packets that can be enqueued on the
   Server socket retransmission queue. If the number of packets enqueued
   reaches this number, no more packets can be sent until one or more enqueued
   packets are released. The default value is set to 20. */
/*
#define NX_HTTP_SERVER_TRANSMIT_QUEUE_DEPTH   20
*/

/* Specifies the minimum size of the packets in the pool specified at Server
   creation. The minimum size is needed to ensure the complete HTTP header can
   be contained in one packet. The default value is set to 600. */
/*
#define NX_HTTP_SERVER_MIN_PACKET_SIZE        600
*/

/* Set the Server socket retransmission timeout in seconds.
   The default value is set to 2. */
/*
#define NX_HTTP_SERVER_RETRY_SECONDS          2
*/

/* This value is used to set the next retransmission timeout. The current
   timeout is multiplied by the number of retransmissions thus far, shifted
   by the value of the socket timeout shift. The default value is set to 1 for
   doubling the timeout. */
/*
#define NX_HTTP_SERVER_RETRY_SHIFT            1
*/

/* This sets the maximum number of retransmissions on Server socket.
   The default value is set to 10. */
/*
#define NX_HTTP_SERVER_RETRY_MAX              10
*/

/* The priority of the HTTP Server thread. By default, this value is defined
   as 16 to specify priority 16. */
/*
#define NX_HTTP_SERVER_PRIORITY               16
*/

/* Server socket window size. By default, this value is 2048 bytes */
/*
#define NX_HTTP_SERVER_WINDOW_SIZE            2048
*/

/*****************************************************************************/
/***************** Configuration options for AUTO_IP *************************/
/*****************************************************************************/

/* This option enables printing a debugging trace using printf.*/
/*
#define NX_AUTO_IP_DEBUG
*/

/* The number of seconds to wait before sending first probe.
   By default, this value is defined as 1.*/
/*
#define NX_AUTO_IP_PROBE_WAIT         1
*/

/* The number of ARP probes to send.
   By default, this value is defined as 3.*/
/*
#define NX_AUTO_IP_PROBE_NUM         3
*/

/* The minimum number of seconds to wait between sending probes.
   By default, this value is defined as 1.*/
/*
#define NX_AUTO_IP_PROBE_MIN         1
*/

/* The maximum number of seconds to wait between sending probes.
   By default, this value is defined as 2.*/
/*
#define NX_AUTO_IP_PROBE_MAX         2
*/

/* The number of AutoIP conflicts before increasing processing delays.
   By default, this value is defined as 10.*/
/*
#define NX_AUTO_IP_MAX_CONFLICTS         10
*/

/* The number of seconds to extend the wait period when the total
   number of conflicts is exceeded. By default, this value is defined as 60.*/
/*
#define NX_AUTO_IP_RATE_LIMIT_INTERVAL         60
*/

/* The number of seconds to wait before sending announcement.
   By default, this value is defined as 2.*/
/*
#define NX_AUTO_IP_ANNOUNCE_WAIT         2
*/

/* The number of ARP announces to send.
   By default, this value is defined as 2.*/
/*
#define NX_AUTO_IP_ANNOUNCE_NUM         2
*/

/* The number of seconds to wait between sending announces.
   By default, this value is defined as 2.*/
/*
#define NX_AUTO_IP_ANNOUNCE_INTERVAL         2
*/

/* The number of seconds to wait between defense announces.
   By default, this value is defined as 10.*/
/*
#define NX_AUTO_IP_DEFEND_INTERVAL         10
*/

/*****************************************************************************/
/************** Configuration options for mDNS *******************************/
/*****************************************************************************/

/* Disables the mDNS/DNS-SD server functionality. Without the server functionality,
   the mDNS/DNS-SD module does not announce services provided by local host,
   nor does it respond to mDNS enquiries. This symbol is not defined by default.*/
/*
#define NX_MDNS_DISABLE_SERVER
*/

/* Maximum IPv6 addresses count of host. The default value is 2.*/
/*
#define NX_MDNS_IPV6_ADDRESS_COUNT        2
*/

/* Maximum string size for host name. The default value is 64.
   Does not include the NULL terminator.*/
/*
#define NX_MDNS_HOST_NAME_MAX        64
*/

/* Maximum string size for service name. The default value is 64.
   Does not include the NULL terminator.*/
/*
#define NX_MDNS_SERVICE_NAME_MAX        64
*/

/* Maximum string size for domain name. The default value is 16.
   Does not include the NULL terminator.*/
/*
#define NX_MDNS_DOMAIN_NAME_MAX        16
*/

/* Maximum conflict count for host name or service name.
   The default value is 8.*/
/*
#define NX_MDNS_CONFLICT_COUNT        8
*/

/* Define UDP socket create options.*/
/*
#define NX_MDNS_UDP_TYPE_OF_SERVICE        NX_IP_NORMAL
*/

/* TTL value for resource records with host name, in second.
   The default value is 120 for 120s.*/
/*
#define NX_MDNS_RR_TTL_HOST        120
*/

/* TTL value for other resource records, in second.
   The default value is 4500 for 75 minutes.*/
/*
#define NX_MDNS_RR_TTL_OTHER        4500
*/

/* The time interval, in ticks, between mDNS probing messages.
   The default value is 25 ticks for 250ms.*/
/*
#define NX_MDNS_PROBING_TIMER_COUNT        25
*/

/* The time interval, in ticks, between mDNS announcement messages.
   The default value is 25 ticks for 250ms.*/
/*
#define NX_MDNS_ANNOUNCING_TIMER_COUNT        25
*/

/* The time interval, in ticks, between repeated "goodbye" messages.
   The default value is 25 ticks for 250ms.*/
/*
#define NX_MDNS_GOODBYE_TIMER_COUNT        25
*/

/* The minimum delay for sending first query, in ticks.
   The default value is 2 ticks for 20ms.*/
/*
#define NX_MDNS_QUERY_DELAY_MIN        2
*/

/* The delay range for sending first query, in ticks.
   The default value is 10 ticks for 100ms.*/
/*
#define NX_MDNS_QUERY_DELAY_RANGE        10
*/

/* The delay, in ticks, in responding to a query to a service that is
   unique to the local network. The default value is 1 tick for 10ms.*/
/*
#define NX_MDNS_RESPONSE_UNIQUE_DELAY        1
*/

/* The minimum delay, in ticks, in responding to a query to a shared resource.
   The default value is 2 ticks for 20ms.*/
/*
#define NX_MDNS_RESPONSE_SHARED_DELAY_MIN        2
*/

/* The delay range, in ticks, in responding to a query to a shared resource.
   The default value is 10 ticks for 100ms.*/
/*
#define NX_MDNS_RESPONSE_SHARED_DELAY_RANGE        10
*/

/* The minimum delay, in ticks, in responding to a query with TC bit.
   The default value is 40 ticks for 400ms.*/
/*
#define NX_MDNS_RESPONSE_TC_DELAY_MIN        40
*/

/* The delay range, in ticks, in responding to a query with TC bit.
   The default value is 10 ticks for 100ms.*/
/*
#define NX_MDNS_RESPONSE_TC_DELAY_RANGE        10
*/

/* When sending out mDNS responses, the packet contains responses that
   otherwise would be sent within this timer counter range.
   The timer count range is expressed in ticks. The default value is 12 for
   120ms.This value allows a response to include messages that would be sent
   within the next 120ms range if each tick is 10ms.*/
/*
#define NX_MDNS_TIMER_COUNT_RANGE        12
*/

/* The number of retransmitted probing messages. The default value is 3.*/
/*
#define NX_MDNS_PROBING_RETRANSMIT_COUNT        3
*/

/* The number of retransmitted "goodbye" messages. The default value is 1.*/
/*
#define NX_MDNS_GOODBYE_RETRANSMIT_COUNT        1
*/

/* The number of queries that no multicast response,
   then the host may take this as an indication that
   the record may no longer be valid. The default value is 2.*/
/*
#define NX_MDNS_POOF_MIN_COUNT        2
*/

/* Disable the mDNS Client functionality.  By default, mDNS client function is enabled. */
/*
#define NX_MDNS_DISABLE_CLIENT
*/

/*****************************************************************************/
/******************** Configuration options for PPP **************************/
/*****************************************************************************/

/* If defined, PPP can transmit packet over Ethernet.*/
/*
#define NX_PPP_PPPOE_ENABLE
*/

/* If defined, internal PPP information gathering is disabled.*/
/*
#define NX_PPP_DISABLE_INFO
*/

/* If defined, internal PPP debug log is enabled.*/
/*
#define NX_PPP_DEBUG_LOG_ENABLE
*/

/* If defined, internal PPP debug log printf to stdio is enabled.
   This is only valid if the debug log is also enabled.*/
/*
#define NX_PPP_DEBUG_LOG_PRINT_ENABLE
*/

/* If defined, internal PPP CHAP logic is removed, including the MD5 digest logic.*/
/*
#define NX_PPP_DISABLE_CHAP
*/

/* If defined, internal PPP PAP logic is removed.*/
/*
#define NX_PPP_DISABLE_PAP
*/

/* If defined, the primary DNS Server Option is disabled in
   the IPCP response. By default this option is not defined.*/
/*
#define NX_PPP_DNS_OPTION_DISABLE
*/

/* This specifies how many times the PPP host will request
  a DNS Server address from the peer in the IPCP state.
  This has no effect if NX_PPP_DNS_OPTION_DISABLE is defined.
  The default value is 2.*/
/*
#define NX_PPP_DNS_ADDRESS_MAX_RETRIES       2
*/

/* Time-slice option for PPP threads.
   By default, this value is TX_NO_TIME_SLICE.
   This define can be set by the application prior
   to inclusion of nx_ppp.h.By default, it is TX_NO_TIME_SLICE.*/
/*
#define NX_PPP_THREAD_TIME_SLICE       TX_NO_TIME_SLICE
*/

/* Specifies the Maximum Receive Unit (MRU) for PPP.
   By default, this value is 1,500 bytes (the minimum value).
   This define can be set by the application prior to inclusion of nx_ppp.h.*/
/*
#define NX_PPP_MRU       1480
*/

/* Specifies the Maximum Receive Unit (MRU) for PPP.
   By default, this value is 1,500 bytes (the minimum value).
   This define can be set by the application prior to inclusion of nx_ppp.h.*/
/*
#define NX_PPP_MINIMUM_MRU       1480
*/

/* Specifies the size of the receive character serial buffer.
   By default, this value is 3,000 bytes.
   This define can be set by the application prior to inclusion of nx_ppp.h.
   By default, it is NX_PPP_MRU*2.*/
/*
#define NX_PPP_SERIAL_BUFFER_SIZE       2960
*/

/* Specifies the size of "name" strings used in authentication.
   The default value is set to 32bytes,
   but can be redefined prior to inclusion of *nx_ppp.h.*/
/*
#define NX_PPP_NAME_SIZE       32
*/

/* Specifies the size of "password" strings used in authentication.
   The default value is set to 32bytes,
   but can be redefined prior to inclusion of *nx_ppp.h.*/
/*
#define NX_PPP_PASSWORD_SIZE       32
*/

/* Specifies the size of "value" strings used in CHAP authentication.
   The default value is set to 32bytes,
   but can be redefined prior to inclusion of nx_ppp.h.*/
/*
#define NX_PPP_VALUE_SIZE       32
*/

/* Specifies the size of "hashed value" strings used in CHAP authentication.
   The default value is set to 16 bytes, but can be redefined prior
   to inclusion of nx_ppp.h.*/
/*
#define NX_PPP_HASHED_VALUE_SIZE       16
*/

/* This defines the number of times the PPP thread task times out
   waiting to receive the next character in a PPP message stream.
   Thereafter, PPP releases the packet and begins waiting to
   receive the next PPP message. The default value is 4. */
/*
#define NX_PPP_RECEIVE_TIMEOUTS       4
*/

/* This defines the wait option (in seconds) for the PPP task to
   receive a response to a PPP protocol request message. The default value is 4 seconds. */
/*
#define NX_PPP_PROTOCOL_TIMEOUT       4
*/

/* Size of debug log (number of entries in the debug log).
   On reaching the last entry, the debug capture wraps
   to the first entry and overwrites any data previously captured.
   The default value is 50. */
/*
#define NX_PPP_DEBUG_LOG_SIZE       50
*/

/* Maximum amount of data captured from a received packet
   payload and saved to debug output. The default value is 50.*/
/*
#define NX_PPP_DEBUG_FRAME_SIZE       50
*/

/* This defines the max number of retries if the PPP times out before sending
   another LCP configure request message. When this number is reached the
   PPP handshake is aborted and the link status is down. The default value is 20. */
/*
#define NX_PPP_MAX_LCP_PROTOCOL_RETRIES       20
*/

/* This defines the max number of retries if the PPP times out
   before sending another PAP authentication request message.
   When this number is reached the PPP handshake is aborted and the
   link status is down. The default value is 20.*/
/*
#define NX_PPP_MAX_PAP_PROTOCOL_RETRIES       20
*/

/* This defines the max number of retries if the PPP times out
   before sending another CHAP challenge message. When this number
   is reached the PPP handshake is aborted and the link status is down.
   The default value is 20.*/
/*
#define NX_PPP_MAX_CHAP_PROTOCOL_RETRIES       20
*/

/* Define the packet header.*/
/*
#define NX_PPP_PACKET        22
*/

/* Define the minimum PPP packet payload, the PPP commands
  (LCP, PAP, CHAP, IPCP) should be in one packet.
   The default value is (NX_PPP_PACKET + 128).*/
/*
#define NX_PPP_MIN_PACKET_PAYLOAD        (NX_PPP_PACKET + 128)
*/

/* Size of the NAK list.*/
/*
#define NX_PPP_OPTION_MESSAGE_LENGTH        64
*/

/*****************************************************************************/
/******************** Configuration options for SNTP *************************/
/*****************************************************************************/

/* This option sets the size of the Client thread stack. The default NetX Duo
   SNTP Client size is 2048. */
/*
#define NX_SNTP_CLIENT_THREAD_STACK_SIZE        2048
*/

/* This option sets the time slice of the scheduler allows for Client thread
   execution. The default NetX Duo SNTP Client size is TX_NO_TIME_SLICE. */
/*
#define NX_SNTP_CLIENT_THREAD_TIME_SLICE        TX_NO_TIME_SLICE
*/

/* This option sets the Client thread priority.
   The NetX Duo SNTP Client default value is 2. */
/*
#define NX_SNTP_CLIENT_THREAD_PRIORITY          2
*/

/* This option sets the sets the level of priority at which the Client thread
   allows preemption. The default NetX Duo SNTP Client value is set to
   NX_SNTP_CLIENT_ THREAD_PRIORITY.  */
/*
#define NX_SNTP_CLIENT_PREEMPTION_THRESHOLD     NX_SNTP_CLIENT_THREAD_PRIORITY
*/

/* This option sets the UDP socket name. The NetX Duo SNTP Client UDP socket
   name default is "SNTP Client socket". */
/*
#define NX_SNTP_CLIENT_UDP_SOCKET_NAME          "SNTP Client socket"
*/

/* This sets the port which the Client socket is bound to. The default NetX
   Duo SNTP Client port is 123.  */
/*
#define NX_SNTP_CLIENT_UDP_PORT                 123
*/

/* This is port which the Client sends SNTP messages to the SNTP Server on.
   The default NetX SNTP Server port is 123. */
/*
#define NX_SNTP_SERVER_UDP_PORT                 123
*/

/* Specifies the number of routers a Client packet can pass before it is
   discarded. The default NetX Duo SNTP Client is set to 0x80
   (NX_IP_TIME_TO_LIVE). */
/*
#define NX_SNTP_CLIENT_TIME_TO_LIVE             NX_IP_TIME_TO_LIVE
*/

/* Maximum number of UDP packets (datagrams) that can be queued in the NetX
   Duo SNTP Client socket. Additional packets received mean the oldest packets
   are released. The default NetX Duo SNTP Client is set to 5. */
/*
#define NX_SNTP_CLIENT_MAX_QUEUE_DEPTH          5
*/

/* SNTP version used by the Client The NetX Duo SNTP Client API was based on
   Version 4. The default value is 3. */
/*
#define NX_SNTP_CLIENT_NTP_VERSION              3
*/

/* Oldest SNTP version the Client will be able to work with. The NetX Duo SNTP
   Client default is Version 3. */
/*
#define NX_SNTP_CLIENT_MIN_NTP_VERSION          3
*/

/* The lowest level (highest numeric stratum level) SNTP Server stratum the
   Client will accept. The NetX Duo SNTP Client default is 2. */
/*
#define NX_SNTP_CLIENT_MIN_SERVER_STRATUM       2
*/

/* The minimum time adjustment in milliseconds the Client will make to its
   local clock time. Time adjustments below this will be ignored. The NetX
   Duo SNTP Client default is 10. */
/*
#define NX_SNTP_CLIENT_MIN_TIME_ADJUSTMENT      10
*/

/* The maximum time adjustment in milliseconds the Client will make to its local
   clock time. For time adjustments above this amount, the local clock adjustment
   is limited to the maximum time adjustment. The NetX Duo SNTP Client default
   is 180000 (3 minutes). */
/*
#define NX_SNTP_CLIENT_MAX_TIME_ADJUSTMENT      180000
*/

/* This enables the maximum time adjustment to be waived when the Client
   receives the first update from its time server. Thereafter, the maximum
   time adjustment is enforced. The intention is to get the Client in synch
   with the server clock as soon as possible. The NetX Duo SNTP Client default
   is NX_TRUE. */
/*
#define NX_SNTP_CLIENT_IGNORE_MAX_ADJUST_STARTUP    NX_TRUE
*/

/* This determines if the SNTP Client in unicast mode should send its first SNTP
   request with the current SNTP server after a random wait interval. It is used
   in cases where significant numbers of SNTP Clients are starting up
   simultaneously to limit traffic congestion on the SNTP Server. The default
   value is NX_FALSE. */
/*
#define NX_SNTP_CLIENT_RANDOMIZE_ON_STARTUP     NX_FALSE
*/

/* Maximum allowable amount of time (seconds) elapsed without a valid time
   update received by the SNTP Client. The SNTP Client will continue in
   operation but the SNTP Server status is set to NX_FALSE. The default
   value is 7200. */
/*
#define NX_SNTP_CLIENT_MAX_TIME_LAPSE           7200
*/

/* The interval (seconds) at which the SNTP Client timer updates the SNTP Client
   time remaining since the last valid update received, and the unicast Client
   updates the poll interval time remaining before sending the next SNTP
   update request. The default value is 1. */
/*
#define NX_SNTP_UPDATE_TIMEOUT_INTERVAL         1
*/

/* The time interval during which the SNTP Client task sleeps. This allows the
   application API calls to be executed by the SNTP Client. The default value
   is 1 timer tick. */
/*
#define NX_SNTP_CLIENT_SLEEP_INTERVAL           1
*/

/* The starting poll interval (seconds) on which the Client sends a unicast
   request to its SNTP server. The NetX Duo SNTP Client default is 3600. */
/*
#define NX_SNTP_CLIENT_UNICAST_POLL_INTERVAL    3600
*/

/* The factor by which the current Client unicast poll interval is increased.
   When the Client fails to receive a server time update, or receiving
   indications from the server that it is temporarily unavailable
   (e.g. not synchronized yet) for time update service, it will increase the
   current poll interval by this rate up to but not exceeding
   NX_SNTP_CLIENT_MAX_TIME_LAPSE. The default is 2. */
/*
#define NX_SNTP_CLIENT_EXP_BACKOFF_RATE         2
*/

/* This option if enabled requires that the SNTP Client calculate round trip
   time of SNTP messages when applying Server updates to the local clock.
   The default value is NX_FALSE (disabled). */
/*
#define NX_SNTP_CLIENT_RTT_REQUIRED             NX_FALSE
*/

/* The maximum server clock dispersion (microseconds), which is a measure of
   server clock precision, the Client will accept. To disable this requirement,
   set the maximum root dispersion to 0x0. The NetX Duo SNTP Client default is
   set to 50000. */
/*
#define NX_SNTP_CLIENT_MAX_ROOT_DISPERSION      50000
*/

/* The limit on the number of consecutive invalid updates received from the
   Client server in either broadcast or unicast mode. When this limit is
   reached, the Client sets the current SNTP Server status to invalid (NX_FALSE)
   although it will continue to try to receive updates from the Server.
   The NetX Duo SNTP Client default is 3. */
/*
#define NX_SNTP_CLIENT_INVALID_UPDATE_LIMIT     3
*/

/* To display date in year/month/date format, set this value to equal or less
   than current year (need not be same year as in NTP time being evaluated).
   The default value is 2015. */
/*
#define NX_SNTP_CURRENT_YEAR                    2015
*/

/* This is the number of seconds into the first NTP Epoch on the master NTP
   clock. It is defined as 0xBA368E80. To disable display of NTP seconds into
   date and time, set to zero. */
/*
#define NTP_SECONDS_AT_01011999                 0xBA368E80
*/

/*****************************************************************************/
/****************** Configuration options for WEB HTTP ***********************/
/*****************************************************************************/

/* Type of service required for the HTTPS TCP requests. By default, this value
   is defined as NX_IP_NORMAL to indicate normal IP packet service. */
/*
#define NX_WEB_HTTP_TYPE_OF_SERVICE             NX_IP_NORMAL
*/

/* Fragment enable for HTTP TCP requests. By default, this value is
   NX_DONT_FRAGMENT to disable HTTP TCP fragmenting. */
/*
#define NX_WEB_HTTP_FRAGMENT_OPTION             NX_DONT_FRAGMENT
*/

/* Specifies the number of routers this packet can pass before it is discarded.
   The default value is set to 0x80. */
/*
#define NX_WEB_HTTP_TIME_TO_LIVE                0x80
*/

/* Specifies the number of bytes allowed in a client supplied resource name.
   The default value is set to 40. */
/*
#define NX_WEB_HTTP_MAX_RESOURCE                40
*/

/* Specifies the number of bytes allowed in a client supplied username.
   The default value is set to 20. */
/*
#define NX_WEB_HTTP_MAX_NAME                    20
*/

/* Specifies the number of bytes allowed in a client supplied password.
   The default value is set to 20. */
/*
#define NX_WEB_HTTP_MAX_PASSWORD                20
*/

/* If defined, this macro enables TLS and HTTPS. Leave undefined to free up
   resources if only plain-text HTTP is desired. By default, this macro is not
   defined. */
/*
#define NX_WEB_HTTPS_ENABLE
*/

/* If defined, authentication using the MD5 digest is enabled on the HTTPS
   Server. By default it is not defined. */
/*
#define NX_WEB_HTTP_DIGEST_ENABLE
*/

/* Specifies the maximum size of the HTTP header field.
   The default value is 256. */
/*
#define NX_WEB_HTTP_MAX_HEADER_FIELD            256
*/

/* Defined, this option provides a stub for FileX dependencies. The HTTPS Client
   will function without any change if this option is defined. The HTTPS Server
   will need to either be modified or the user will have to create a handful of
   FileX services in order to function properly. */
/*
#define NX_WEB_HTTP_NO_FILEX
*/

/* The priority of the HTTPS Server thread. By default, this value is defined
   as 4 to specify priority 4. */
/*
#define NX_WEB_HTTP_SERVER_PRIORITY             4
*/

/* Server socket window size. By default, this value is 8192. */
/*
#define NX_WEB_HTTP_SERVER_WINDOW_SIZE          8192
*/

/* Specifies the number of simultaneous sessions for an HTTP or HTTPS Server.
   A TCP socket and a TLS session (if HTTPS is enabled) are allocated for each
   session. The default value is set to 2. */
/*
#define NX_WEB_HTTP_SERVER_SESSION_MAX          2
*/

/* Specifies the number of connections that can be queued for the HTTPS Server.
   The default value is set to twice the maximum number of server sessions
   ((NX_WEB_HTTP_SERVER_SESSION_MAX << 1). */
/*
#define NX_WEB_HTTP_SERVER_MAX_PENDING          (NX_WEB_HTTP_SERVER_SESSION_MAX << 1)
*/

/* The number of timer ticks the Server thread is allowed to run before yielding
   to threads of the same priority. The default value is 2. */
/*
#define NX_WEB_HTTP_SERVER_THREAD_TIME_SLICE    2
*/

/* Specifies the minimum size of the packets in the pool specified at server
   creation. The minimum size is needed to ensure the complete HTTP header can
   be contained in one packet. The default value is set to 600. */
/*
#define NX_WEB_HTTP_SERVER_MIN_PACKET_SIZE      600
*/

/* Set the Server socket retransmission timeout in seconds. The default value
   is set to 2.*/
/*
#define NX_WEB_HTTP_SERVER_RETRY_SECONDS        2
*/

/* This specifies the maximum number of packets that can be enqueued on the
   Server socket retransmission queue. If the number of packets enqueued
   reaches this number, no more packets can be sent until one or more enqueued
   packets are released. */
/*
#define NX_WEB_HTTP_SERVER_TRANSMIT_QUEUE_DEPTH   20
*/

/* This value is used to set the next retransmission timeout. The current
   timeout is multiplied by the number of retransmissions thus far, shifted
   by the value of the socket timeout shift. */
/*
#define NX_WEB_HTTP_SERVER_RETRY_SHIFT          1
*/

/* If defined, enables HTTPS Server to support multipart HTTP requests. */
/*
#define NX_WEB_HTTP_MULTIPART_ENABLE
*/

/* This sets the maximum number of retransmissions on Server socket. The default
   value is set to 10. */
/*
#define NX_WEB_HTTP_SERVER_RETRY_MAX            10
*/

/* Defined, the TCP/IP offload feature is enabled.
   NX_ENABLE_INTERFACE_CAPABILITY must be defined to enable this feature.  */
/* #define NX_ENABLE_TCPIP_OFFLOAD */

#ifdef NX_DISABLE_IPV6
#ifdef NX_DISABLE_IPV4
#error "At least one of the IPv4 or IPv6 protocols must be enabled"
#endif
#endif

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

#endif /* NX_USER_H */
