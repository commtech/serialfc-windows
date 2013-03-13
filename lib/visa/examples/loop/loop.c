/********************************************************************/
/*                Read and Write to a Serial Instrument             */
/*                                                                  */
/* This code demonstrates sending synchronous read & write commands */
/* through the serial port using VISA.                              */
/* The example loops data between serial ports                      */
/*                                                                  */
/* The general flow of the code is                                  */
/*    Open Resource Manager                                         */
/*    Open VISA Session to an Instrument                            */
/*    Configure the Serial Port                                     */
/*    Write the data Using viWrite                                  */
/*    Read the data back With viRead                                */
/*    Close the VISA Session                                        */
/********************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>

#include <visa.h>

#define DATA_LENGTH 20

void init(ViSession instr);
int loop(ViSession instr1, ViSession instr2);

int main(int argc, char *argv[])
{
    ViSession defaultRM;     
    ViSession instr1, instr2;     
    ViStatus status;
    unsigned port_num_1, port_num_2;
    char port_name_1[10], port_name_2[10];
    unsigned reset;
    unsigned iterations = 0;
    unsigned mismatched = 0;
    int e = 0;

    if (argc < 3 || argc > 4) {
        fprintf(stdout, "%s PORT_NUM PORT_NUM [RESET_SETTINGS=1]", argv[0]);
        return EXIT_FAILURE;
    }

    port_num_1 = atoi(argv[1]);
    port_num_2 = atoi(argv[2]);
    reset = (argc == 4) ? atoi(argv[3]) : 1;

    memset(port_name_1, 0, sizeof(port_name_1));
    memset(port_name_2, 0, sizeof(port_name_2));

    sprintf_s(port_name_1, sizeof(port_name_1), "COM%i", port_num_1);
    sprintf_s(port_name_2, sizeof(port_name_2), "COM%i", port_num_2);

    /*
     * First we must call viOpenDefaultRM to get the manager
     * handle.  We will store this handle in defaultRM.
     */
    status = viOpenDefaultRM (&defaultRM);
    if (status < VI_SUCCESS) {
        printf ("Could not open a session to the VISA Resource Manager!\n");
        return EXIT_FAILURE;
    }
                                                          
    /*
     * Now we will open a VISA session to the serial port (COM3).
     * We must use the handle from viOpenDefaultRM and we must   
     * also use a string that indicates which instrument to open.  This
     * is called the instrument descriptor.  The format for this string
     * can be found in the function panel by right clicking on the 
     * descriptor parameter. After opening a session to the
     * device, we will get a handle to the instrument which we 
     * will use in later VISA functions.  The AccessMode and Timeout
     * parameters in this function are reserved for future
     * functionality.  These two parameters are given the value VI_NULL.
     */
    status = viOpen(defaultRM, port_name_1, VI_NULL, VI_NULL, &instr1);
    if (status < VI_SUCCESS) {
        printf ("Cannot open a session to the device1.\n");
        viClose (defaultRM);
        return EXIT_FAILURE;
    }

    status = viOpen(defaultRM, port_name_2, VI_NULL, VI_NULL, &instr2);
    if (status < VI_SUCCESS) {
        printf ("Cannot open a session to the device2.\n");
        viClose (instr1);
        viClose (defaultRM);
        return EXIT_FAILURE;
    }

    if (reset) {
        init(instr1);
        init(instr2);
    }

    fprintf(stdout, "Data looping, press any key to stop...\n");  

    while (_kbhit() == 0) {
        e = loop(instr1, instr2);
        if (e != 0) {
            if (e == 1) {
                mismatched++;
            }
            else {
                viClose (instr1);
                viClose (instr2);
                viClose (defaultRM);
                return EXIT_FAILURE;
            }
        }

        iterations++;
    }

    if (mismatched == 0)
        fprintf(stdout, "Passed (%d iterations).", iterations);
    else
        fprintf(stderr, "Failed (%d out of %d iterations).", 
                mismatched, iterations);

    viClose (instr1);
    viClose (instr2);
    viClose (defaultRM);

   return EXIT_SUCCESS;
}

void init(ViSession instr)
{
    /* 
     * At this point we now have a session open to the serial instrument. 
     * Now we need to configure the serial port:
     */                                      
      
    /* Set the timeout to 5 seconds (5000 milliseconds). */
    viSetAttribute (instr, VI_ATTR_TMO_VALUE, 100);
  
    /* Set the baud rate to 4800 (default is 9600). */
    viSetAttribute (instr, VI_ATTR_ASRL_BAUD, 115200);
  
    /* Set the number of data bits contained in each frame (from 5 to 8). 
     * The data bits for  each frame are located in the low-order bits of
     * every byte stored in memory.    
     */
    viSetAttribute (instr, VI_ATTR_ASRL_DATA_BITS, 8);
  
    /* Specify parity. Options: 
     * VI_ASRL_PAR_NONE  - No parity bit exists, 
     * VI_ASRL_PAR_ODD   - Odd parity should be used, 
     * VI_ASRL_PAR_EVEN  - Even parity should be used,
     * VI_ASRL_PAR_MARK  - Parity bit exists and is always 1,
     * VI_ASRL_PAR_SPACE - Parity bit exists and is always 0.
     */
    viSetAttribute (instr, VI_ATTR_ASRL_PARITY, VI_ASRL_PAR_NONE);
  
   /* Specify stop bit. Options:
    * VI_ASRL_STOP_ONE   - 1 stop bit is used per frame,
    * VI_ASRL_STOP_ONE_5 - 1.5 stop bits are used per frame,
    * VI_ASRL_STOP_TWO   - 2 stop bits are used per frame.
    */
    viSetAttribute (instr, VI_ATTR_ASRL_STOP_BITS, VI_ASRL_STOP_ONE);
  
    /* Specify that the read operation should terminate when a termination 
     * character is received.
     */
    viSetAttribute (instr, VI_ATTR_TERMCHAR_EN, VI_TRUE); 
 
    /* Set the termination character to 0xA                            
     */
    viSetAttribute (instr, VI_ATTR_TERMCHAR, 0xA);
}

int loop(ViSession instr1, ViSession instr2)
{
    ViStatus status;
    unsigned char odata[DATA_LENGTH];
    unsigned char idata[100];
    ViUInt32 retCount;
    ViUInt32 writeCount;

    memset(odata, 0x01, sizeof(odata));
    memset(&idata, 0, sizeof(idata));

    odata[DATA_LENGTH - 1] = '\n';
 
    /* We will use the viWrite function to send the device the string "*IDN?\n",
     * asking for the device's identification.  
    */
    status = viWrite (instr1, (ViBuf)odata, (ViUInt32)sizeof(odata), &writeCount);
    if (status < VI_SUCCESS) {
        printf ("Error writing to the device.\n");
        return -1;
    }
    
    /*
     * Now we will attempt to read back a response from the device to
     * the identification query that was sent.  We will use the viRead
     * function to acquire the data.  We will try to read back 100 bytes.
     * This function will stop reading if it finds the termination character
     * before it reads 100 bytes.
     * After the data has been read the response is displayed.
     */
    status = viRead (instr2, idata, sizeof(idata), &retCount);
    if (status < VI_SUCCESS) {
        printf ("Error reading a response from the device.\n");
        return -1;
    }
    
    if (retCount == 0 || memcmp(odata, idata, sizeof(odata)) != 0)
        return 1;

    return 0;
}