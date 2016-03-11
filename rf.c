#include <stdio.h>
#include <time.h>
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

int readPacket()
{
    
    return 1;
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
    
    switch(oldtime - time){
        
    // x = 1 -> '1'
        case T-M ... T+M: return 1;
    // x = 5 -> '0'
    
    // x = 10 -> 'SYNC'
    
    // x = 40 -> 'PAUSE'
    
        default return -1;
    
    }
    return 1;
}

int readVal()
{
    int ret = 0;
    
    return ret;
}

int main() 
{
    // Setup wiringPi. Wpi-mode (type 'gpio readall' to see pin numbering)
    wiringPiSetup();
    pinMode(RPIN, INPUT);
    pinMode(WPIN, OUTPUT);
    
    printf("Hello %s!", "world");
}