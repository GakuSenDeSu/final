#include "mbed.h"
#include "bbcar.h"

Serial pc(USBTX,USBRX); //tx,rx
Serial uart(D1,D0); //tx,rx
Serial xbee(D12, D11); //tx,rx

Ticker servo_ticker;
Ticker encoder_ticker;
Timer matrix_shot;
PwmOut pin8(D8), pin9(D9);
DigitalIn pin3(D3);
DigitalInOut pin10(D10);
DigitalOut GREEN(LED2);
DigitalOut BLUE(LED3);
char s[21];
char recv[1000] = {0};

BBCar car(pin8, pin9, servo_ticker);


int main() {
    wait(3);
    BLUE = 1;
    
    // enter //
    parallax_encoder encoder0(pin3, encoder_ticker);
    encoder0.reset();
    GREEN = 0; car.goStraight(100); //enter maze
    while(encoder0.get_cm()<100) {wait_us(50000); xbee.printf("Go Straight\r\n");} //Have try: 100
    GREEN = 1; car.stop(); wait_us(50000); xbee.printf("Wait for a while\r\n");
    
    // calibrate by matrix(save shot)//
    BLUE = 0;
    uart.baud(9600);
    
    matrix_shot.start();
    while(matrix_shot.read()<10){
      xbee.printf(s,"matrix");
      uart.puts(s);
      xbee.printf("send\r\n");
      wait(0.5);
      if(uart.readable()){
        char recv = uart.getc();
        xbee.putc(recv);
        xbee.printf("\r\n");
      }
    }
    matrix_shot.reset();
    BLUE = 1;
    
    // turn left //
    car.turn(100, +0.1); wait_us(1500); xbee.printf("Turn left\r\n"); car.stop();
    encoder0.reset();
    GREEN = 0; car.goStraight(100);
    while(encoder0.get_cm()<25) {wait_us(50000);xbee.printf("Go Straight(enter Mission 1)\r\n");} // enter mission 1
    GREEN = 1; 
    
    car.turn(100, +0.1); wait_us(1500); xbee.printf("Turn left\r\n"); car.stop();
    encoder0.reset();
    GREEN = 0; car.goStraight(100);
    while(encoder0.get_cm()<25) {wait_us(50000);xbee.printf("Go Straight\r\n");}
    GREEN = 1;
    
    car.turn(100, -0.1); wait_us(1500); xbee.printf("Turn right\r\n"); car.stop();
    encoder0.reset();
    GREEN = 0; car.goStraight(100);
    while(encoder0.get_cm()<35) {wait_us(50000);xbee.printf("Go Straight\r\n");}
    GREEN = 1;

    car.turn(100, -0.1); wait_us(1500); xbee.printf("Turn right(face to number)\r\n"); car.stop();
    
    // machine learning //
    BLUE = 0;
    matrix_shot.start();
    while(matrix_shot.read()<10){
      xbee.printf(s,"number");
      uart.puts(s);
      xbee.printf("send\r\n");
      wait(0.5);
      if(uart.readable()){
        char recv = uart.getc();
        xbee.putc(recv);
        xbee.printf("\r\n");
      }
    }
    matrix_shot.reset();
    BLUE = 1;
    
    // car into garage
    car.turn(-100, -0.1); wait_us(1500); xbee.printf("Reverse right\r\n"); car.stop();
    encoder0.reset();
    GREEN = 0; car.goStraight(-100);
    while(encoder0.get_cm()<35) {wait_us(50000);xbee.printf("Go Reverse\r\n");}
    GREEN = 1;

    car.turn(-100, +0.3); wait_us(1500); xbee.printf("Reverse into garage\r\n"); car.stop();wait_us(2000000);
    encoder0.reset();
    GREEN = 0; car.goStraight(100);
    while(encoder0.get_cm()<35) {wait_us(50000);xbee.printf("Go straight\r\n");}
    GREEN = 1;

    car.turn(100, -0.3); wait_us(3000); xbee.printf("Turn right(leave Mission 1)\r\n"); car.stop();
    encoder0.reset();
    GREEN = 0; car.goStraight(100);
    while(encoder0.get_cm()<100) {wait_us(50000);xbee.printf("Go straight\r\n");}
    GREEN = 1;

    // enter mission2
    car.turn(100, -0.1); wait_us(1500); xbee.printf("Turn right\r\n"); car.stop();
    encoder0.reset();
    GREEN = 0; car.goStraight(100);
    while(encoder0.get_cm()<15) {wait_us(50000);xbee.printf("Go Straight(enter Mission 2)\r\n");} // enter mission 2
    GREEN = 1; 

    car.turn(100, -0.1); wait_us(1500); xbee.printf("Turn right\r\n"); car.stop();
    encoder0.reset();
    GREEN = 0; car.goStraight(100);
    while(encoder0.get_cm()<12) {wait_us(50000);xbee.printf("Go Straight(enter Mission 2)\r\n");} // enter mission 2
    GREEN = 1;

    car.turn(100, -0.1); wait_us(1500); xbee.printf("Turn right(face to object)\r\n"); car.stop();
    
    // calibrate by matrix(save shot)//
    BLUE = 0;
    matrix_shot.start();
    while(matrix_shot.read()<10){
      xbee.printf(s,"matrix");
      uart.puts(s);
      xbee.printf("send\r\n");
      wait(0.5);
      if(uart.readable()){
        char recv = uart.getc();
        xbee.putc(recv);
        xbee.printf("\r\n");
      }
    }
    matrix_shot.reset();
    BLUE = 1;
    wait_us(1000000);

    // detect the object by ping
    BLUE = 0;
    matrix_shot.start();
    xbee.printf("Detect the object");
    while(matrix_shot.read()<5){
        parallax_ping  ping1(pin10);
        if((float)ping1>62){xbee.printf("Triangle\r\n");}
        else if((float)ping1>50 && (float)ping1<54){xbee.printf("Square\r\n");}
        else if((float)ping1>56 && (float)ping1<59){xbee.printf("Right Triangle\r\n");}
        else {xbee.printf("Teeth\r\n");}
    }
    matrix_shot.reset();
    BLUE = 1;

    // leave the region
    car.turn(-100, -0.1); wait_us(1500); xbee.printf("Reverse right(face to exit)\r\n"); car.stop();
    encoder0.reset();
    GREEN = 0; car.goStraight(100);
    while(encoder0.get_cm()<50) {wait_us(50000);xbee.printf("Go Straight\r\n");}
    GREEN = 1;

    car.turn(100, -0.1); wait_us(1500); xbee.printf("Turn right\r\n"); car.stop();
    encoder0.reset();
    GREEN = 0; car.goStraight(100);
    while(encoder0.get_cm()<100) {wait_us(50000);xbee.printf("Go Straight(to exit)\r\n");}
    GREEN = 1; car.stop(); wait_us(50000); xbee.printf("Leave\r\n");
    
    while(1){car.goStraight(0);}
}