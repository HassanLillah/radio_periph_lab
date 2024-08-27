/******************************************************************************
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdint.h>
#include <stdio.h>
#include <sys/_intsup.h>
#include <xil_types.h>
#include <math.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xgpio_l.h"
#include "xiic_l.h"
#include "sleep.h"
#include "xuartps_hw.h"
#include "xllfifo_hw.h"

void write_codec_register(u8 regnum, u16 regval)
{
    u8 data[2];
    data[0] = regnum;
    data[0] = data[0] << 1;
    data[0] = data[0] & 0xfe;
    data[0] = data[0] | ((regval >> 8) & 0x01);
    data[1] = regval & 0xff;
    XIic_Send(XPAR_XIIC_0_BASEADDR, 0x1a,
		    &data[0], 2, XIIC_STOP);
}
void configure_codec()
{
    write_codec_register(15,0x0000);
    usleep(1000);
    write_codec_register(6,0x0037);
    write_codec_register(0,0x0080);
    write_codec_register(1,0x0080);
    write_codec_register(2,0x0079);
    write_codec_register(3,0x0079);
    write_codec_register(4,0x0010);
    write_codec_register(5,0x0000);
    write_codec_register(7,0x0002);
    write_codec_register(8,0x0000);
    usleep(75000);
    write_codec_register(6,0x0027);
    usleep(75000);
    write_codec_register(9,0x0001);
}
void print_menu()
{
    print("Press '+' to increase volume\n\r");
    print("Press '-' to decrease volume\n\r");
    print("Press 'F' to enter a new input signal frequency\n\r");
    print("Press 'T' to enter a new tune frequency\n\r");
    print("Press 'U/u' to increase frequency by 1000/100 Hz\n\r");
    print("Press 'D/d' to decrease frequency by 1000/100 Hz\n\r");
    print("Press 'r' to reset the phase\n\r");
    print("Press [space] to repeat this menu\n\n\r");
}
int calc_phase_inc(int Fout, int N, int clk_freq)
{
    float phasetemp;
    int phase;
    phasetemp = (float)Fout*(float)pow(2,N);
    phasetemp = phasetemp/(float)clk_freq;
    phasetemp = round(phasetemp);
    phase = (int)phasetemp;
    printf("Phase Increment = %d\n\n\r", phase);
    return phase;
}
int fetch_frequency()
{
    u8  byte;
    int freq;
    freq = 0;
    while(!XUartPs_IsReceiveData(XPAR_XUARTPS_0_BASEADDR)){}
    byte = XUartPs_RecvByte(XPAR_XUARTPS_0_BASEADDR);
    if (byte==0x0a || byte==0x0d){
        print("Invalid Frequency. Aborting Command.\n\r");
        print("Frequency = 0\n\r");
        freq = 0;
        return freq;
    }
    while(byte!=0x0a && byte!=0x0d){
        switch (byte){
        case 0x30:
            freq= freq*10;
            print("0");         
            break;
        case 0x31:
            freq= freq*10+1;
            print("1"); 
            break;
        case 0x32:
            freq= freq*10+2;
            print("2"); 
            break;
        case 0x33:
            freq= freq*10+3;
            print("3"); 
            break;
        case 0x34:
            freq= freq*10+4;
            print("4"); 
            break;
        case 0x35:
            freq= freq*10+5;
            print("5"); 
            break;
        case 0x36:
            freq= freq*10+6;
            print("6"); 
            break;
        case 0x37:
            freq= freq*10+7;
            print("7"); 
            break;
        case 0x38:
            freq= freq*10+8;
            print("8"); 
            break;
        case 0x39:
            freq= freq*10+9;
            print("9"); 
            break;
        default:
            print("Invalid Frequency. Aborting Command.\n\r");
            print("Frequency = 0\n\r");
            freq=0;
            return freq;
        }
        while(!XUartPs_IsReceiveData(XPAR_XUARTPS_0_BASEADDR)){}
        byte = XUartPs_RecvByte(XPAR_XUARTPS_0_BASEADDR);
    }
    printf("Frequency = %d\n\r", freq);
    return freq;
}

int main()
{
    init_platform();

    const int   N = 27;
    const int   clk_freq=125000000;
    int         PhaseInc;
    int         TunerPhaseInc;
    int         Fout;
    int         Tout;
    u8          user_input;
    u16         volume=0x0079;
    usleep(5000000);
    print("\n\r");
    print("Welcome to Lab 5 by Hassan Nisar\n\r");
    configure_codec();
    print("The codec start-up has been completed\n\r");
    print_menu();
    XGpio_WriteReg(XPAR_AXI_GPIO_0_BASEADDR, XGPIO_DATA_OFFSET,0x01);
    //XGpio_WriteReg(XPAR_TUNER_PHASE_RESETN_BASEADDR, XGPIO_DATA_OFFSET,0x01); 

    while(TRUE){
        if (XUartPs_IsReceiveData(XPAR_XUARTPS_0_BASEADDR)) {
            user_input = XUartPs_RecvByte(XPAR_XUARTPS_0_BASEADDR);
            printf("User Unicode Input =%x\n\r", user_input);
            if(user_input==0x2b) {
                /* + */
                if(volume<0x007f){
                    volume = volume + 1;
                }
                write_codec_register(2,volume);
                write_codec_register(3,volume);
                printf("Volume=%x\r\n",volume);
            } else if(user_input==0x2d){
                /* - */
                if(volume>0x0000) {
                    volume = volume - 1;
                }
                write_codec_register(2,volume);
                write_codec_register(3,volume);
                printf("Volume=%x\r\n",volume);
            } else if(user_input==0x46){
                /* f */
                print("F");
                Fout = fetch_frequency();
                PhaseInc = calc_phase_inc(Fout, N, clk_freq);
                XGpio_WriteReg(XPAR_AXI_GPIO_1_BASEADDR, XGPIO_DATA_OFFSET, PhaseInc);
            } else if (user_input==0x54) {
                /* T */
                print("T");
                Tout = fetch_frequency();
                TunerPhaseInc = calc_phase_inc(Tout, N, clk_freq);
                XGpio_WriteReg(XPAR_TUNER_PHASE_INC_BASEADDR, XGPIO_DATA_OFFSET, TunerPhaseInc);
            } else if (user_input==0x55) {
                /* U */
                Fout = Fout + 1000;
                if (Fout > 125000000){
                    Fout = 125000000;
                    print("Frequency cannot be greater than 125 MHz\n\r");
                }
                printf("Frequency = %d\n\r", Fout);
                PhaseInc = calc_phase_inc(Fout, N, clk_freq);
                XGpio_WriteReg(XPAR_AXI_GPIO_1_BASEADDR, XGPIO_DATA_OFFSET, PhaseInc);
            } else if (user_input==0x75) {
                /* u */
                Fout = Fout + 100;
                if (Fout > 125000000){
                    Fout = 125000000;
                    print("Frequency cannot be greater than 125 MHz\n\r");
                }
                printf("Frequency = %d\n\r", Fout);
                PhaseInc = calc_phase_inc(Fout, N, clk_freq);
                XGpio_WriteReg(XPAR_AXI_GPIO_1_BASEADDR, XGPIO_DATA_OFFSET, PhaseInc);
            } else if (user_input==0x44) {
                /* D */
                Fout = Fout - 1000;
                if (Fout < 0){
                    Fout = 0;
                    print("Frequency cannot be less than 0 Hz\n\r");
                }
                printf("Frequency = %d\n\r", Fout);
                PhaseInc = calc_phase_inc(Fout, N, clk_freq);
                XGpio_WriteReg(XPAR_AXI_GPIO_1_BASEADDR, XGPIO_DATA_OFFSET, PhaseInc);
            } else if (user_input==0x64) {
                /* d */
                Fout = Fout - 100;
                if (Fout < 0){
                    Fout = 0;
                    print("Frequency cannot be less than 0 Hz\n\r");
                }
                printf("Frequency = %d\n\r", Fout);
                PhaseInc = calc_phase_inc(Fout, N, clk_freq);
                XGpio_WriteReg(XPAR_AXI_GPIO_1_BASEADDR, XGPIO_DATA_OFFSET, PhaseInc);
            } else if (user_input==0x72) {
                /* r */
                print("Reset the Phase to 0\n\r");  
                XGpio_WriteReg(XPAR_AXI_GPIO_0_BASEADDR, XGPIO_DATA_OFFSET,0x00); 
                usleep(1);
                XGpio_WriteReg(XPAR_AXI_GPIO_0_BASEADDR, XGPIO_DATA_OFFSET,0x01);
            } else if (user_input==0x20) {
                /* [space] */  
                print_menu();  
            }
        }
        //XGpio_WriteReg(XPAR_AXI_GPIO_1_BASEADDR, XGPIO_DATA_OFFSET, PhaseInc);
        //XGpio_WriteReg(XPAR_TUNER_PHASE_INC_BASEADDR, XGPIO_DATA_OFFSET, TunerPhaseInc);
        //current_val = XGpio_ReadReg(XPAR_AXI_GPIO_2_BASEADDR,XGPIO_DATA_OFFSET);
        //XGpio_WriteReg(XPAR_AXI_GPIO_2_BASEADDR, XGPIO_DATA2_OFFSET,current_val);
    }
    cleanup_platform();
    return 0;
}
