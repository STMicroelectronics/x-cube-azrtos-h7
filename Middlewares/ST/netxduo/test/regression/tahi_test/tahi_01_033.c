#include "nx_api.h"
#if defined(NX_TAHI_ENABLE) && defined(FEATURE_NX_IPV6)  

#include "netx_tahi.h"

static char pkt1[] = {
0x00, 0x11, 0x22, 0x33, 0x44, 0x56, 0x00, 0x00, 
0x00, 0x00, 0x01, 0x00, 0x86, 0xdd, 0x60, 0x00, 
0x00, 0x00, 0x00, 0x18, 0x3c, 0xff, 0xfe, 0x80, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 
0x00, 0xff, 0xfe, 0x00, 0x01, 0x00, 0xfe, 0x80, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x11, 
0x22, 0xff, 0xfe, 0x33, 0x44, 0x56, 0x3a, 0x00, 
0x47, 0x04, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 
0x09, 0x05, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 
0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };

#if 0
//pkt 2 -- pkt 5 is wrong packet
//10.0.0.1    10.0.0.255    BROWSER    245
static char pkt2[] = {
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x16, 
0x17, 0xc9, 0xa1, 0xaf, 0x08, 0x00, 0x45, 0x00, 
0x00, 0xe7, 0x41, 0xbd, 0x00, 0x00, 0x40, 0x11, 
0x00, 0x00, 0x0a, 0x00, 0x00, 0x01, 0x0a, 0x00, 
0x00, 0xff, 0x00, 0x8a, 0x00, 0x8a, 0x00, 0xd3, 
0x15, 0xe4, 0x11, 0x0a, 0x72, 0x96, 0x0a, 0x00, 
0x00, 0x01, 0x00, 0x8a, 0x00, 0xbd, 0x00, 0x00, 
0x20, 0x45, 0x45, 0x45, 0x49, 0x43, 0x41, 0x43, 
0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 
0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 
0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 0x41, 0x41, 
0x41, 0x00, 0x20, 0x46, 0x48, 0x45, 0x50, 0x46, 
0x43, 0x45, 0x4c, 0x45, 0x48, 0x46, 0x43, 0x45, 
0x50, 0x46, 0x46, 0x46, 0x41, 0x43, 0x41, 0x43, 
0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 
0x41, 0x42, 0x4f, 0x00, 0xff, 0x53, 0x4d, 0x42, 
0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x23, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x23, 0x00, 0x56, 0x00, 0x03, 
0x00, 0x01, 0x00, 0x01, 0x00, 0x02, 0x00, 0x34, 
0x00, 0x5c, 0x4d, 0x41, 0x49, 0x4c, 0x53, 0x4c, 
0x4f, 0x54, 0x5c, 0x42, 0x52, 0x4f, 0x57, 0x53, 
0x45, 0x00, 0x0f, 0x76, 0x80, 0xfc, 0x0a, 0x00, 
0x44, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x04, 0x09, 0x03, 0x9a, 0x84, 0x00, 0x0f, 0x01, 
0x55, 0xaa, 0x64, 0x68, 0x00 };

static char pkt3[] = {
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x16, 
0x17, 0xc9, 0xa1, 0xaf, 0x08, 0x00, 0x45, 0x00, 
0x00, 0xe7, 0x41, 0xbd, 0x00, 0x00, 0x40, 0x11, 
0x00, 0x00, 0x0a, 0x00, 0x00, 0x01, 0x0a, 0x00, 
0x00, 0xff, 0x00, 0x8a, 0x00, 0x8a, 0x00, 0xd3, 
0x15, 0xe4, 0x11, 0x0a, 0x72, 0x96, 0x0a, 0x00, 
0x00, 0x01, 0x00, 0x8a, 0x00, 0xbd, 0x00, 0x00, 
0x20, 0x45, 0x45, 0x45, 0x49, 0x43, 0x41, 0x43, 
0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 
0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 
0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 0x41, 0x41, 
0x41, 0x00, 0x20, 0x46, 0x48, 0x45, 0x50, 0x46, 
0x43, 0x45, 0x4c, 0x45, 0x48, 0x46, 0x43, 0x45, 
0x50, 0x46, 0x46, 0x46, 0x41, 0x43, 0x41, 0x43, 
0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 
0x41, 0x42, 0x4f, 0x00, 0xff, 0x53, 0x4d, 0x42, 
0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x23, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x23, 0x00, 0x56, 0x00, 0x03, 
0x00, 0x01, 0x00, 0x01, 0x00, 0x02, 0x00, 0x34, 
0x00, 0x5c, 0x4d, 0x41, 0x49, 0x4c, 0x53, 0x4c, 
0x4f, 0x54, 0x5c, 0x42, 0x52, 0x4f, 0x57, 0x53, 
0x45, 0x00, 0x0f, 0x76, 0x80, 0xfc, 0x0a, 0x00, 
0x44, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x04, 0x09, 0x03, 0x9a, 0x84, 0x00, 0x0f, 0x01, 
0x55, 0xaa, 0x64, 0x68, 0x00 };

static char pkt4[] = {
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x16, 
0x17, 0xc9, 0xa1, 0xaf, 0x08, 0x00, 0x45, 0x00, 
0x00, 0xe7, 0x41, 0xbe, 0x00, 0x00, 0x40, 0x11, 
0x00, 0x00, 0x0a, 0x00, 0x00, 0x01, 0x0a, 0x00, 
0x00, 0xff, 0x00, 0x8a, 0x00, 0x8a, 0x00, 0xd3, 
0x15, 0xe4, 0x11, 0x0a, 0x72, 0x97, 0x0a, 0x00, 
0x00, 0x01, 0x00, 0x8a, 0x00, 0xbd, 0x00, 0x00, 
0x20, 0x45, 0x45, 0x45, 0x49, 0x43, 0x41, 0x43, 
0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 
0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 
0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 0x41, 0x41, 
0x41, 0x00, 0x20, 0x41, 0x42, 0x41, 0x43, 0x46, 
0x50, 0x46, 0x50, 0x45, 0x4e, 0x46, 0x44, 0x45, 
0x43, 0x46, 0x43, 0x45, 0x50, 0x46, 0x48, 0x46, 
0x44, 0x45, 0x46, 0x46, 0x50, 0x46, 0x50, 0x41, 
0x43, 0x41, 0x42, 0x00, 0xff, 0x53, 0x4d, 0x42, 
0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x23, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x23, 0x00, 0x56, 0x00, 0x03, 
0x00, 0x01, 0x00, 0x01, 0x00, 0x02, 0x00, 0x34, 
0x00, 0x5c, 0x4d, 0x41, 0x49, 0x4c, 0x53, 0x4c, 
0x4f, 0x54, 0x5c, 0x42, 0x52, 0x4f, 0x57, 0x53, 
0x45, 0x00, 0x0c, 0x76, 0x80, 0xfc, 0x0a, 0x00, 
0x57, 0x4f, 0x52, 0x4b, 0x47, 0x52, 0x4f, 0x55, 
0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x04, 0x09, 0x00, 0x10, 0x00, 0x80, 0x0f, 0x01, 
0x55, 0xaa, 0x44, 0x48, 0x00 };

static char pkt5[] = {
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x16, 
0x17, 0xc9, 0xa1, 0xaf, 0x08, 0x00, 0x45, 0x00, 
0x00, 0xe7, 0x41, 0xbe, 0x00, 0x00, 0x40, 0x11, 
0x00, 0x00, 0x0a, 0x00, 0x00, 0x01, 0x0a, 0x00, 
0x00, 0xff, 0x00, 0x8a, 0x00, 0x8a, 0x00, 0xd3, 
0x15, 0xe4, 0x11, 0x0a, 0x72, 0x97, 0x0a, 0x00, 
0x00, 0x01, 0x00, 0x8a, 0x00, 0xbd, 0x00, 0x00, 
0x20, 0x45, 0x45, 0x45, 0x49, 0x43, 0x41, 0x43, 
0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 
0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 
0x41, 0x43, 0x41, 0x43, 0x41, 0x43, 0x41, 0x41, 
0x41, 0x00, 0x20, 0x41, 0x42, 0x41, 0x43, 0x46, 
0x50, 0x46, 0x50, 0x45, 0x4e, 0x46, 0x44, 0x45, 
0x43, 0x46, 0x43, 0x45, 0x50, 0x46, 0x48, 0x46, 
0x44, 0x45, 0x46, 0x46, 0x50, 0x46, 0x50, 0x41, 
0x43, 0x41, 0x42, 0x00, 0xff, 0x53, 0x4d, 0x42, 
0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x23, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x23, 0x00, 0x56, 0x00, 0x03, 
0x00, 0x01, 0x00, 0x01, 0x00, 0x02, 0x00, 0x34, 
0x00, 0x5c, 0x4d, 0x41, 0x49, 0x4c, 0x53, 0x4c, 
0x4f, 0x54, 0x5c, 0x42, 0x52, 0x4f, 0x57, 0x53, 
0x45, 0x00, 0x0c, 0x76, 0x80, 0xfc, 0x0a, 0x00, 
0x57, 0x4f, 0x52, 0x4b, 0x47, 0x52, 0x4f, 0x55, 
0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x04, 0x09, 0x00, 0x10, 0x00, 0x80, 0x0f, 0x01, 
0x55, 0xaa, 0x44, 0x48, 0x00 };
#endif

static char pkt6[] = {
0x33, 0x33, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 
0x00, 0x00, 0x01, 0x00, 0x86, 0xdd, 0x60, 0x00, 
0x00, 0x00, 0x00, 0x20, 0x3a, 0xff, 0xfe, 0x80, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 
0x00, 0xff, 0xfe, 0x00, 0x01, 0x00, 0xff, 0x02, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x88, 0x00, 
0xcd, 0x04, 0x20, 0x00, 0x00, 0x00, 0xfe, 0x80, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 
0x00, 0xff, 0xfe, 0x00, 0x01, 0x00, 0x02, 0x01, 
0x00, 0x11, 0x33, 0x77, 0x55, 0x11 };

static char pkt7[] = {
0x00, 0x11, 0x22, 0x33, 0x44, 0x56, 0x00, 0x00, 
0x00, 0x00, 0x01, 0x00, 0x86, 0xdd, 0x60, 0x00, 
0x00, 0x00, 0x00, 0x10, 0x3a, 0xff, 0xfe, 0x80, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 
0x00, 0xff, 0xfe, 0x00, 0x01, 0x00, 0xfe, 0x80, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x11, 
0x22, 0xff, 0xfe, 0x33, 0x44, 0x56, 0x80, 0x00, 
0x09, 0x05, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 
0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };

static char pkt8[] = {
0x33, 0x33, 0xff, 0x00, 0x01, 0x00, 0x00, 0x11, 
0x22, 0x33, 0x44, 0x56, 0x86, 0xdd, 0x60, 0x00, 
0x00, 0x00, 0x00, 0x20, 0x3a, 0xff, 0xfe, 0x80, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x11, 
0x22, 0xff, 0xfe, 0x33, 0x44, 0x56, 0xff, 0x02, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x01, 0xff, 0x00, 0x01, 0x00, 0x87, 0x00, 
0xab, 0x68, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x80, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 
0x00, 0xff, 0xfe, 0x00, 0x01, 0x00, 0x01, 0x01, 
0x00, 0x11, 0x22, 0x33, 0x44, 0x56 };

#if 0
static char pkt9[] = {
0x33, 0x33, 0xff, 0x00, 0x01, 0x00, 0x00, 0x11, 
0x22, 0x33, 0x44, 0x56, 0x86, 0xdd, 0x60, 0x00, 
0x00, 0x00, 0x00, 0x20, 0x3a, 0xff, 0xfe, 0x80, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x11, 
0x22, 0xff, 0xfe, 0x33, 0x44, 0x56, 0xff, 0x02, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x01, 0xff, 0x00, 0x01, 0x00, 0x87, 0x00, 
0xab, 0x68, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x80, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 
0x00, 0xff, 0xfe, 0x00, 0x01, 0x00, 0x01, 0x01, 
0x00, 0x11, 0x22, 0x33, 0x44, 0x56 };

static char pkt10[] = {
0x33, 0x33, 0xff, 0x00, 0x01, 0x00, 0x00, 0x11, 
0x22, 0x33, 0x44, 0x56, 0x86, 0xdd, 0x60, 0x00, 
0x00, 0x00, 0x00, 0x20, 0x3a, 0xff, 0xfe, 0x80, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x11, 
0x22, 0xff, 0xfe, 0x33, 0x44, 0x56, 0xff, 0x02, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x01, 0xff, 0x00, 0x01, 0x00, 0x87, 0x00, 
0xab, 0x68, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x80, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 
0x00, 0xff, 0xfe, 0x00, 0x01, 0x00, 0x01, 0x01, 
0x00, 0x11, 0x22, 0x33, 0x44, 0x56 };
#endif

TAHI_TEST_SEQ tahi_01_033[] = {
    {TITLE, "01-033", 6, 0},
    {WAIT, NX_NULL, 0, 5},

    {INJECT, &pkt1[0], sizeof(pkt1), 0},
    {N_CHECK, (char *)NS, 0, 5},
    {INJECT, &pkt6[0], sizeof(pkt6), 0},
    {INJECT, &pkt7[0], sizeof(pkt7), 0},
    {CHECK, &pkt8[0], sizeof(pkt8), 10},
    {WAIT, NX_NULL, 0, 10},
    //{CHECK, &pkt9[0], sizeof(pkt9), 5},
    //{CHECK, &pkt10[0], sizeof(pkt10), 5},

    {CLEANUP, NX_NULL, 0, 0},
    {CLEAN_HOP_LIMIT, NX_NULL, 0, 0},
    {DUMP, NX_NULL, 0, 0}
};

int tahi_01_033_size = sizeof(tahi_01_033) / sizeof(TAHI_TEST_SEQ);

#endif /* NX_TAHI_ENABLE */
