/* Specify that this is a module! */
#define TXM_MODULE

/* Include the ThreadX module header. */
#include "txm_module.h"

/* Define constants. */
#define DEFAULT_STACK_SIZE         512
#define DEFAULT_BYTE_POOL_SIZE     2048

#define READONLY_REGION            0x24040000
#define READWRITE_REGION           0x24040100

typedef enum {
PROCESSING_NOT_STARTED    = 99,
WRITING_TO_READWRITE      = 88,
WRITING_TO_READONLY       = 77,
READING_FROM_READWRITE    = 66,
READING_FROM_READONLY     = 55,
PROCESSING_FINISHED       = 44
} ProgressState;

#define UNUSED(x) (void)(x)

/* Define the pool space in the bss section of the module. ULONG is used to
   get word alignment. */

#if defined(__GNUC__) || defined(__CC_ARM)
ULONG  default_module_pool_space[DEFAULT_BYTE_POOL_SIZE / 4] __attribute__ ((aligned(32)));
#else /* __ICCARM__ */
_Pragma("data_alignment=32") ULONG  default_module_pool_space[DEFAULT_BYTE_POOL_SIZE / 4];
#endif

/* Define the ThreadX object control block pointers. */

TX_THREAD               *thread_0;
TX_BYTE_POOL            *byte_pool_0;
TX_QUEUE                *resident_queue;

/* Function prototypes*/

void default_module_start(ULONG id);
void thread_0_entry(ULONG thread_input);
void Error_Handler(void);

/* Module entry function */
void    default_module_start(ULONG id)
{

    CHAR    *pointer;

    /* Allocate all the objects. In MPU mode, modules cannot allocate control blocks within
       their own memory area so they cannot corrupt the resident portion of ThreadX by overwriting
       the control block(s).  */
    txm_module_object_allocate((void*)&thread_0, sizeof(TX_THREAD));
    txm_module_object_allocate((void*)&byte_pool_0, sizeof(TX_BYTE_POOL));

    /* Create a byte memory pool from which to allocate the thread stacks.  */
    tx_byte_pool_create(byte_pool_0, "module byte pool 0", (UCHAR*)default_module_pool_space, DEFAULT_BYTE_POOL_SIZE);

    /* Allocate the stack for thread 0.  */
    tx_byte_allocate(byte_pool_0, (VOID **) &pointer, DEFAULT_STACK_SIZE, TX_NO_WAIT);

    /* Create the main thread.  */
    tx_thread_create(thread_0, "module thread 0", thread_0_entry, 0,
            pointer, DEFAULT_STACK_SIZE,
            2, 2, TX_NO_TIME_SLICE, TX_AUTO_START);

}

/* Module main thread function */
void thread_0_entry(ULONG thread_input)
{
  UINT status;
  ULONG s_msg;
  ULONG readbuffer;

  /* Request access to the queue from the module manager */
  status = txm_module_object_pointer_get(TXM_QUEUE_OBJECT, "Resident Queue", (VOID **)&resident_queue);

  if(status)
  {
    Error_Handler();
  }

  /* Writing to write and read region */
  s_msg = WRITING_TO_READWRITE;
  tx_queue_send(resident_queue, &s_msg, TX_NO_WAIT);
  *(ULONG *)READWRITE_REGION = 0xABABABAB;
  tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 10);

  /* WReading from write and read region */
  s_msg = READING_FROM_READWRITE;
  tx_queue_send(resident_queue, &s_msg, TX_NO_WAIT);
  readbuffer = *(ULONG*)READWRITE_REGION;
  tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 10);

  /* Reading from read only region */
  s_msg = READING_FROM_READONLY;
  tx_queue_send(resident_queue, &s_msg, TX_NO_WAIT);
  readbuffer = *(ULONG*)READONLY_REGION;
  tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 10);

  /* Writing to read only region */
  s_msg = WRITING_TO_READONLY;
  tx_queue_send(resident_queue, &s_msg, TX_NO_WAIT);
  *(ULONG *)READONLY_REGION = 0xABABABAB;
  tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 10);

  /* Suppress unused variable warning */
  UNUSED(readbuffer);
}

void Error_Handler(void)
{
  /* Nothing to do, block here */
  tx_thread_sleep(TX_WAIT_FOREVER);
}
