/* Includes */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "disp.h"

/* Global */
const char* program;
const char* devName;

/* Prototypes */
void die(const char* where, eb_status_t status);

/* Function die(...) */
void die(const char* where, eb_status_t status)
{
  fprintf(stderr, "%s: %s failed: %s\n",
  program, where, eb_status(status));
  exit(1);
}

/* Function main(...) */
int main(int argc, char** argv)
{
  /* Helpers */
  unsigned int i;
  int opt;
  char str[170] = "\f";
  char givenDisplay = '0';
  str[169] = '\0';

  /* Etherbone */
  eb_socket_t socket;
  eb_status_t status;
  eb_device_t device;
  
  /* Process the command-line arguments */
  program = argv[0];
  while ((opt = getopt(argc, argv, "s:d:")) != -1)
  {
    switch (opt)
    {
      case 's':
      {
        i=0;
        while(i<168 && (optarg[i] != '\0')) { str[i+1] = optarg[i]; i++;}
        str[i+1] = '\0';
        break;
      }
      case 'd':
      {
        givenDisplay = optarg[0];
        switch (givenDisplay)
        {
          case '1': { desiredDisplay = OLED; break; }
          case '2': { desiredDisplay = LCD; break; }
          case '3': { desiredDisplay = SSD1325; break; }
          default:  
          { 
            fprintf(stderr, "%s: Selected display id %c is unknown!\n", program, givenDisplay);
            return 1;
            break;
          }
        }
        break;
      }
      case '?':
      {
        fprintf(stderr, "Usage: %s <protocol/host/port> -s <\"my string\">\n", program);
        break;
      }
      default:
      {
        fprintf(stderr, "%s: bad getopt result\n", program);
        return 1;
      }
    }
  }
  
  /* Print help depending on arguments */
  if (optind + 1 != argc)
  {
    fprintf(stderr, "Usage: %s <protocol/host/port> -s <\"my string\"> -d <device>\n", program);
    fprintf(stderr, "\n");
    fprintf(stderr, "Devices:\n");
    fprintf(stderr, "  -d %d ... for OLEDisplay \n", OLED);
    fprintf(stderr, "  -d %d ... for LCDisplay \n", LCD);
    fprintf(stderr, "  -d %d ... for SSD1325 Display \n", SSD1325);
    fprintf(stderr, "\n");
    fprintf(stderr, "Example: %s udp/192.168.191.40 -s \"Hello World!\" -d 2\n", program);
    fprintf(stderr, "  ... will print \"Hello World\" to the LCDisplay\n");
    return 1;
  }
  
  /* Open EB socket and device */
  devName = argv[optind];
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDR32|EB_DATA32, &socket)) != EB_OK)
    die("eb_socket_open", status);
  if ((status = eb_device_open(socket, devName, EB_ADDR32|EB_DATA32, 3, &device)) != EB_OK)
    die("eb_device_open", status);
  
  /* Check if a display was found */
  if (!init_disp(device))
  {
    return 1;
  }
  else
  {
    disp_put_s(device, str);
  }
    
  /* Close handler cleanly */
  if ((status = eb_device_close(device)) != EB_OK)
    die("eb_device_close", status);
  if ((status = eb_socket_close(socket)) != EB_OK)
    die("eb_socket_close", status);

  /* Done */
  return 0;
}

