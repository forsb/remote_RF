#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <wiringPi.h>

#define RPIN 5
#define VCC 13

int main(){
    wiringPiSetup();
    pinMode(RPIN, INPUT);
    pinMode(VCC, OUTPUT);
    
    digitalWrite(VCC, HIGH);
    
    while(1)
    {
        int a = digitalRead(RPIN);
        if(a > 10){
            printf("readval: %d\n", a);
        }
    }
}
