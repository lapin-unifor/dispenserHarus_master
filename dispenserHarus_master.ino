/*
 * Dispenser Harus
 * Daniel Almeida Chagas SET/2026
 */


#include <Adafruit_PCF8574.h>


Adafruit_PCF8574 pcf1;
Adafruit_PCF8574 pcf2;
Adafruit_PCF8574 pcf3;

char caractere = ' ';
String txtRecebido = "";

//módulos
bool modulo1 = false;
bool modulo2 = false;
bool modulo3 = false;

bool estadoLed = false;
bool estadoLedPiscando = false;

//liberado para uso
bool liberado1 = true;
bool liberado2 = false;
bool liberado3 = false;

//status dos reles
bool rele1 = false;
bool rele2 = false;
bool rele3 = false;

//timers
double timerLeds = 0;
double timerLedPiscando = 0;
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
  delay(3000);
}

void loop() {
  if(Serial.available()>0){
    caractere = Serial.read();
    if(caractere=='\n'){
      //executar o comando
      Serial.print("Voce digitou ");
      Serial.println(txtRecebido);
      maquinaDeEstados(txtRecebido); //chama a maq estados p executar
      txtRecebido = "";
    } else {
      txtRecebido = txtRecebido + caractere;
    }
  }
  if(timerLeds < millis()){
    if(estadoLed){
      timerLeds = millis() + 1900;
    } else {
      timerLeds = millis() + 100;
    }
    
    estadoLed = !estadoLed;
    if(modulo1) pcf1.digitalWrite(1,estadoLed ^ liberado1);
    if(modulo2) pcf2.digitalWrite(1,estadoLed ^ liberado1);
    if(modulo3) pcf3.digitalWrite(1,estadoLed ^ liberado1);
  }
  if(timerLedPiscando < millis()){
    timerLedPiscando = millis() + 100;
    estadoLedPiscando = !estadoLedPiscando;
    if(rele1 && modulo1) pcf1.digitalWrite(1,estadoLedPiscando);
    if(rele2 && modulo2) pcf2.digitalWrite(1,estadoLedPiscando);
    if(rele3 && modulo3) pcf3.digitalWrite(1,estadoLedPiscando);
  }
  if(timerStatus < millis()){
    timerStatus = millis() + 10000;
    printStatus();
  }

  
  if(pcf1.digitalRead(0)==0){
    if(liberado1){
      rele1 = !rele1;
      mensagem("Acionando rele 1");
    } else {
      mensagem("Acesso negado ao Mod1!");
    }
    delay(250);
  }
  
  
  if(modulo1) pcf1.digitalWrite(2,!rele1);
  if(modulo2) pcf2.digitalWrite(2,!rele2);
  if(modulo3) pcf3.digitalWrite(2,!rele3);
  
}

void maquinaDeEstados(String texto){
  //Receber comandos, analisar e chamar as devidas funções
  String comando = texto.substring(0,1); //isto recebe o 1º caractere
  Serial.println("Comando: " + comando);
  String parametro = texto.substring(1,texto.length());
  Serial.println("Parametro: " + parametro);
  //executar os comandos e parâmetros
  switch(comando.charAt(0)){
    case 'l':
    	mensagem("Liberando dispenser");
    	if(parametro.toInt() == 1) liberado1 = true;
      if(parametro.toInt() == 2) liberado2 = true;
      if(parametro.toInt() == 3) liberado3 = true;
    	break;
    case 'x':
    	mensagem("Travando dispenser");
    	if(parametro.toInt() == 1) liberado1 = rele1 = false;
      if(parametro.toInt() == 2) liberado2 = rele2 = false;
      if(parametro.toInt() == 3) liberado3 = rele3 = false;
    	break;
    default:
    	Serial.println("Comando desconhecido");
  }
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
