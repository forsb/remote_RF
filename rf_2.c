#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <wiringPi.h>

/*
RECEIVER

wpi     phys        pins
-----------------------
gnd     gnd         gnd
5       18          digital out
14      23          linear out
13      21          vcc

*/

#ifndef RPIN 
#define RPIN 5
#endif

#ifndef WPIN 
#define WPIN 2
#endif

#ifndef VCC
#define VCC 13
#endif

#define PRE 275
#define SYNC 2675
#define ZERO 275
#define ONE 1225

int readPacket();
int readBit();
int readVal();

int sendPacket(int packet);
int sendBit(int bit);
int sendVal(int val);

int main() 
{
    // Setup wiringPi. Wpi-mode (type 'gpio readall' to see pin numbering)
    wiringPiSetup();
    pinMode(RPIN, INPUT);
    pinMode(WPIN, OUTPUT);
    pinMode(VCC, OUTPUT);
    
    digitalWrite(VCC, HIGH);
    
    int ret, host, group, on, device;
    
    while(1)
    {        
        ret = readPacket();
        
        host = (ret & 0xFFFFFFC0) >> 6;
        group = (ret & 0x00000020) >> 5;
        on = (ret & 0x00000010) >> 4;
        device = ret & 0x0000000F;
        
        printf("host:group:on:device %d:%d:%d:%d\n", host, group, on, device);
            
    }
    
    return 0;
}

int readPacket()
{
    
    /*
    S HHHH HHHH HHHH HHHH HHHH HHHH HHGO DDDD

    S = Sync bit.
    H = The first 26 bits are transmitter unique codes, and it is this code that the reciever "learns" to recognize.
    G = Group code. Set to 0 for on, 1 for off.
    O = On/Off bit. Set to 0 for on, 1 for off.
    D = Device to be turned on or off.
    
    */
    int i, prevbit, bit, ret;
    
    while (readBit() != 'S');
    
    for (i = 0; i < 64; i++)
    {
        bit = readBit();
        
        if (i % 2 == 0) //First bit of 2
        {
            prevbit = bit;
            continue;
        }
        
        if ((prevbit ^ bit) == 0) //Bit-errors
        {
            return 0;
        }
        
        ret = ret << 1 | prevbit;
        
    }
    
    return ret;
}

int readBit()
{
    double time = 0, oldtime = 0;
    struct timeval  tv;
    
    
    /*
    
    * A latch of 275us high, 2675us low is sent before the data.
    * There is a gap of 10ms between each message.
    * 0 = holding the line high for 275us then low for 275us.
    * 1 = holding the line high for 275us then low for 1225us.
    
    SYNC
     _____
    |     |
    |     |_____ .... _____
      275        2675
      
    
    ZERO
     _____
    |     |
    |     |_____ 
      275   275
      
    
    ONE
     _____
    |     |
    |     |_____ .... _____
      275        1225
    
    
    */
    
    // Wait for high
    while (!readVal());
    
    // Read PRE us high
    while (readVal());
    
    // Read x us low
    gettimeofday(&tv, NULL); 
    oldtime = (tv.tv_sec * 1000000) + tv.tv_usec;
    
    while (!readVal());
    
    gettimeofday(&tv, NULL); 
    time = (tv.tv_sec * 1000000) + tv.tv_usec;    
    
    double t = time - oldtime;
    
    // ZERO
    if (0.9 * ZERO < t && t < 1.1 * ZERO)
    {
        return 1;
    }
    
    // ONE
    else if (0.9 * ONE < t && t < 1.1 * ONE)
    {
        return 0;
    }
    // SYNC
    else if (0.9 * SYNC < t && t < 1.1 * SYNC)
    {
        return 'S';
    }
    
    // None of above
    return -1;
    
}

int readVal()
{    
    return = digitalRead(RPIN);
}

int sendPacket(int packet)
{
    return 1;
}

int sendBit(int bit)
{
    return 1;
}

int sendVal(int val)
{
    return 1;
}