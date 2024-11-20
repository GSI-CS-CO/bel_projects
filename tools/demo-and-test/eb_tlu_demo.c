//
// eb_tlu_demo_v2: demo for using the TLU wishbone device via the native Etherbone library
//
// Dietrich Beck, GSI, 2013: derived from a etherbone demo program by Wesley Terpstra
// Pritesh Gudge, 2014: adapted to release "Asterisk"
// 

//standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

//Etherbone
#include <etherbone.h>

//Wishbone devices
#include <wb_slaves.h>

static const char* program;

//Stores reverse 'base'ary Representation of number 'n' to the String Buffer 's'
void itoa(unsigned int n, char s[], int base){
     int i;
 
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % base + '0';   /* get next digit */
     } while ((n /= base) > 0);     /* delete it */
     s[i] = '\0';
 }
//Auxillary Function
void generateChannelList(int nChannels,char buff[])
{
	unsigned int i =0;
	if(buff == NULL)
		return;
	for(i =0; i < 32 && i < nChannels; i++)
	{
		buff[i] = '0' + i;
	}
	buff[i] = '\0';
}


void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
    program, where, eb_status(status));
  exit(1);
}

int main(int argc, const char** argv) {
  eb_status_t status;
  eb_device_t device;
  eb_socket_t socket;
  eb_cycle_t  cycle;
  struct sdb_device sdbDevice;
  eb_address_t wrTLU;
  int nDevices;
  eb_data_t   data, data1, data2,data3;

  const char* devName;
  int channelSelect;
  int nIterations;
  int nLatches = 0;
  char time[60];
  char buff[33];
  char channelStr[33];
  unsigned long long timestamp = 0;
  unsigned long long timestamp_start = 0;
  unsigned long long timestamp_stop  = 0;
  unsigned long long actu_timestamp = 0;
  unsigned long long prev_timestamp = 0;
  unsigned long long diff_timestamp = 0;
  unsigned long long clockFreq = 125000000;
  unsigned long long count_prev = 0;
  unsigned long long count_final = 0;
  unsigned long long temp = 0;
  unsigned int       elapsed_time = 0;
  double             timePerEBCycle;
  char bufft[64+1];
  
  
  time_t secs;
  const struct tm* tm;
  
  long l_i;

  
  program = argv[0];
  if (argc < 4) {
    fprintf(stderr, "reads value of timestamp from time stamp latch unit\n");
    fprintf(stderr, "Syntax: %s <protocol/host/port> <nIterations> <Channel(0..n)>\n", argv[0]);
    return 1;
  }
  
  devName     	 = argv[1];
  channelSelect  = strtol(argv[3],0,10);
  nIterations 	 = strtol(argv[2],0,10);

  /* Open a socket supporting only 32-bit operations.
   * As we are not exporting any slaves, we don't care what port we get => 0.
   * This function always returns immediately.
   * EB_ABI_CODE helps detect if the application matches the library.
   */
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDR32|EB_DATA32, &socket)) != EB_OK)
    die("eb_socket_open", status);
  
  /* Open the remote device with 3 attempts to negotiate bus width.
   * This function is blocking and may stall the thread for up to 3 seconds.
   * If you need asynchronous open, see eb_device_open_nb.
   * Note: the supported widths can never be more than the socket supports.
   */
  if ((status = eb_device_open(socket, devName, EB_ADDR32|EB_DATA32, 3, &device)) != EB_OK)
    die("eb_device_open", status);
  
  /* Find a TLU device on the remote Wishbone bus using the SDB records.
   * Blocking call; use eb_sdb_scan_* for asynchronous access to full SDB table.
   * Increase sdbDevice and initial nDevices value to support multiple results.
   * nDevices reports the number of devices found (potentially more than fit).
   */
  nDevices = 1;
  if ((status = eb_sdb_find_by_identity(device, GSI_TM_LATCH_VENDOR, GSI_TM_LATCH_PRODUCT, &sdbDevice, &nDevices)) != EB_OK)
    die("TLU eb_sdb_find_by_identity", status);
  
  if (nDevices == 0)
    die("no TLU found", EB_FAIL);
  if (nDevices > 1)
    die("too many TLUsfound", EB_FAIL);
  
  /* Record the address of the device */
  wrTLU     = sdbDevice.sdb_component.addr_first;
 
  /* Read actual timestamp from TLU generator.
   * Passing "eb_block" for the asynchronous callback makes the call blocking.
   * Upon return, the operation has been completed and data is valid.
   * This function also packs only a single operation into a cycle (and packet)!
   * For high-throughput applications, pack multiple operations per cycle and
   * use asynchronous notification to support pipelining multiple cycles.
   */
  if ((status = eb_device_read(device, wrTLU + GSI_TM_LATCH_ATSHI, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block)) != EB_OK)
    die("TLU eb_device_read", status);
  timestamp = data;
  timestamp = (timestamp << 32);
  
  if ((status = eb_device_read(device, wrTLU + GSI_TM_LATCH_ATSLO, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block)) != EB_OK)
    die("TLU eb_device_read", status);
  timestamp = timestamp + data;
  printf("Current TAI in cycles: %llu\n",timestamp);
  
  /* Format and print date and time */
  secs = timestamp / clockFreq;
  tm = localtime(&secs);
  strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S %Z", tm);
  printf("Current TAI          : %s\n", time);

  /* remember timestamp for performance measurement */
  timestamp_start = timestamp;


 /* Number of Channels on device */
 //Read Number of Channels  
 if ((status = eb_device_read(device, wrTLU + GSI_TM_LATCH_CHNS_TOTAL, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block)) != EB_OK)
 		die("TLU eb_device_read", status);
 	temp = data;
  printf("Number of Channels %lld\n",temp);
  generateChannelList(temp,channelStr);
  


 /* SIZE of FIFO Qs on device */
 //Read CHANNEL FIFO DEPTH   
 if ((status = eb_device_read(device, wrTLU + GSI_TM_LATCH_CHNS_FIFOSIZE, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block)) != EB_OK)
 		die("TLU eb_device_read", status);
 	temp = data;
  printf("Size of FIFO Queues %lld\n",temp);


  /* clear all FIFOs */
 //if ((status = eb_device_write(device, wrTLU + GSI_TM_LATCH_FIFO_CLEAR, EB_BIG_ENDIAN|EB_DATA32, 0xFFFFFFFF, 0, eb_block)) != EB_OK)
   // die("TLU eb_device_write", status);

    
 /* prepare the TLU for latching of timestamps */
  /* arm triggers for latching */
  if ((status = eb_device_write(device, wrTLU + GSI_TM_LATCH_TRIG_ARMSET, EB_BIG_ENDIAN|EB_DATA32, 0xFFFFFFFF, 0, eb_block)) != EB_OK)
    die("TLU eb_device_write", status);

    // Sleep Required only for Ultra-Low frequencies
	usleep(1000);

 /* Channels containing timestamp */
  if ((status = eb_device_read(device, wrTLU + GSI_TM_LATCH_FIFO_READY, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block)) != EB_OK)
		die("TLU eb_device_read", status);
	temp = data;
  itoa(temp,bufft,2);
     
  printf("Channels                         :%s\n",channelStr);
  printf("Channels containing Timestamp    :%s\n",bufft);


  /* Select Channel to perform FIFO Related Operations*/
  if ((status = eb_device_write(device, wrTLU + GSI_TM_LATCH_CH_SELECT, EB_BIG_ENDIAN|EB_DATA32, channelSelect, 0, eb_block)) != EB_OK)
    die("TLU eb_device_write", status);
    
  /* Read Channel Select Value */
  if ((status = eb_device_read(device, wrTLU + GSI_TM_LATCH_CH_SELECT, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block)) != EB_OK)
		die("TLU eb_device_read", status);
	temp = data;
	channelSelect = temp;
  printf("Channel Selected is %lld\n\n",temp);

 /* Generate Test Pulse on  the selected channel */
 // if ((status = eb_device_write(device, wrTLU + GSI_TM_LATCH_FIFO_TEST, EB_BIG_ENDIAN|EB_DATA32, 0xF,0,eb_block)) != EB_OK)
 //    die("TLU eb_device_write", status);



  /* Channel FIFO Length Before Read Operation */
  if ((status = eb_device_read(device, wrTLU + GSI_TM_LATCH_FIFO_CNT, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block)) != EB_OK)
		die("TLU eb_device_read", status);
  count_prev = data;
 
    
 


////////////////////////////////////////////////////////////////////
  /* Read data from FIFO in one cycle.
   * This demonstrates how to read multiple values in a single round trip.
   * eb_cycle_open always returns immediately.
   */

  for (l_i=0; l_i<nIterations; l_i++) {

    // >>>>>>>>>>>>>>>>>>>>>>>>> this is the important part >>>>>>>>>>>>>>
    if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK)
      die("EP eb_cycle_open", status);


    /* Queueing operations to a cycle never fails (EB_OOM is reported later) */
    /* read status of FIFOs is disabled, since a successful read is detected by the status returned by eb_cycle_close */
        
    eb_cycle_read(cycle, wrTLU + GSI_TM_LATCH_FIFO_READY, EB_BIG_ENDIAN|EB_DATA32, &data1);
    /* read high word of latched timestamp */
    eb_cycle_read(cycle, wrTLU + GSI_TM_LATCH_FIFO_FTSHI, EB_BIG_ENDIAN|EB_DATA32, &data2);
    /* read low word of latched timestamp */
    eb_cycle_read(cycle, wrTLU + GSI_TM_LATCH_FIFO_FTSLO, EB_BIG_ENDIAN|EB_DATA32, &data3);
    /* pop timestamp from FIFO */
    eb_cycle_write(cycle, wrTLU + GSI_TM_LATCH_FIFO_POP, EB_BIG_ENDIAN|EB_DATA32, 0xF);

    /* Because the cycle was opened with eb_block, this is a blocking call.
     * Upon termination, data and data2 will be valid.
     * For higher performance, use multiple asynchronous cycles in a pipeline.
     */

    /* for the TLU, "POP" yields an error in case no TS is available */
    if ((status = eb_cycle_close(cycle)) == EB_OK) {
      // <<<<<<<<<<<<<<<<<<<<<<< this was the important part
      /* do some gymnastics to get a human readible timestamp */
      timestamp = data2;
      //shift high word
      timestamp = (timestamp << 32);
      //add low word
      timestamp = timestamp + data3;

      actu_timestamp = timestamp;
      diff_timestamp = actu_timestamp - prev_timestamp;   
      prev_timestamp = actu_timestamp;
      nLatches++;

      printf("diff time_stamp of channel %d  : %llu [ns] \n", channelSelect, diff_timestamp<<3); //fflush (stdout);  
      printf("latched timestamp of channel %d: %llu [ns] \n", channelSelect, actu_timestamp<<3); fflush (stdout);
    }
  }

if ((status = eb_device_read(device, wrTLU + GSI_TM_LATCH_FIFO_CNT, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block)) != EB_OK)
		die("TLU eb_device_read", status);
	count_final = data;
    printf("\nFIFO Length: Before Read %lld, After Read %lld\n",count_prev,count_final);

  /* read the actual timestamp a second time - needed for performance measurement */
  if ((status = eb_device_read(device, wrTLU + GSI_TM_LATCH_ATSHI, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block)) != EB_OK)
    die("TLU eb_device_read", status);
  timestamp = data;
  timestamp = (timestamp << 32);
  
  if ((status = eb_device_read(device, wrTLU + GSI_TM_LATCH_ATSLO, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block)) != EB_OK)
    die("TLU eb_device_read", status);
  timestamp = timestamp + data;
  timestamp_stop = timestamp;

  //calculate performance data
  elapsed_time   = (double)(timestamp_stop - timestamp_start) / (double)clockFreq * 1000000.0;
  timePerEBCycle = (double)elapsed_time / (double)nIterations;
  //timePerEBCycle = elapsed_time / nIterations;
  printf("elapsed time     : %d [us] \n", elapsed_time);
  printf("time per EB cycle: %f [us] \n", timePerEBCycle);
  printf("readout rate     : %.1f [Hz] \n", 1000000.0 / (double)timePerEBCycle);

  //convert latched timestamp to seconds
  secs = actu_timestamp / clockFreq;
  tm = localtime(&secs);
  strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S %Z", tm);
  
  /* Print the result */
  itoa(data1, buff, 2);
 
  printf("FIFOs                      :%s\n",channelStr);
  printf("Status of FIFOs            :%s\n", buff);
    if (actu_timestamp > 0) {
    printf("latched timestamp of FIFO %d: %s\n", channelSelect, time);  
    printf("nOf latched timestamps     : %d\n", nLatches);
  }
  else               printf("sorry: no latched timestamp \n");

  /* close handler cleanly */
  if ((status = eb_device_close(device)) != EB_OK)
    die("eb_device_close", status);
  if ((status = eb_socket_close(socket)) != EB_OK)
    die("eb_socket_close", status);
  
  return 0;
}



