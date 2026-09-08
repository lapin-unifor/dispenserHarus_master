/*
 * Dispenser Harus
 * Daniel Almeida Chagas SET/2026
 */


#include <Adafruit_PCF8574.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <Preferences.h>
#include <time.h>
#include <Adafruit_PN532.h>

LiquidCrystal_I2C lcd(0x26,20,4);
String terminal1 = "";
String terminal2 = "";
String terminal3 = "";

Adafruit_PCF8574 pcf1;
Adafruit_PCF8574 pcf2;
Adafruit_PCF8574 pcf3;

char caractere = ' ';
String txtRecebido = "";

bool tituloLcd = false;

//módulos
bool modulo1 = false;
bool modulo2 = false;
bool modulo3 = false;

bool estadoLed = false;
bool estadoLedPiscando = false;

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
double timerLedPiscando = 0;
double timerStatus = 10000;
int tempoStatus = 10000;
double timerLcd = 10000;
double timerHora = 0;
double timerLiberacao = 0; //define o tempo que uma liberação pode ocorrer
bool emLiberacao = false;
int tempoLiberacao = 12000;

// Instancia o objeto Preferences
Preferences preferences;

String ssid = "";
String password = "";
bool ipConectado = false;

// Configurações do Servidor NTP (Tempo)
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -3 * 3600; // Fuso horário UTC-3 (Brasil/Fortaleza) em segundos
const int   daylightOffset_sec = 0;    // Sem horário de verão atualmente no Brasil
bool horaConfigurada = false;
struct tm timeinfo;

//config RFID
#define PN532_IRQ   (2)
#define PN532_RESET (3)

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

void setup() {
  Serial.begin(115200);
  Wire.begin(19, 20);
  //Wire.begin(8, 9);
  lcd.init();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("  Harus Tecnologia  ");
  lcd.setCursor(0,1);
  lcd.print("Iniciando...");
  delay(1000);
  // Inicia a biblioteca Preferences com o namespace "wifi_config"
  preferences.begin("wifi_config", false);

  // Lê o SSID e a senha salvos na memória flash
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("pass", "");
  //while (!Serial) { delay(10); }
  mensagem("Dispenser Harus MVP");

  
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    //        1---5----10---15--20
    mensagem("FALHA! RFID nao enc.");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

  mensagem("Leitor RFID OK!");

  if (ssid != "") {
    //        1---5----10---15--20
    mensagem("Conectando a rede");
    mensagem(ssid);
    
    // Inicia a tentativa de conexão
    WiFi.begin(ssid.c_str(), password.c_str());
    
    // Aguarda a conexão com um limite de tentativas
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      //        1---5----10---15--20
      mensagem("Conectado! IP:");
      mensagem(WiFi.localIP().toString());
      ipConectado = true;

      // ==========================================
      // CONFIGURAÇÃO DO RELÓGIO VIA INTERNET (NTP)
      // ==========================================
      //        1---5----10---15--20
      mensagem("Sinc. relogio NTP...");
      
      // Inicia a sincronização de tempo em segundo plano
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      
      // Dá um pequeno tempo para a primeira sincronização acontecer
      delay(2000); 
      
      // Exibe a hora obtida
      printLocalTime();

    } else {
      //        1---5----10---15--20
      mensagem("Falha ao conectar!");
      ipConectado = false;
    }
  } else {
    //        1---5----10---15--20
    mensagem("Nenhuma rede config.");
    ipConectado = false;
  }

  if (!pcf1.begin(0x20, &Wire)) {
    mensagem("FALHA! Disp1 inativo");
  } else {
    mensagem("Módulo Disp1 ativo");
    modulo1 = true;
    pcf1.pinMode(0, INPUT_PULLUP); //botão
    pcf1.pinMode(1, OUTPUT); //led
    pcf1.pinMode(2, OUTPUT); //relé
    pcf1.digitalWrite(2, HIGH);
  }
  if (!pcf2.begin(0x21, &Wire)) {
    mensagem("FALHA! Disp2 inativo");
  } else {
    mensagem("Módulo Disp2 ativo");
    modulo2 = true;
    pcf2.pinMode(0, INPUT_PULLUP);
    pcf2.pinMode(1, OUTPUT);
    pcf2.pinMode(2, OUTPUT);
    pcf2.digitalWrite(2, HIGH);
  }
  if (!pcf3.begin(0x22, &Wire)) {
    //        1---5----10---15--20
    mensagem("FALHA! Disp3 inativo");
  } else {
    mensagem("Módulo Disp3 ativo");
    modulo3 = true;
    pcf3.pinMode(0, INPUT_PULLUP);
    pcf3.pinMode(1, OUTPUT);
    pcf3.pinMode(2, OUTPUT);
    pcf3.digitalWrite(2, HIGH);
  }
  delay(3000);
  if(ipConectado) printIp();
}

void loop() {
  if(Serial.available()>0){
    caractere = Serial.read();
    if(caractere=='\n'){
      //executar o comando
      //Serial.print("Voce digitou ");
      //Serial.println(txtRecebido);
      maquinaDeEstados(txtRecebido); //chama a maq estados p executar
      txtRecebido = "";
    } else {
      txtRecebido = txtRecebido + caractere;
    }
  }
  
  if(timerHora< millis()){
    timerHora = millis() + 1000;
    if(horaConfigurada){
      printLocalTime();
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
    if(modulo2) pcf2.digitalWrite(1,estadoLed ^ liberado2);
    if(modulo3) pcf3.digitalWrite(1,estadoLed ^ liberado3);
  }

  if(timerLedPiscando < millis()){
    timerLedPiscando = millis() + 100;
    estadoLedPiscando = !estadoLedPiscando;
    if(rele1 && modulo1) pcf1.digitalWrite(1,estadoLedPiscando);
    if(rele2 && modulo2) pcf2.digitalWrite(1,estadoLedPiscando);
    if(rele3 && modulo3) pcf3.digitalWrite(1,estadoLedPiscando);
  }

  //timer de exibir o status do equipamento
  if(timerStatus < millis()){
    timerStatus = millis() + tempoStatus;
    printStatus();
    if(ipConectado) printIp();
  }

  //timer de mudar o titulo do LCD
  if(timerLcd < millis()){
    timerLcd = millis() + 10000;
    lcd.setCursor(0,0);
    lcd.print("                    ");
    lcd.setCursor(0,0);
    if(tituloLcd){
      lcd.print("  Harus Tecnologia  ");
    } else {
      //         1---5----10---15--20
      lcd.print("EcoDispenser MVP 1.0");
    }
    tituloLcd = !tituloLcd;
  }

  if(emLiberacao){
    if(timerLiberacao < millis()){
      maquinaDeEstados("x1");
      maquinaDeEstados("x2");
      maquinaDeEstados("x3");
      emLiberacao = false;
      printStatus();
    }
  }

  if(modulo1){
    if(pcf1.digitalRead(0)==0){
      if(liberado1){
        rele1 = !rele1;
        //        1---5----10---15--20
        mensagem("Acionando Disp1...");
      } else {
        //        1---5----10---15--20
        mensagem("Acesso negado Disp1!");
      }
      delay(250);
    }
  }
  
  if(modulo2){
    if(pcf2.digitalRead(0)==0){
      if(liberado2){
        rele2 = !rele2;
        //        1---5----10---15--20
        mensagem("Acionando Disp2...");
      } else {
        //        1---5----10---15--20
        mensagem("Acesso negado Disp2!");
      }
      delay(250);
    }
  }

  if(modulo3){
    if(pcf3.digitalRead(0)==0){
      if(liberado3){
        rele3 = !rele3;
        //        1---5----10---15--20
        mensagem("Acionando Disp3...");
      } else {
        //        1---5----10---15--20
        mensagem("Acesso negado Disp3!");
      }
      delay(250);
    }
  }
  
  if(modulo1) pcf1.digitalWrite(2,!rele1);
  if(modulo2) pcf2.digitalWrite(2,!rele2);
  if(modulo3) pcf3.digitalWrite(2,!rele3);

  
   //RFID NFC
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100);

  if (success) {
    // Display some basic information about the card
    mensagem("Lendo RFID...");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);

    if (uidLength == 4){
      // We probably have a Mifare Classic card ...
      uint32_t cardid = uid[0];
      cardid <<= 8;
      cardid |= uid[1];
      cardid <<= 8;
      cardid |= uid[2];
      cardid <<= 8;
      cardid |= uid[3];
      String numeroRfid = String(cardid);
      mensagem("RFID #" + numeroRfid);
      delay(1000);
      checaUsuario(cardid);
    }
    Serial.println("");
  }
}

void maquinaDeEstados(String texto){
  texto.trim();
  //Receber comandos, analisar e chamar as devidas funções
  String comando = texto.substring(0,1); //isto recebe o 1º caractere
  //Serial.println("Comando: " + comando);
  String parametro = texto.substring(1,texto.length());
  //Serial.println("Parametro: " + parametro);
  //executar os comandos e parâmetros
  switch(comando.charAt(0)){
    case 'l':
      //        1---5----10---15--20
    	mensagem("Liberado dispenser " + parametro);
    	if(parametro.toInt() == 1) liberado1 = true;
      if(parametro.toInt() == 2) liberado2 = true;
      if(parametro.toInt() == 3) liberado3 = true;
      emLiberacao = true;
      timerLiberacao = millis() + tempoLiberacao;
    	break;
    case 'x':
      //        1---5----10---15--20
    	mensagem("Travado dispenser " + parametro);
    	if(parametro.toInt() == 1) liberado1 = rele1 = false;
      if(parametro.toInt() == 2) liberado2 = rele2 = false;
      if(parametro.toInt() == 3) liberado3 = rele3 = false;
    	break;
    case 'r':
      //        1---5----10---15--20
    	mensagem("SSID:" + parametro);
      preferences.putString("ssid", parametro);
      Serial.print("SSID salvo na memoria FLASH com sucesso: ");
      Serial.println("Reinicie o ESP32 para conectar.");
    	break;
    case 's':
      //        1---5----10---15--20
    	mensagem("pass:" + parametro);
      preferences.putString("pass", parametro);
      Serial.print("Senha salva na memoria FLASH com sucesso: ");
      Serial.println("Reinicie o ESP32 para conectar.");
    	break;
    default:
      //        1---5----10---15--20
    	mensagem("Comando desconhecido");
  }
}

void checaUsuario(int identificador){
  delay(500);
  //cartões hard coded!
  if(identificador == 1704012831){
    maquinaDeEstados("l1"); //libera 1
    maquinaDeEstados("l3"); //libera 3
  }
  if(identificador == 2849453330){
    maquinaDeEstados("l2"); //libera 2
  }
  //aqui ficará o código de checar uma API via consulta HTTP
  printStatus();
}

void printStatus(){
  String statusTxt = "Status: ";
  if(modulo1){
    if(liberado1){
      statusTxt = statusTxt + "[L1]";
    } else {
      statusTxt = statusTxt + " D1 ";
    }
  } else statusTxt = statusTxt + " -- ";
  if(modulo2) {
    if(liberado2){
      statusTxt = statusTxt + "[L2]";
    } else {
      statusTxt = statusTxt + " D2 ";
    }
  } else statusTxt = statusTxt + " -- ";
  if(modulo3) {
    if(liberado3){
      statusTxt = statusTxt + "[L3]";
    } else {
      statusTxt = statusTxt + " D3 ";
    }
  } else {
    statusTxt = statusTxt + " -- ";
  }
  mensagem(statusTxt);
}

void mensagem(String msg){
  Serial.println(msg);
  if(ipConectado == false){
    lcd.setCursor(0,1);
    lcd.print("                    ");
    lcd.setCursor(0,2);
    lcd.print("                    ");
    terminal1 = terminal2;
    terminal2 = terminal3;
    
    lcd.setCursor(0,1);
    lcd.print(terminal1.substring(0,20));
    lcd.setCursor(0,2);
    lcd.print(terminal2.substring(0,20));
  }
  terminal3 = msg;
  lcd.setCursor(0,3);
  lcd.print("                    ");
  lcd.setCursor(0,3);
  lcd.print(terminal3.substring(0,20));
}

// Função para imprimir a hora atual no monitor serial
void printLocalTime() {
  // getLocalTime() aguarda até que o relógio tenha sido sincronizado
  if(!getLocalTime(&timeinfo)){
    //        1---5----10---15--20
    mensagem("Falha cfg data hora!");
    horaConfigurada = false;
    return;
  }
  
  // Imprime no formato: Dia/Mês/Ano Hora:Minuto:Segundo
  //        1---5----10---15--20
  //mensagem("Data/Hora atualizada");
  horaConfigurada = true;
  //Serial.println(&timeinfo, "%d/%m/%Y %H:%M:%S");
  char lcdBuffer[21]; // Buffer to hold the string (19 chars + null terminator)
  // Format the timeinfo struct into a string
  strftime(lcdBuffer, sizeof(lcdBuffer), "%d/%m/%Y  %H:%M:%S", &timeinfo);
  //lcd.setCursor(0, 1);
  //         1---5----10---15--20
  //lcd.print("                   ");
  lcd.setCursor(0, 1);
  lcd.print(lcdBuffer);
}

void printIp(){
  lcd.setCursor(0, 2);
  lcd.print("                    ");
  lcd.setCursor(3, 2);
  lcd.print(WiFi.localIP().toString());
}
