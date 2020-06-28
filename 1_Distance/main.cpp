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
EventQueue queue1(32 * EVENTS_EVENT_SIZE);
EventQueue queue(32 * EVENTS_EVENT_SIZE);
Thread xbee_get;
Thread pingled;
char buf1[1000] = {0};
char s[21];

BBCar car(pin8, pin9, servo_ticker);

void xbee_rx_interrupt(void);
void xbee_rx(void);


int main() {
    wait(3);
    BLUE = 1;
    // Setup a serial interrupt function of receiving data from xbee
    xbee_get.start(callback(&queue, &EventQueue::dispatch_forever));
    xbee.attach(xbee_rx_interrupt, Serial::RxIrq);
    
    // enter //
    parallax_encoder encoder0(pin3, encoder_ticker);
    encoder0.reset();
    GREEN = 0; car.goStraight(100); //enter maze
    while(encoder0.get_cm()<100) {wait_us(50000); xbee.printf("Go Straight\r\n");} //Have try: 100
    GREEN = 1; car.stop(); wait_us(50000); xbee.printf("Wait for a while\r\n");
    
    // calibrate by matrix(save shot)//
    BLUE = 0;
    uart.baud(9600);
    
    xbee.printf(s,"matrix");
    uart.puts(s);
    xbee.printf("send\r\n");
    wait(0.5);
    matrix_shot.start();
    while(uart.readable() && matrix_shot.read()<10){
        for( int i = 0;;i++){
            char recv = uart.getc();
            if (recv == '\r'){
                break;
            }
            buf1[i] = pc.putc(recv);
        }
        xbee.printf("%s\r\n",buf1);
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
    BLUE = 1;
    xbee.printf(s,"number");
    uart.puts(s);
    xbee.printf("send\r\n");
    wait(0.5);
    matrix_shot.start();
    while(uart.readable() && matrix_shot.read()<10){
        for( int i = 0;;i++){
            char recv = uart.getc();
            if (recv == '\r'){
                break;
            }
            buf1[i] = pc.putc(recv);
        }
        xbee.printf("%s\r\n",buf1);
    }
    matrix_shot.reset();
    BLUE = 0;
    
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
    xbee.printf(s,"matrix");
    uart.puts(s);
    xbee.printf("send\r\n");
    wait(0.5);
    matrix_shot.start();
    while(uart.readable() && matrix_shot.read()<10){
        for( int i = 0;;i++){
            char recv = uart.getc();
            if (recv == '\r'){
                break;
            }
            buf1[i] = pc.putc(recv);
        }
        xbee.printf("%s\r\n",buf1);
    }
    matrix_shot.reset();
    BLUE = 1;
    wait_us(1000000);

    // detect the object by ping
    BLUE = 0;
    matrix_shot.start();
    xbee.printf("Detect the object");
    while(matrix_shot.read()<10){
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
}

void xbee_rx_interrupt(void)
{
  xbee.attach(NULL, Serial::RxIrq); // detach interrupt
  queue.call(&xbee_rx);
}

void xbee_rx(void)
{
  char buf[100] = {0};
  while(xbee.readable()){
    for (int i=0; ; i++) {
      char recv = xbee.getc();
      if (recv == '\r') {
         break;
      }
      buf[i] = pc.putc(recv);
    }
    xbee.printf("%s\r\n", buf);
    wait(0.1);
  }
  xbee.attach(xbee_rx_interrupt, Serial::RxIrq); // reattach interrupt
}