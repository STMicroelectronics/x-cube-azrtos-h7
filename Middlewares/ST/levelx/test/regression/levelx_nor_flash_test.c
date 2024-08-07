/* Basic NOR flash tests...  */

#include <stdio.h>
#include "lx_api.h"

#define     DEMO_STACK_SIZE         4096


/* Define the ThreadX object control blocks...  */
#ifndef LX_STANDALONE_ENABLE
TX_THREAD               thread_0;
#endif
UCHAR                   thread_0_stack[DEMO_STACK_SIZE];

/* Define LevelX structures.  */

LX_NOR_FLASH    nor_sim_flash;
ULONG           buffer[128];
ULONG           readbuffer[128];


/* Define LevelX NOR flash simulator prototoypes.  */

UINT  _lx_nor_flash_simulator_erase_all(VOID);
UINT  _lx_nor_flash_simulator_initialize(LX_NOR_FLASH *nor_flash);



/* Define thread prototypes.  */

void    thread_0_entry(ULONG thread_input);



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

/* Define the test threads.  */

void    thread_0_entry(ULONG thread_input)
{

ULONG   i, j, sector;
UINT    status;

ULONG   *word_ptr;

  
    /* Erase the simulated NOR flash.  */
    _lx_nor_flash_simulator_erase_all();
    
    /* Initialize LevelX.  */
    _lx_nor_flash_initialize();
    
    /* Test 1: Simple write 100 sectors and read 100 sectors.  */
    printf("Test 1: Simple write-read 100 sectors...........");
    
    lx_nor_flash_open(&nor_sim_flash, "sim nor flash", _lx_nor_flash_simulator_initialize);
    
    /* Write 100 sectors....  */
    for (i = 0; i < 100; i++)
    {
        for (j = 0; j < 128; j++)
          buffer[j] =  i;
        
        status =  lx_nor_flash_sector_write(&nor_sim_flash, i, buffer);
      
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
        
        status =  lx_nor_flash_sector_read(&nor_sim_flash, i, buffer);
      
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

    _lx_nor_flash_close(&nor_sim_flash);
    printf("SUCCESS!\n");

    /* Test 2: Write same sector 120 times.  */
    printf("Test 2: Write same sector 120 times.............");
    
    /* Reinitialize...  */    
    _lx_nor_flash_simulator_erase_all();
    
    
    lx_nor_flash_initialize();
    lx_nor_flash_open(&nor_sim_flash, "sim nor flash", _lx_nor_flash_simulator_initialize);

    for (j = 0; j < 128; j++)
         buffer[j] =  0xFFFFFFFF;
    
    /* Write same sector 120 sectors....  */
    for (i = 0; i < 120; i++)
    {
        for (j = 0; j < 128; j++)
          buffer[j] =  i;
      
        status =  lx_nor_flash_sector_write(&nor_sim_flash, 7, buffer);
      
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
        
        status =  lx_nor_flash_sector_read(&nor_sim_flash, 7, readbuffer);
      
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
          lx_nor_flash_sector_write(&nor_sim_flash, 1, buffer);
        if (i == 16)
          lx_nor_flash_sector_write(&nor_sim_flash, 16, buffer);
        if (i == 32)
          lx_nor_flash_sector_write(&nor_sim_flash, 32, buffer);
        if (i == 48)
          lx_nor_flash_sector_write(&nor_sim_flash, 48, buffer);
        if (i == 64)
          lx_nor_flash_sector_write(&nor_sim_flash, 64, buffer);
        if (i == 80)
          lx_nor_flash_sector_write(&nor_sim_flash, 80, buffer);
        if (i == 96)
          lx_nor_flash_sector_write(&nor_sim_flash, 96, buffer);
        if (i == 112)
          lx_nor_flash_sector_write(&nor_sim_flash, 112, buffer);
    }

    status =  lx_nor_flash_defragment(&nor_sim_flash);

    
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 7, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 1, readbuffer);
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
     status =  lx_nor_flash_sector_read(&nor_sim_flash, 16, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 32, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 48, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 64, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 80, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 96, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 112, readbuffer);
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

    status =  lx_nor_flash_defragment(&nor_sim_flash);
    
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
   
    
    /* Point at the simulated NOR flash memory.  */
    word_ptr =  nor_sim_flash.lx_nor_flash_base_address;
    
    status =  lx_nor_flash_close(&nor_sim_flash);
    
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
   
    /* Test 3: Corrupt block 0, simulate a power interruption during erase of block 0, 
       after the erase, but before the free bit map and erase count is setup.  */
    printf("Test 3: Block erase-initialize interrupted......");
    word_ptr[0] =  0xFFFFFFFF;
    word_ptr[3] =  0xFFFFFFFF;

    /* Open the flash and see if we recover properly.  */    
    status =  lx_nor_flash_open(&nor_sim_flash, "sim nor flash", _lx_nor_flash_simulator_initialize);

    if ((status != LX_SUCCESS) ||
        (nor_sim_flash.lx_nor_flash_free_physical_sectors != 111) ||
        (nor_sim_flash.lx_nor_flash_mapped_physical_sectors != 9))
    {
      
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }
    
    status =  lx_nor_flash_close(&nor_sim_flash);
    
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
   
    /* Corrupt block 0, simulate a power interruption during erase of block 0, 
       after the erase, and after the free bit map setup, but before erase count is setup.  */
    word_ptr[0] =  0xFFFFFFFF;

    /* Open the flash and see if we recover properly.  */    
    status =  lx_nor_flash_open(&nor_sim_flash, "sim nor flash", _lx_nor_flash_simulator_initialize);

    if ((status != LX_SUCCESS) ||
        (nor_sim_flash.lx_nor_flash_free_physical_sectors != 111) ||
        (nor_sim_flash.lx_nor_flash_mapped_physical_sectors != 9))
    {
      
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }

    status =  lx_nor_flash_close(&nor_sim_flash);
    
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

    /* Test 4: simulate a power interruption after a new block is allocated but before
       anything else can be done.  */
    printf("Test 4: Power interrupted new block allocation..");
    word_ptr[3] =  word_ptr[3] & ~((ULONG) 1);

    /* Open the flash and see if we recover properly.  */    
    status =  lx_nor_flash_open(&nor_sim_flash, "sim nor flash", _lx_nor_flash_simulator_initialize);

    if ((status != LX_SUCCESS) ||
        (nor_sim_flash.lx_nor_flash_free_physical_sectors != 110) ||
        (nor_sim_flash.lx_nor_flash_mapped_physical_sectors != 9))
    {
      
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }
    
    status =  lx_nor_flash_close(&nor_sim_flash);
    
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

    /* Simulate a power interruption after a new block is allocated but before
       anything else can be done.  */
    word_ptr[(16*128)+3] =  0x7C00;

    /* Open the flash and see if we recover properly.  */    
    status =  lx_nor_flash_open(&nor_sim_flash, "sim nor flash", _lx_nor_flash_simulator_initialize);

    if ((status != LX_SUCCESS) ||
        (nor_sim_flash.lx_nor_flash_free_physical_sectors != 109) ||
        (nor_sim_flash.lx_nor_flash_mapped_physical_sectors != 9))
    {
      
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }

    status =  lx_nor_flash_close(&nor_sim_flash);
    
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
    word_ptr[3] =  0x7FFC;
    word_ptr[(16*128)+6] =  word_ptr[(16*128)+6] & ~((ULONG) 0x40000000);
    
    /* Open the flash and see if we recover properly.  */    
    status =  lx_nor_flash_open(&nor_sim_flash, "sim nor flash", _lx_nor_flash_simulator_initialize);

    if ((status != LX_SUCCESS) ||
        (nor_sim_flash.lx_nor_flash_free_physical_sectors != 108) ||
        (nor_sim_flash.lx_nor_flash_mapped_physical_sectors != 9))
    {
      
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }

    lx_nor_flash_close(&nor_sim_flash);
    
    /* Simulate a power interruption after a new sector is allocated, after data
       had been copied, and the superceeded bit is clear, the new entry is setup, but the old entry
       has not been invalidated.  */
    word_ptr[3] =  0x7FF8;
    word_ptr[6] =  0xC0000070;
    for (i = 0; i < 128; i++)
    {
        word_ptr[(3*128)+i] =  0x70;
    }
    
    /* Open the flash and see if we recover properly.  */    
    status =  lx_nor_flash_open(&nor_sim_flash, "sim nor flash", _lx_nor_flash_simulator_initialize);

    if ((status != LX_SUCCESS) ||
        (nor_sim_flash.lx_nor_flash_free_physical_sectors != 107) ||
        (nor_sim_flash.lx_nor_flash_mapped_physical_sectors != 9))
    {
      
          printf("FAILED!\n");
#ifdef BATCH_TEST
    exit(1);
#endif
          while(1)
          {
          }
    }

    status =  lx_nor_flash_sector_read(&nor_sim_flash, 7, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 1, readbuffer);
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
     status =  lx_nor_flash_sector_read(&nor_sim_flash, 16, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 32, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 48, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 64, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 80, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 96, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 112, readbuffer);
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

    status =  lx_nor_flash_defragment(&nor_sim_flash);
    
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
    
   
    status =  lx_nor_flash_defragment(&nor_sim_flash);
    
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
    
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 7, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 1, readbuffer);
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
     status =  lx_nor_flash_sector_read(&nor_sim_flash, 16, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 32, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 48, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 64, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 80, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 96, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 112, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 112, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 96, readbuffer);
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
     status =  lx_nor_flash_sector_read(&nor_sim_flash, 80, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 64, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 48, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 32, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 16, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 1, readbuffer);
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
    status =  lx_nor_flash_sector_read(&nor_sim_flash, 7, readbuffer);
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
        
    status =  lx_nor_flash_sector_release(&nor_sim_flash, 7);
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
    status =  lx_nor_flash_sector_release(&nor_sim_flash, 8);
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
        
    status =  lx_nor_flash_defragment(&nor_sim_flash);
    
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

    status =  lx_nor_flash_close(&nor_sim_flash);

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

    printf("Test 5: Randow write/read sector................");

    /* Erase the simulated NOR flash.  */
    _lx_nor_flash_simulator_erase_all();

    /* Open the flash.  */    
    status =  lx_nor_flash_open(&nor_sim_flash, "sim nor flash", _lx_nor_flash_simulator_initialize);

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
        for (j = 0; j < 128; j++)
          buffer[j] =  i;
        
        status =  lx_nor_flash_sector_write(&nor_sim_flash, i, buffer);
      
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
        
        status =  lx_nor_flash_sector_read(&nor_sim_flash, i, buffer);
      
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
        status =  lx_nor_flash_sector_read(&nor_sim_flash, sector, buffer);
      
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
        for (j = 0; j < 128; j++)
        {
 
            buffer[j] =  (buffer[j] & 0x0000FFFF) | (i << 16);
        }
        
        status =  lx_nor_flash_sector_write(&nor_sim_flash, sector, buffer);

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
    
    status =  lx_nor_flash_close(&nor_sim_flash);
    
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
#ifdef BATCH_TEST
    exit(0);
#endif

     /* All done!  */
     while(1)
     {
     }
}


