/* Basic NOR flash tests...  */

#include <stdio.h>
#include "lx_api.h"

#define     DEMO_STACK_SIZE         4096


/* Define the ThreadX object control blocks...  */
#ifndef LX_STANDALONE_ENABLE
TX_THREAD               thread_0;
#endif
UCHAR                   thread_0_stack[DEMO_STACK_SIZE];

//#define EXTENDED_CACHE
#ifdef EXTENDED_CACHE
UCHAR                   cache_memory[50000];
#endif

/* Define LevelX structures.  */

LX_NAND_FLASH   nand_sim_flash;
ULONG           buffer[2048];
ULONG           readbuffer[2048];

extern ULONG    *nand_flash_memory;

/* Define LevelX NOR flash simulator prototoypes.  */

UINT  _lx_nand_flash_simulator_erase_all(VOID);
UINT  _lx_nand_flash_simulator_initialize(LX_NAND_FLASH *nand_flash);

VOID  _fx_nand_flash_read_sectors(ULONG logical_sector, ULONG sectors, UCHAR *destination_buffer);
VOID  _fx_nand_flash_write_sectors(ULONG logical_sector, ULONG sectors, UCHAR *source_buffer);

#if 0
typedef struct SECTOR_STRUCT
{
    ULONG       words[2048/4];
} SECTOR;

SECTOR  test_write[8];
SECTOR  test_read[8];
#endif

#if 1

UCHAR   byte_buffer[2048];
UCHAR   ecc_bytes[24];
UCHAR   lx_ecc_buffer[24];

#endif

/* Define thread prototypes.  */

void    thread_0_entry(ULONG thread_input);

VOID ComputePageECC(UCHAR *data, INT size, UCHAR *code);
INT  CorrectPageECC(UCHAR *data, INT size, UCHAR *code);



/* Define main entry point.  */

int main()
{
  
    /* Enter the ThreadX kernel.  */
#ifndef LX_STANDALONE_ENABLE
    tx_kernel_enter();
#else
    thread_0_entry(0);
#endif

}


/* Define what the initial system looks like.  */
#ifndef LX_STANDALONE_ENABLE
void    tx_application_define(void *first_unused_memory)
{


    /* Create the main thread.  */
    tx_thread_create(&thread_0, "thread 0", thread_0_entry, 0,  
            thread_0_stack, DEMO_STACK_SIZE, 
            1, 1, TX_NO_TIME_SLICE, TX_AUTO_START);
}
#endif


#define NAND_MEMORY_SIZE 2056
ULONG nand_memory_space[NAND_MEMORY_SIZE];

/* For random read/write test */
#define MAX_SECTOR_ADDRESS 64000*4
#define SECTOR_SIZE 512

UCHAR local_data_buffer[MAX_SECTOR_ADDRESS * SECTOR_SIZE];

/* Define the test threads.  */

void    thread_0_entry(ULONG thread_input)
{

ULONG   i, j, sector;
UINT    status;

ULONG   *word_ptr;
UCHAR   *byte_ptr;

  
    /* Erase the simulated NOR flash.  */
    _lx_nand_flash_simulator_erase_all();
    
    /* Initialize LevelX.  */
    _lx_nand_flash_initialize();

    /* Test 1: Simple write 520 sectors and read 520 sectors.  */
    printf("Test 1: Simple write-read 520 sectors...........");

    status = lx_nand_flash_format(&nand_sim_flash, "sim nand flash", _lx_nand_flash_simulator_initialize, nand_memory_space, sizeof(nand_memory_space));
    if (status != LX_SUCCESS)
    {
        printf("FAILED!\n");
#ifdef BATCH_TEST
        exit(1);
#endif
        while (1)
        {
        }
    }

    status= lx_nand_flash_open(&nand_sim_flash, "sim nand flash", _lx_nand_flash_simulator_initialize, nand_memory_space, sizeof(nand_memory_space));
    if (status != LX_SUCCESS)
    {
        printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
    while(1)
        {
        }
    }
#ifdef EXTENDED_CACHE
    lx_nand_flash_extended_cache_enable(&nand_sim_flash, cache_memory, sizeof(cache_memory));
#endif
    
    /* Write 520 sectors....  */
    for (i = 0; i < 520; i++)
    {
        for (j = 0; j < 512; j++)
          buffer[j] =  i;
        
        status =  lx_nand_flash_sector_write(&nand_sim_flash, i, buffer);
      
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    }
    
    /* Close and reopen the flash. */
    lx_nand_flash_close(&nand_sim_flash);

    LX_MEMSET(nand_memory_space, 0, sizeof(nand_memory_space));
    LX_MEMSET(&nand_sim_flash, 0, sizeof(nand_sim_flash));

    status = lx_nand_flash_open(&nand_sim_flash, "sim nand flash", _lx_nand_flash_simulator_initialize, nand_memory_space, sizeof(nand_memory_space));
    if (status != LX_SUCCESS)
    {
        printf("FAILED!\n");
#ifdef BATCH_TEST
        exit(1);
#endif
        while (1)
        {
        }
    }

    /* Read back 520 sectors...  */
    for (i = 0; i < 520; i++)
    {
        
        status =  lx_nand_flash_sector_read(&nand_sim_flash, i, buffer);
      
        if (status != LX_SUCCESS)
        {
#ifdef BATCH_TEST
    exit(1);
#endif
          printf("FAILED!\n");
          while(1)
          {
          }
        }
        
        for (j = 0; j < 128; j++)
        {
          
          if (buffer[j] !=  i)
          {
            printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
            while(1)
            {
            }
          }
        }
    }

    _lx_nand_flash_close(&nand_sim_flash);
    printf("SUCCESS!\n");

    /* Test 2: Write same sector 320 times.  */
    printf("Test 2: Write same sector 320 times.............");
    
    /* Reinitialize...  */    
    _lx_nand_flash_simulator_erase_all();
    
    
    lx_nand_flash_initialize();

    status = lx_nand_flash_format(&nand_sim_flash, "sim nand flash", _lx_nand_flash_simulator_initialize, nand_memory_space, sizeof(nand_memory_space));


    lx_nand_flash_open(&nand_sim_flash, "sim nand flash", _lx_nand_flash_simulator_initialize, nand_memory_space, sizeof(nand_memory_space));
#ifdef EXTENDED_CACHE
    lx_nand_flash_extended_cache_enable(&nand_sim_flash, cache_memory, sizeof(cache_memory));
#endif
    
    for (j = 0; j < 512; j++)
         buffer[j] =  0xFFFFFFFF;
    
    /* Write same sector 320 sectors....  */
    for (i = 0; i < 320; i++)
    {
        for (j = 0; j < 512; j++)
          buffer[j] =  i;
      
        if (i == 319)
            buffer[j-1]--;
        status =  lx_nand_flash_sector_write(&nand_sim_flash, 7, buffer);
      
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        
        status =  lx_nand_flash_sector_read(&nand_sim_flash, 7, readbuffer);
      
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        
        for (j = 0; j < 128; j++)
        {
          
          if (buffer[j] !=  readbuffer[j])
          {
            printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
            while(1)
            {
            }
          }
        }
        
        /* Write other sectors just to have additional sectors to manage.  */
        if (i == 1)
          lx_nand_flash_sector_write(&nand_sim_flash, 1, buffer);
        if (i == 16)
          lx_nand_flash_sector_write(&nand_sim_flash, 16, buffer);
        if (i == 32)
          lx_nand_flash_sector_write(&nand_sim_flash, 32, buffer);
        if (i == 48)
          lx_nand_flash_sector_write(&nand_sim_flash, 48, buffer);
        if (i == 64)
          lx_nand_flash_sector_write(&nand_sim_flash, 64, buffer);
        if (i == 80)
          lx_nand_flash_sector_write(&nand_sim_flash, 80, buffer);
        if (i == 96)
          lx_nand_flash_sector_write(&nand_sim_flash, 96, buffer);
        if (i == 112)
          lx_nand_flash_sector_write(&nand_sim_flash, 112, buffer);
    }

    status =  lx_nand_flash_defragment(&nand_sim_flash);

    
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 7, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 319)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 1, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 1)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
     status =  lx_nand_flash_sector_read(&nand_sim_flash, 16, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 16)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 32, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 32)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 48, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 48)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 64, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 64)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 80, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 80)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 96, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 96)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 112, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 112)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }

    status =  lx_nand_flash_defragment(&nand_sim_flash);
    
    if (status != LX_NOT_SUPPORTED)
    {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }
   
    status =  lx_nand_flash_close(&nand_sim_flash);
    
    if (status != LX_SUCCESS)
    {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }
    printf("SUCCESS!\n");
  
    printf("Test 3: Page copy test..........................");

    /* Reinitialize...  */
    _lx_nand_flash_simulator_erase_all();


    lx_nand_flash_initialize();

    status = lx_nand_flash_format(&nand_sim_flash, "sim nand flash", _lx_nand_flash_simulator_initialize, nand_memory_space, sizeof(nand_memory_space));


    lx_nand_flash_open(&nand_sim_flash, "sim nand flash", _lx_nand_flash_simulator_initialize, nand_memory_space, sizeof(nand_memory_space));

    for (j = 0; j < 512; j++)
        buffer[j] = 0xFFFFFFFF;

    status = lx_nand_flash_sector_write(&nand_sim_flash, 0, buffer);

    /* Write same sector 320 sectors....  */
    for (i = 256; i < 512; i++)
    {
        for (j = 0; j < 512; j++)
            buffer[j] = i;

        if (i == 300) continue;

        status = lx_nand_flash_sector_write(&nand_sim_flash, i, buffer);

        if (status != LX_SUCCESS)
        {
            printf("FAILED!\n");
#ifdef BATCH_TEST
            exit(1);
#endif
            while (1)
            {
            }
        }
    }

    status = lx_nand_flash_sector_write(&nand_sim_flash, 300, buffer);
    if (status != LX_SUCCESS)
    {
        printf("FAILED!\n");
#ifdef BATCH_TEST
        exit(1);
#endif
        while (1)
        {
        }
    }

    status = lx_nand_flash_sector_write(&nand_sim_flash, 400, buffer);
    if (status != LX_SUCCESS)
    {
        printf("FAILED!\n");
#ifdef BATCH_TEST
        exit(1);
#endif
        while (1)
        {
        }
    }

    if (status != LX_SUCCESS)
    {
        printf("FAILED!\n");
#ifdef BATCH_TEST
        exit(1);
#endif
        while (1)
        {
        }
    }
    if (nand_sim_flash.lx_nand_flash_block_status_table[nand_sim_flash.lx_nand_flash_block_mapping_table[1]] & LX_NAND_BLOCK_STATUS_NON_SEQUENTIAL)
    {
        printf("FAILED!\n");
#ifdef BATCH_TEST
        exit(1);
#endif
        while (1)
        {
        }
    }
    
    status =  lx_nand_flash_close(&nand_sim_flash);
    
    if (status != LX_SUCCESS)
    {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }
    printf("SUCCESS!\n");

    printf("Test 4: Random read/write test..................");

    LX_MEMSET(local_data_buffer, 0xff, sizeof(local_data_buffer));

    /* Reinitialize...  */
    _lx_nand_flash_simulator_erase_all();


    lx_nand_flash_initialize();

    status = lx_nand_flash_format(&nand_sim_flash, "sim nand flash", _lx_nand_flash_simulator_initialize, nand_memory_space, sizeof(nand_memory_space));
    if (status != LX_SUCCESS)
    {
        printf("FAILED!\n");
#ifdef BATCH_TEST
        exit(1);
#endif
        while (1)
        {
        }
    }

    status = lx_nand_flash_open(&nand_sim_flash, "sim nand flash", _lx_nand_flash_simulator_initialize, nand_memory_space, sizeof(nand_memory_space));
    if (status != LX_SUCCESS)
    {
        printf("FAILED!\n");
#ifdef BATCH_TEST
        exit(1);
#endif
        while (1)
        {
        }
    }

    static ULONG loop_count = 200;
    ULONG operation;
    ULONG sector_number;
    ULONG data_byte;
    ULONG sector_count;
    static ULONG read_count = 0;
    static ULONG write_count = 0;
    static ULONG release_count = 0;
    static ULONG reopen_count = 0;

    while (loop_count--)
    {
        operation = (ULONG)rand() % 32768;
        if (operation < 16384)
        {
            sector_number = ((ULONG)rand() * (ULONG)rand()) % MAX_SECTOR_ADDRESS;
            sector_count = ((ULONG)rand() % 768) + 1;
            if (sector_number + sector_count > MAX_SECTOR_ADDRESS)
            {
                sector_count = MAX_SECTOR_ADDRESS - sector_number;
            }

            while (sector_count--)
            {
                status = lx_nand_flash_sector_read(&nand_sim_flash, sector_number, readbuffer);
                if (status != LX_SUCCESS)
                {
                    printf("FAILED!\n");
#ifdef BATCH_TEST
                    exit(1);
#endif
                    while (1)
                    {
                    }
                }
#ifdef FULL_SECTOR_DATA_VERIFY
                status = memcmp((local_data_buffer + sector_number * SECTOR_SIZE), readbuffer, SECTOR_SIZE);
#else
                status = *(ULONG*)(local_data_buffer + sector_number * SECTOR_SIZE) - readbuffer[0];
#endif
                if (status != 0)
                {
                    printf("FAILED!\n");
#ifdef BATCH_TEST
                    exit(1);
#endif
                    while (1)
                    {
                    }
                }
                read_count++;
                sector_number++;
            }
        }
        else if (operation < 29491)
        {
            sector_number = ((ULONG)rand() * (ULONG)rand()) % MAX_SECTOR_ADDRESS;
            sector_count = ((ULONG)rand() % 768) + 1;
            if (sector_number + sector_count > MAX_SECTOR_ADDRESS)
            {
                sector_count = MAX_SECTOR_ADDRESS - sector_number;
            }

            while (sector_count--)
            {
                data_byte = (ULONG)rand();

#ifdef FULL_SECTOR_DATA_VERIFY
                LX_MEMSET((local_data_buffer + sector_number * SECTOR_SIZE), data_byte, SECTOR_SIZE);
#else
                *(ULONG*)(local_data_buffer + sector_number * SECTOR_SIZE) = data_byte;
#endif
                status = lx_nand_flash_sector_write(&nand_sim_flash, sector_number, local_data_buffer + sector_number * SECTOR_SIZE);
                if (status != LX_SUCCESS)
                {
                    printf("FAILED!\n");
#ifdef BATCH_TEST
                    exit(1);
#endif
                    while (1)
                    {
                    }
                }
                write_count++;
                sector_number++;
            }
        }
        else if(operation < 32760)
        {
            sector_number = ((ULONG)rand() * (ULONG)rand()) % MAX_SECTOR_ADDRESS;
            data_byte = 0xffffffff;
            sector_count = ((ULONG)rand() % 768) + 1;
            if (sector_number + sector_count > MAX_SECTOR_ADDRESS)
            {
                sector_count = MAX_SECTOR_ADDRESS - sector_number;
            }

            while (sector_count--)
            {
                
#ifdef FULL_SECTOR_DATA_VERIFY
                LX_MEMSET((local_data_buffer + sector_number * SECTOR_SIZE), data_byte, SECTOR_SIZE);
#else
                * (ULONG*)(local_data_buffer + sector_number * SECTOR_SIZE) = data_byte;
#endif
                status = lx_nand_flash_sector_release(&nand_sim_flash, sector_number);
                if (status != LX_SUCCESS)
                {
                    printf("FAILED!\n");
#ifdef BATCH_TEST
                    exit(1);
#endif
                    while (1)
                    {
                    }
                }
                release_count++;
                sector_number++;
            }
        }
        else
        {
            /* Close and reopen the flash. */
            status = lx_nand_flash_close(&nand_sim_flash);
            if (status != LX_SUCCESS)
            {
                printf("FAILED!\n");
#ifdef BATCH_TEST
                exit(1);
#endif
                while (1)
                {
                }
            }

            status = lx_nand_flash_open(&nand_sim_flash, "sim nor flash", _lx_nand_flash_simulator_initialize, nand_memory_space, sizeof(nand_memory_space));
            if (status != LX_SUCCESS)
            {
                printf("FAILED!\n");
#ifdef BATCH_TEST
                exit(1);
#endif
                while (1)
                {
                }
            }
            reopen_count++;
        }
    }

    printf("SUCCESS!\n");

#if 0
    /* Point at the simulated NOR flash memory.  */
    word_ptr =  nand_flash_memory;
    
   
    /* Test 3: Corrupt block 0, simulate a power interruption during erase of block 0, 
       after the erase, but before the erase count is setup.  */
    printf("Test 3: Block erase-initialize interrupted......");
    word_ptr[0] =  0xFFFFFFFF;
    word_ptr[3] =  0x00000000;  /* This simulates a non-erased block.  */

    /* Open the flash and see if we recover properly.  */    
    status =  lx_nand_flash_open(&nand_sim_flash, "sim nor flash", _lx_nand_flash_simulator_initialize, nand_memory_space, sizeof(nand_memory_space));
#ifdef EXTENDED_CACHE
    status += lx_nand_flash_extended_cache_enable(&nand_sim_flash, cache_memory, sizeof(cache_memory));
#endif
    
    if ((status != LX_SUCCESS) ||
        (nand_sim_flash.lx_nand_flash_free_pages != 111) ||
        (nand_sim_flash.lx_nand_flash_mapped_pages != 9))
    {
      
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }
    
    status =  lx_nand_flash_close(&nand_sim_flash);
    
    if (status != LX_SUCCESS)
    {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }

   
    /* Corrupt block 0, simulate a power interruption after erase of block 0, 
       but before erase count is setup.  */
    word_ptr[0] =  0xFFFFFFFF;

    /* Open the flash and see if we recover properly.  */    
    status =  lx_nand_flash_open(&nand_sim_flash, "sim nor flash", _lx_nand_flash_simulator_initialize, nand_memory_space, sizeof(nand_memory_space));
#ifdef EXTENDED_CACHE
    status += lx_nand_flash_extended_cache_enable(&nand_sim_flash, cache_memory, sizeof(cache_memory));
#endif
    
    if ((status != LX_SUCCESS) ||
        (nand_sim_flash.lx_nand_flash_free_pages != 111) ||
        (nand_sim_flash.lx_nand_flash_mapped_pages != 9))
    {
      
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }

    status =  lx_nand_flash_close(&nand_sim_flash);
    
    if (status != LX_SUCCESS)
    {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }
    printf("SUCCESS!\n");


    /* Test 4: simulate a power interruption while a new page is being setup.  */
    printf("Test 4: Power interrupted new page setup........");
    
    /* Partially setup new page.  */
    byte_ptr =  (UCHAR *) word_ptr;
    byte_ptr[2053] = 0xBF;      /* Set block 0/page 0 extra bytes to indicate block not empty.  */
    word_ptr[528] =  0x60;

    /* Open the flash and see if we recover properly.  We should mark this page as obsolete. */    
    status =  lx_nand_flash_open(&nand_sim_flash, "sim nor flash", _lx_nand_flash_simulator_initialize, nand_memory_space, sizeof(nand_memory_space));
#ifdef EXTENDED_CACHE
    status += lx_nand_flash_extended_cache_enable(&nand_sim_flash, cache_memory, sizeof(cache_memory));
#endif
    
    if ((status != LX_SUCCESS) ||
        (nand_sim_flash.lx_nand_flash_free_pages != 110) ||
        (nand_sim_flash.lx_nand_flash_mapped_pages != 9))
    {
      
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }
    
    status =  lx_nand_flash_close(&nand_sim_flash);
    
    if (status != LX_SUCCESS)
    {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }


    /* Simulate a power interruption after a new sector is allocated, after data
       had been copied, and the superceeded bit is clear, but before the new entry can be
       setup.  */
       
    /* Copy data block and reset spare.  */
    byte_ptr[2053] = 0xBF;          /* Set block 0/page 0 extra bytes to indicate block not empty.  */
    for (i = 1; i < 512; i++)       /* Fill block 0, page 1 data  */
        word_ptr[i+528] = 0x60; 
    word_ptr[i+528] =  0xFFFFFFFF;  /* Reset extra bytes for block 0, page 1 */
    word_ptr[i+529] =  0xFFFFFFFF;  /* Reset extra bytes for block 0, page 1 */ 
    
    /* Set the superceded bit in block 1/page 1.   */
    byte_ptr[37957] = 0x80;
    
    /* Open the flash and see if we recover properly.  */    
    status =  lx_nand_flash_open(&nand_sim_flash, "sim nor flash", _lx_nand_flash_simulator_initialize, nand_memory_space, sizeof(nand_memory_space));
#ifdef EXTENDED_CACHE
    lx_nand_flash_extended_cache_enable(&nand_sim_flash, cache_memory, sizeof(cache_memory));
#endif
    
    if ((status != LX_SUCCESS) ||
        (nand_sim_flash.lx_nand_flash_free_pages != 110) ||
        (nand_sim_flash.lx_nand_flash_mapped_pages != 9))
    {
      
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }

    lx_nand_flash_close(&nand_sim_flash);

    /* Simulate a power interruption after a new sector is allocated, after data
       had been copied, and the superceeded bit is clear, the new entry is setup, but the old entry
       has not been invalidated.  */
    byte_ptr[2053] = 0xBF;          /* Set block 0/page 0 extra bytes to indicate block not empty.  */
    for (i = 1; i < 512; i++)       /* Fill block 0, page 1 data  */
        word_ptr[i+528] = 0x60;     
    word_ptr[i+528] =  0xFFFFFFFF;  /* Reset extra bytes for block 0, page 1 */
    word_ptr[i+529] =  0xFFFFFFFF;  /* Reset extra bytes for block 0, page 1 */
    
    byte_ptr =  (UCHAR *) word_ptr;

    /* set the spare info in the new page.  */
    byte_ptr[4162] = 0x60;          /* Setup extra bytes for block 0, page 1  */
    byte_ptr[4163] = 0;             /* Setup extra bytes for block 0, page 1  */
    byte_ptr[4164] = 0;             /* Setup extra bytes for block 0, page 1  */
    byte_ptr[4165] = 0xC0;          /* Setup extra bytes for block 0, page 1  */

    /* Set the superceded bit in block 1/page 1.   */
    byte_ptr[37957] = 0x80;

    /* Open the flash and see if we recover properly.  */    
    status =  lx_nand_flash_open(&nand_sim_flash, "sim nor flash", _lx_nand_flash_simulator_initialize, nand_memory_space, sizeof(nand_memory_space));
#ifdef EXTENDED_CACHE
    status += lx_nand_flash_extended_cache_enable(&nand_sim_flash, cache_memory, sizeof(cache_memory));
#endif
    
    if ((status != LX_SUCCESS) ||
        (nand_sim_flash.lx_nand_flash_free_pages != 110) ||
        (nand_sim_flash.lx_nand_flash_mapped_pages != 9))
    {
      
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }

    status =  lx_nand_flash_sector_read(&nand_sim_flash, 7, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 119)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 1, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 1)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
     status =  lx_nand_flash_sector_read(&nand_sim_flash, 16, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 16)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 32, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 32)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 48, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 48)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 64, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 64)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 80, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 80)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 96, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 96)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 112, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 112)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }

    status =  lx_nand_flash_defragment(&nand_sim_flash);
    
    if (status != LX_NOT_SUPPORTED)
    {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }
    
   
    status =  lx_nand_flash_defragment(&nand_sim_flash);
    
    if (status != LX_NOT_SUPPORTED)
    {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }
    
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 7, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 119)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 1, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 1)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
     status =  lx_nand_flash_sector_read(&nand_sim_flash, 16, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 16)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 32, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 32)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 48, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 48)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 64, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 64)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 80, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 80)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 96, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 96)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 112, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 112)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }

    /* Read in reverse order to see if caching helps!  */
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 112, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 112)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 96, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 96)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
     status =  lx_nand_flash_sector_read(&nand_sim_flash, 80, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 80)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 64, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 64)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 48, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 48)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 32, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 32)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 16, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 16)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 1, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 1)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_read(&nand_sim_flash, 7, readbuffer);
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        if (readbuffer[0] != 119)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        
    status =  lx_nand_flash_sector_release(&nand_sim_flash, 7);
    if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    status =  lx_nand_flash_sector_release(&nand_sim_flash, 8);
    if (status != LX_SECTOR_NOT_FOUND)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        
    status =  lx_nand_flash_defragment(&nand_sim_flash);
    
    if (status != LX_SUCCESS)
    {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }

    status =  lx_nand_flash_close(&nand_sim_flash);

    if (status != LX_SUCCESS)
    {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }
   
    printf("SUCCESS!\n");
#endif
#if 0
    printf("Test 5: Random write/read sector................");

    /* Erase the simulated NOR flash.  */
    _lx_nand_flash_simulator_erase_all();

    /* Open the flash.  */    
    status =  lx_nand_flash_open(&nand_sim_flash, "sim nor flash", _lx_nand_flash_simulator_initialize, nand_memory_space, sizeof(nand_memory_space));
#ifdef EXTENDED_CACHE
    status += lx_nand_flash_extended_cache_enable(&nand_sim_flash, cache_memory, sizeof(cache_memory));
#endif
    
    if (status != LX_SUCCESS) 
    {
      
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }
    
    /* Write 100 sectors....  */
    for (i = 0; i < 100; i++)
    {
        for (j = 0; j < 512; j++)
          buffer[j] =  i;
        
        status =  lx_nand_flash_sector_write(&nand_sim_flash, i, buffer);
      
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    }
    
    /* Read back 100 sectors...  */
    for (i = 0; i < 100; i++)
    {
        
        status =  lx_nand_flash_sector_read(&nand_sim_flash, i, buffer);
      
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        
        for (j = 0; j < 512; j++)
        {
          
          if (buffer[j] !=  i)
          {
            printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
            while(1)
            {
            }
          }
        }
    }
    
    /* Now, perform 1000 sector writes to randomly selected sectors, each time
       reading first to make sure the previous contents are valid.  */
    for (i = 0; i < 1000; i++)
    {
    
        /* Pickup random sector.  */
        sector =  (rand() % 100);

        /* Read that sector.  */
        status =  lx_nand_flash_sector_read(&nand_sim_flash, sector, buffer);
      
        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
        
        for (j = 0; j < 512; j++)
        {
          
          if ((buffer[j] & 0x0000FFFF) !=  sector)
          {
            printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
            while(1)
            {
            }
          }
        }

        /* Include the itteraction in the buffer to generate a new write.  */
        for (j = 0; j < 512; j++)
        {
 
            buffer[j] =  (buffer[j] & 0x0000FFFF) | (i << 16);
        }
        
        status =  lx_nand_flash_sector_write(&nand_sim_flash, sector, buffer);

        if (status != LX_SUCCESS)
        {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
        }
    }
    
    status =  lx_nand_flash_close(&nand_sim_flash);
    
    if (status != LX_SUCCESS)
    {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }
    printf("SUCCESS!\n");
#endif
    
#if 0
    
  for (i = 0; i < 256; i++)
  {
  
      /* Setup buffer.  */
      for (j = 0; j < 2048; j++)
          byte_buffer[j] =  (UCHAR) (rand() % 256);
      
      /* Call the ECC calculate routine.  */
      ComputePageECC(byte_buffer, 2048, ecc_bytes);
      status =  (UINT) CorrectPageECC(byte_buffer, 2048, ecc_bytes);
      if (status != 0)
      {
        
          /* Check for corrected return value.  */
          if (status != 1)
          {
            while(1)
            {
            }
          }
      }
      
      /* Call LevelX ECC routines to do the same thing.  */
      status =  lx_nand_flash_page_ecc_compute(&nand_sim_flash, byte_buffer, lx_ecc_buffer);

      if (status != LX_SUCCESS)
      {
          printf("FAILED!\n");
          while(1)
          {
          }
      }

//      ComputePageECC(byte_buffer, 2048, ecc_bytes);

      
      /* Determine if there is any difference between the original and the new ECC routine.  */
      if ((ecc_bytes[0] != lx_ecc_buffer[0]) || 
          (ecc_bytes[1] != lx_ecc_buffer[1]) ||
          (ecc_bytes[2] != lx_ecc_buffer[2]) ||
          (ecc_bytes[3] != lx_ecc_buffer[3]) ||
          (ecc_bytes[4] != lx_ecc_buffer[4]) ||
          (ecc_bytes[5] != lx_ecc_buffer[5]) ||
          (ecc_bytes[6] != lx_ecc_buffer[6]) ||
          (ecc_bytes[7] != lx_ecc_buffer[7]) ||  
          (ecc_bytes[8] != lx_ecc_buffer[8]) ||  
          (ecc_bytes[9] != lx_ecc_buffer[9]) ||
          (ecc_bytes[10] != lx_ecc_buffer[10]) ||
          (ecc_bytes[11] != lx_ecc_buffer[11]))
      {
         while(1)
         {
         }
      }

      status =  lx_nand_flash_page_ecc_check(&nand_sim_flash, byte_buffer, lx_ecc_buffer);

      if (status != LX_SUCCESS)
      {
        
          /* Is it the corrected status?  */
          if (status !=  LX_NAND_ERROR_CORRECTED)
          {
            printf("FAILED!\n");
            while(1)
            {
            }
          }
      }
  }
#endif

#if 0  
  for (i = 0; i < 256; i++)
  {
  
      /* Setup buffer.  */
      for (j = 0; j < 2048; j++)
          byte_buffer[j] =  (UCHAR) (rand() % 256);
      
      /* Call the ECC calculate routine.  */
      ComputePageECC(byte_buffer, 2048, ecc_bytes);
      
      /* Corrupt a bit in each 256 byte page.  */
      if (byte_buffer[7] & 1)
          byte_buffer[7] =  byte_buffer[7] & 0xFE;
      else
          byte_buffer[7] =  byte_buffer[7] | 1;
 
      if (byte_buffer[277] & 1)
          byte_buffer[277] =  byte_buffer[277] & 0xFE;
      else
          byte_buffer[277] =  byte_buffer[277] | 1;

      if (byte_buffer[577] & 1)
          byte_buffer[577] =  byte_buffer[577] & 0xFE;
      else
          byte_buffer[577] =  byte_buffer[577] | 1;
      
      if (byte_buffer[777] & 1)
          byte_buffer[777] =  byte_buffer[777] & 0xFE;
      else
          byte_buffer[777] =  byte_buffer[777] | 1;
      
      if (byte_buffer[1077] & 1)
          byte_buffer[1077] =  byte_buffer[1077] & 0xFE;
      else
          byte_buffer[1077] =  byte_buffer[1077] | 1;

      if (byte_buffer[1297] & 1)
          byte_buffer[1297] =  byte_buffer[1297] & 0xFE;
      else
          byte_buffer[1297] =  byte_buffer[1297] | 1;
      
      if (byte_buffer[1636] & 1)
          byte_buffer[1636] =  byte_buffer[1636] & 0xFE;
      else
          byte_buffer[1636] =  byte_buffer[1636] | 1;

      if (byte_buffer[1892] & 1)
          byte_buffer[1892] =  byte_buffer[1892] & 0xFE;
      else
          byte_buffer[1892] =  byte_buffer[1892] | 1;
     
      
      status =  (UINT) CorrectPageECC(byte_buffer, 2048, ecc_bytes);
      if (status != 0)
      {
        
          /* Check for corrected return value.  */
          if (status != 1)
          {
            while(1)
            {
            }
          }
      }
      
      /* Call LevelX ECC routines to do the same thing.  */
      status =  lx_nand_flash_page_ecc_compute(&nand_sim_flash, byte_buffer, lx_ecc_buffer);

      if (status != LX_SUCCESS)
      {
          printf("FAILED!\n");
          while(1)
          {
          }
      }


      
      /* Determine if there is any difference between the original and the new ECC routine.  */
      if ((ecc_bytes[0] != lx_ecc_buffer[0]) || 
          (ecc_bytes[1] != lx_ecc_buffer[1]) ||
          (ecc_bytes[2] != lx_ecc_buffer[2]) ||
          (ecc_bytes[3] != lx_ecc_buffer[3]) ||
          (ecc_bytes[4] != lx_ecc_buffer[4]) ||
          (ecc_bytes[5] != lx_ecc_buffer[5]) ||
          (ecc_bytes[6] != lx_ecc_buffer[6]) ||
          (ecc_bytes[7] != lx_ecc_buffer[7]) ||  
          (ecc_bytes[8] != lx_ecc_buffer[8]) ||  
          (ecc_bytes[9] != lx_ecc_buffer[9]) ||
          (ecc_bytes[10] != lx_ecc_buffer[10]) ||
          (ecc_bytes[11] != lx_ecc_buffer[11]))
      {
         while(1)
         {
         }
      }

      /* Corrupt a bit in each 256 byte page.  */
      if (byte_buffer[7] & 1)
          byte_buffer[7] =  byte_buffer[7] & 0xFE;
      else
          byte_buffer[7] =  byte_buffer[7] | 1;
 
      if (byte_buffer[277] & 1)
          byte_buffer[277] =  byte_buffer[277] & 0xFE;
      else
          byte_buffer[277] =  byte_buffer[277] | 1;

      if (byte_buffer[577] & 1)
          byte_buffer[577] =  byte_buffer[577] & 0xFE;
      else
          byte_buffer[577] =  byte_buffer[577] | 1;
      
      if (byte_buffer[777] & 1)
          byte_buffer[777] =  byte_buffer[777] & 0xFE;
      else
          byte_buffer[777] =  byte_buffer[777] | 1;
      
      if (byte_buffer[1077] & 1)
          byte_buffer[1077] =  byte_buffer[1077] & 0xFE;
      else
          byte_buffer[1077] =  byte_buffer[1077] | 1;

      if (byte_buffer[1297] & 1)
          byte_buffer[1297] =  byte_buffer[1297] & 0xFE;
      else
          byte_buffer[1297] =  byte_buffer[1297] | 1;
      
      if (byte_buffer[1636] & 1)
          byte_buffer[1636] =  byte_buffer[1636] & 0xFE;
      else
          byte_buffer[1636] =  byte_buffer[1636] | 1;

      if (byte_buffer[1892] & 1)
          byte_buffer[1892] =  byte_buffer[1892] & 0xFE;
      else
          byte_buffer[1892] =  byte_buffer[1892] | 1;
      
      status =  lx_nand_flash_page_ecc_check(&nand_sim_flash, byte_buffer, lx_ecc_buffer);

      if (status != LX_SUCCESS)
      {
        
          /* Is it the corrected status?  */
          if (status !=  LX_NAND_ERROR_CORRECTED)
          {
            printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
            while(1)
            {
            }
          }
      }

      /* Call LevelX ECC routines to create the ECC over the corrected buffer.  */
      status =  lx_nand_flash_page_ecc_compute(&nand_sim_flash, byte_buffer, lx_ecc_buffer);

      if (status != LX_SUCCESS)
      {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
      }

  }



#endif
    
#if 0  

  /* Corrupting ECC code is detected and returns a failure... this test is not valid!  */

  for (i = 0; i < 256; i++)
  {
  
      /* Setup buffer.  */
      for (j = 0; j < 2048; j++)
          byte_buffer[j] =  (UCHAR) (rand() % 256);
      
      /* Call the ECC calculate routine.  */
      ComputePageECC(byte_buffer, 2048, ecc_bytes);
      
      
      /* Call LevelX ECC routines to do the same thing.  */
      status =  lx_nand_flash_page_ecc_compute(&nand_sim_flash, byte_buffer, lx_ecc_buffer);

      if (status != LX_SUCCESS)
      {
          printf("FAILED!\n");
          while(1)
          {
          }
      }

      /* Determine if there is any difference between the original and the new ECC routine.  */
      if ((ecc_bytes[0] != lx_ecc_buffer[0]) || 
          (ecc_bytes[1] != lx_ecc_buffer[1]) ||
          (ecc_bytes[2] != lx_ecc_buffer[2]) ||
          (ecc_bytes[3] != lx_ecc_buffer[3]) ||
          (ecc_bytes[4] != lx_ecc_buffer[4]) ||
          (ecc_bytes[5] != lx_ecc_buffer[5]) ||
          (ecc_bytes[6] != lx_ecc_buffer[6]) ||
          (ecc_bytes[7] != lx_ecc_buffer[7]) ||  
          (ecc_bytes[8] != lx_ecc_buffer[8]) ||  
          (ecc_bytes[9] != lx_ecc_buffer[9]) ||
          (ecc_bytes[10] != lx_ecc_buffer[10]) ||
          (ecc_bytes[11] != lx_ecc_buffer[11]))
      {
         while(1)
         {
         }
      }

      /* Corrupt a bit in each ECC for 256 byte page.  */
      if (ecc_bytes[1] & 1)
          ecc_bytes[1] =  ecc_bytes[1] & 0xFE;
      else
          ecc_bytes[1] =  ecc_bytes[1] | 1;
 
      if (ecc_bytes[4] & 1)
          ecc_bytes[4] =  ecc_bytes[4] & 0xFE;
      else
          ecc_bytes[4] =  ecc_bytes[4] | 1;

      if (ecc_bytes[7] & 1)
          ecc_bytes[7] =  ecc_bytes[7] & 0xFE;
      else
          ecc_bytes[7] =  ecc_bytes[7] | 1;
      
      if (ecc_bytes[10] & 1)
          ecc_bytes[10] =  ecc_bytes[10] & 0xFE;
      else
          ecc_bytes[10] =  ecc_bytes[10] | 1;
      
      if (ecc_bytes[13] & 1)
          ecc_bytes[13] =  ecc_bytes[13] & 0xFE;
      else
          ecc_bytes[13] =  ecc_bytes[13] | 1;

      if (ecc_bytes[16] & 1)
          ecc_bytes[16] =  ecc_bytes[16] & 0xFE;
      else
          ecc_bytes[16] =  ecc_bytes[16] | 1;
      
      if (ecc_bytes[19] & 1)
          ecc_bytes[19] =  ecc_bytes[19] & 0xFE;
      else
          ecc_bytes[19] =  ecc_bytes[19] | 1;

      if (ecc_bytes[22] & 1)
          ecc_bytes[22] =  ecc_bytes[22] & 0xFE;
      else
          ecc_bytes[22] =  ecc_bytes[22] | 1;
     
      
      status =  (UINT) CorrectPageECC(byte_buffer, 2048, ecc_bytes);
      if (status != 0)
      {
        
          /* Check for corrected return value.  */
          if (status != 1)
          {
            while(1)
            {
            }
          }
      }

      /* Corrupt a bit in each ECC for 256 byte page.  */
      if (lx_ecc_buffer[1] & 1)
          lx_ecc_buffer[1] =  lx_ecc_buffer[1] & 0xFE;
      else
          lx_ecc_buffer[1] =  lx_ecc_buffer[1] | 1;
 
      if (lx_ecc_buffer[4] & 1)
          lx_ecc_buffer[4] =  lx_ecc_buffer[4] & 0xFE;
      else
          lx_ecc_buffer[4] =  lx_ecc_buffer[4] | 1;

      if (lx_ecc_buffer[7] & 1)
          lx_ecc_buffer[7] =  lx_ecc_buffer[7] & 0xFE;
      else
          lx_ecc_buffer[7] =  lx_ecc_buffer[7] | 1;
      
      if (lx_ecc_buffer[10] & 1)
          lx_ecc_buffer[10] =  lx_ecc_buffer[10] & 0xFE;
      else
          lx_ecc_buffer[10] =  lx_ecc_buffer[10] | 1;
      
      if (lx_ecc_buffer[13] & 1)
          lx_ecc_buffer[13] =  lx_ecc_buffer[13] & 0xFE;
      else
          lx_ecc_buffer[13] =  lx_ecc_buffer[13] | 1;

      if (lx_ecc_buffer[16] & 1)
          lx_ecc_buffer[16] =  lx_ecc_buffer[16] & 0xFE;
      else
          lx_ecc_buffer[16] =  lx_ecc_buffer[16] | 1;
      
      if (lx_ecc_buffer[19] & 1)
          lx_ecc_buffer[19] =  lx_ecc_buffer[19] & 0xFE;
      else
          lx_ecc_buffer[19] =  lx_ecc_buffer[19] | 1;

      if (lx_ecc_buffer[22] & 1)
          lx_ecc_buffer[22] =  lx_ecc_buffer[22] & 0xFE;
      else
          lx_ecc_buffer[22] =  lx_ecc_buffer[22] | 1;
     

      
      status =  lx_nand_flash_page_ecc_check(&nand_sim_flash, byte_buffer, lx_ecc_buffer);

      if (status != LX_SUCCESS)
      {
        
          /* Is it the corrected status?  */
          if (status !=  LX_NAND_ERROR_CORRECTED)
          {
            printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
            while(1)
            {
            }
          }
      }

      /* Call LevelX ECC routines to create the ECC over the corrected buffer.  */
      status =  lx_nand_flash_page_ecc_compute(&nand_sim_flash, byte_buffer, lx_ecc_buffer);

      if (status != LX_SUCCESS)
      {
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
      }

  }



#endif
 
  
    
#if 0
/* TEST FileX packing routines.  */

    /* Erase the simulated NAND flash.  */
    _lx_nand_flash_simulator_erase_all();

    /* Open the flash.  */    
    status =  lx_nand_flash_open(&nand_sim_flash, "sim nor flash", _lx_nand_flash_simulator_initialize);
#ifdef EXTENDED_CACHE
    status += lx_nand_flash_extended_cache_enable(&nand_sim_flash, cache_memory, sizeof(cache_memory));
#endif
    
    if (status != LX_SUCCESS) 
    {
      
          printf("FAILED!\n");
          while(1)
          {
          }
    }

  /* Setup the write data.  */
  for (i = 0; i < 8; i++)
  {
      for (j = 0; j < 512/4; j++)
        test_write[i].words[j] = i;
  }

  /* Now write the data one logical sector at a time.  */
  for (i = 0; i < 8; i++)
  {
       _fx_nand_flash_write_sectors(i, 1, (UCHAR *) &test_write[i]);
  }

  /* Setup the read buffer.  */
  for (i = 0; i < 8; i++)
  {
      for (j = 0; j < 512/4; j++)
        test_read[i].words[j] = 0xEFEFEFEF;
  }

  /* Read back in one sector at a time.  */
  for (i = 0; i < 8; i++)
  {
       _fx_nand_flash_read_sectors(i, 1, (UCHAR *) &test_read[i].words[0]);
  }
  
  /* Setup the read buffer.  */
  for (i = 0; i < 8; i++)
  {
      for (j = 0; j < 512/4; j++)
        test_read[i].words[j] = 0xEFEFEFEF;
  }

  _fx_nand_flash_read_sectors(3, 5, (UCHAR *) &test_read[3].words[0]);
  _fx_nand_flash_read_sectors(1, 2, (UCHAR *) &test_read[1].words[0]);
  _fx_nand_flash_read_sectors(0, 1, (UCHAR *) &test_read[0].words[0]);

  /* Setup the read buffer.  */
  for (i = 0; i < 8; i++)
  {
      for (j = 0; j < 512/4; j++)
        test_read[i].words[j] = 0xEFEFEFEF;
  }

  _fx_nand_flash_read_sectors(0, 8, (UCHAR *) &test_read[0].words[0]);

  
    status =  lx_nand_flash_close(&nand_sim_flash);
    
    if (status != LX_SUCCESS)
    {
          printf("FAILED!\n");
          while(1)
          {
          }
    }

    /* Erase the simulated NAND flash.  */
    _lx_nand_flash_simulator_erase_all();

    /* Open the flash.  */    
    status =  lx_nand_flash_open(&nand_sim_flash, "sim nor flash", _lx_nand_flash_simulator_initialize);
#ifdef EXTENDED_CACHE
    status += lx_nand_flash_extended_cache_enable(&nand_sim_flash, cache_memory, sizeof(cache_memory));
#endif
    
    if (status != LX_SUCCESS) 
    {
      
          printf("FAILED!\n");
          while(1)
          {
          }
    }

    _fx_nand_flash_write_sectors(3, 5, (UCHAR *) &test_read[3].words[0]);
    _fx_nand_flash_write_sectors(1, 2, (UCHAR *) &test_read[1].words[0]);
    _fx_nand_flash_write_sectors(0, 1, (UCHAR *) &test_read[0].words[0]);

    /* Setup the read buffer.  */
    for (i = 0; i < 8; i++)
    {
      for (j = 0; j < 512/4; j++)
          test_read[i].words[j] = 0xEFEFEFEF;
    }

    _fx_nand_flash_read_sectors(0, 1, (UCHAR *) &test_read[0].words[0]);
    _fx_nand_flash_read_sectors(1, 2, (UCHAR *) &test_read[1].words[0]);
    _fx_nand_flash_read_sectors(3, 5, (UCHAR *) &test_read[3].words[0]);

    status =  lx_nand_flash_close(&nand_sim_flash);
    
    if (status != LX_SUCCESS)
    {
          printf("FAILED!\n");
          while(1)
          {
          }
    }

  /* End */    
#endif    
    
#ifdef BATCH_TEST
    exit(0);
#endif
     /* All done!  */
     while(1)
     {
     }
}


