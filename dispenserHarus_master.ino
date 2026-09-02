/*
 * Dispenser Harus
 * Daniel Almeida Chagas SET/2026
 */


#include <Adafruit_PCF8574.h>


Adafruit_PCF8574 pcf1;
Adafruit_PCF8574 pcf2;
Adafruit_PCF8574 pcf3;

//módulos
bool modulo1 = false;
bool modulo2 = false;
bool modulo3 = false;

bool estadoLed = false;

//liberado para uso
bool liberado1 = false;
bool liberado2 = false;
bool liberado3 = false;

//status dos reles
bool rele1 = false;
bool rele2 = false;
bool rele3 = false;

//timers
double timerLeds = 0;
double timerStatus = 10000;

void setup() {
  Serial.begin(115200);
  Wire.begin(19, 20);
  delay(1000);
  while (!Serial) { delay(10); }
  mensagem("Dispenser Harus MVP");

  if (!pcf1.begin(0x20, &Wire)) {
    mensagem("FALHA! MOD 1 inativo!");
  } else {
    mensagem("Módulo 1 ativo");
    modulo1 = true;
    pcf1.pinMode(0, INPUT_PULLUP); //botão
    pcf1.pinMode(1, OUTPUT); //led
    pcf1.pinMode(2, OUTPUT); //relé
    pcf1.digitalWrite(2, HIGH);
  }
  if (!pcf2.begin(0x21, &Wire)) {
    mensagem("FALHA! MOD 2 inativo!");
  } else {
    mensagem("Módulo 2 ativo");
    modulo2 = true;
    pcf2.pinMode(0, INPUT_PULLUP);
    pcf2.pinMode(1, OUTPUT);
    pcf2.pinMode(2, OUTPUT);
    pcf2.digitalWrite(2, HIGH);
  }
  if (!pcf3.begin(0x22, &Wire)) {
    mensagem("FALHA! MOD 3 inativo!");
  } else {
    mensagem("Módulo 3 ativo");
    modulo3 = true;
    pcf3.pinMode(0, INPUT_PULLUP);
    pcf3.pinMode(1, OUTPUT);
    pcf3.pinMode(2, OUTPUT);
    pcf3.digitalWrite(2, HIGH);
  }
}

void loop() {
  if(timerLeds < millis()){
    if(estadoLed){
      timerLeds = millis() + 1900;
    } else {
      timerLeds = millis() + 100;
    }
    
    estadoLed = !estadoLed;
    if(modulo1) pcf1.digitalWrite(1,estadoLed);
    if(modulo2) pcf2.digitalWrite(1,estadoLed);
    if(modulo3) pcf3.digitalWrite(1,estadoLed);
  }
  if(timerStatus < millis()){
    timerStatus = millis() + 10000;
    printStatus();
  }

  /*
  if(pcf1.digitalRead(0)==0){
    if(liberado1){
      rele1 = !rele1;
      mensagem("Acionando rele 1");
    } else {
      mensagem("Acesso negado ao Mod1!");
    }
    delay(250);
  }
  
  
  if(modulo1) pcf1.digitalWrite(2,rele1);
  if(modulo2) pcf2.digitalWrite(2,rele2);
  if(modulo3) pcf3.digitalWrite(2,rele3);
  */
}

void printStatus(){
  mensagem("Status:");
  if(modulo1) mensagem("Mod1 OK!");
  if(modulo2) mensagem("Mod2 OK!");
  if(modulo3) mensagem("Mod3 OK!");
}

void mensagem(String msg){
  Serial.println(msg);
}
