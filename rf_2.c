#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <wiringPi.h>

#define RPIN 14
#define WPIN 2
#define VCC 0

/*
RECEIVER

wpi     phys        pins
-----------------------
gnd     gnd         gnd
5       18          digital out
14      23          linear out
13      21          vcc


TRANSMITTER

wpi     phys        pins
-----------------------
gnd     gnd (9)     gnd
0       11          vcc
2       13          data     

*/

/*
S HHHH HHHH HHHH HHHH HHHH HHHH HHGO DDDD

S = Sync bit.
H = The first 26 bits are transmitter unique codes, and it is this code that the reciever "learns" to recognize.
G = Group code. Set to 0 for on, 1 for off.
O = On/Off bit. Set to 0 for on, 1 for off.
D = Device to be turned on or off.

*/
    
    
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



#define PRE 275
#define SYNC 2675
#define ZERO 275
#define ONE 1225

int readPacket();
int readBit();
int readVal();

int createPacket(int host, int group, int on, int device);
int sendPacket(int packet);
int sendBit(int bit);
int sendVal(int val);

int main() 
{
    printf("high1: %d\n", HIGH);

    // Setup wiringPi. Wpi-mode (type 'gpio readall' to see pin numbering)
    wiringPiSetup();
    pinMode(RPIN, INPUT);
    pinMode(WPIN, OUTPUT);
    pinMode(VCC, OUTPUT);
    
    digitalWrite(VCC, HIGH);

    printf("high2: %d\n", HIGH);
    
    while(1)
    {
        int paket = createPacket(1337, 0, 0, 23);
        sendPacket(paket);
        delay(2000);
    }
    
    
    /*int ret, host, group, on, device;
    
    while(1)
    {        
        ret = readPacket();
        
        host = (ret & 0xFFFFFFC0) >> 6;
        group = (ret & 0x00000020) >> 5;
        on = (ret & 0x00000010) >> 4;
        device = ret & 0x0000000F;
        
        printf("host:group:on:device %d:%d:%d:%d\n", host, group, on, device);
            
    }*/
    
    return 0;
}

int createPacket(int host, int group, int on, int device)
{
    return (host << 6) | (group << 5) | (on << 4) | device;
}

int sendPacket(int packet)
{
    sendBit('S');
    
    for (i = 0; i < 32; i++)
    {
        sendBit((packet >> i) & 0x1);
        sendBit(!(packet >> i) & 0x1);
    }
    return 1;
}

int sendBit(int bit)
{
    digitalWrite(WPIN, HIGH);
    delayMicroseconds(PRE);
    digitalWrite(WPIN, LOW);
    
    if (bit == 'S')
        delayMicroseconds(SYNC);
        
    else if (bit == 0)
        delayMicroseconds(ZERO);
        
    else if (bit == 1)
        delayMicroseconds(ONE);
        
    else
        return -1;
        
    return 1;
}

int sendVal(int val)
{
    digitalWrite(WPIN, val);
    return 1;
}

int readPacket()
{
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
    return digitalRead(RPIN);
}
