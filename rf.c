#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <wiringPi.h>

#ifndef RPIN 
#define RPIN 1
#endif

#ifndef WPIN 
#define WPIN 2
#endif

#define T 250

#define M 50

int readPacket();
int readBit();
int readVal();

int main() 
{
    // Setup wiringPi. Wpi-mode (type 'gpio readall' to see pin numbering)
    wiringPiSetup();
    pinMode(RPIN, INPUT);
    pinMode(WPIN, OUTPUT);
    
    int ret, code, group, on, channel, unit;
    
    while(1)
    {        
        ret = readPacket();
        
        code = (ret & 0xFFFFFFC0) >> 6;
        group = (ret & 0x00000020) >> 5;
        on = (ret & 0x00000010) >> 4;
        channel = (ret & 0x0000000C) >> 2;
        unit = ret & 0x00000003;
        
        printf("packet received: \n");
        printf("code 0x%x, group %d, on %d, channel %d, unit %d \n\n", code, group, on, channel, unit);
    }
    
    return 0;
}

int readPacket()
{
    
    /*
    S HHHH HHHH HHHH HHHH HHHH HHHH HHGO CCEE P

    S = Sync bit.
    H = The first 26 bits are transmitter unique codes, and it is this code that the reciever "learns" to recognize.
    G = Group code. Set to 0 for on, 1 for off.
    O = On/Off bit. Set to 0 for on, 1 for off.
    C = Channel bits. Proove/Anslut = 00, Nexa = 11.
    E = Unit bits. Device to be turned on or off.
    Proove/Anslut Unit #1 = 00, #2 = 01, #3 = 10.
    Nexa Unit #1 = 11, #2 = 10, #3 = 01.
    P = Pause bit.
    */
    int i, prevbit, bit, ret;
    
    while (readBit() != 's');
    
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
    
    // Read T us H
    while (readVal());
    
    // Read x us L
    gettimeofday(&tv, NULL); 
    oldtime = (tv.tv_sec * 1000000) + tv.tv_usec;
    
    while (!readVal());
    
    gettimeofday(&tv, NULL); 
    time = (tv.tv_sec * 1000000) + tv.tv_usec;
    
    
    double t = time - oldtime;
    // x = 1 -> '1'
    if (T-M < t && t < T+M)
    {
        return 1;
    }
    // x = 5 -> '0'
    else if (5*T-M < t && t < 5*T+M)
    {
        return 0;
    }
    // x = 10 -> 'SYNC'
    else if (10*T-M < t && t < 10*T+M)
    {
        return 's';
    }
    // x = 40 -> 'PAUSE'
    else if (40*T-M < t && t < 40*T+M)
    {
        return 'p';
    }
    // not anything of above
    return -1;
    

}

int readVal()
{
    int ret;
    
    ret = digitalRead(RPIN);
    
    return ret;
}
