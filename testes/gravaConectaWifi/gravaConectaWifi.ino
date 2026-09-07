#include <WiFi.h>
#include <Preferences.h>
#include <time.h> // Biblioteca nativa para lidar com o tempo

// Instancia o objeto Preferences
Preferences preferences;

String ssid = "";
String password = "";

// Configurações do Servidor NTP (Tempo)
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -3 * 3600; // Fuso horário UTC-3 (Brasil/Fortaleza) em segundos
const int   daylightOffset_sec = 0;    // Sem horário de verão atualmente no Brasil

// Função para imprimir a hora atual no monitor serial
void printLocalTime() {
  struct tm timeinfo;
  // getLocalTime() aguarda até que o relógio tenha sido sincronizado
  if(!getLocalTime(&timeinfo)){
    Serial.println("Falha ao obter a hora do servidor NTP.");
    return;
  }
  
  // Imprime no formato: Dia/Mês/Ano Hora:Minuto:Segundo
  Serial.print("Data/Hora atualizada: ");
  Serial.println(&timeinfo, "%d/%m/%Y %H:%M:%S");
}

void setup() {
  // Inicia a comunicação serial
  Serial.begin(115200);
  delay(1000);

  // Inicia a biblioteca Preferences com o namespace "wifi_config"
  preferences.begin("wifi_config", false);

  // Lê o SSID e a senha salvos na memória flash
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("pass", "");

  Serial.println("\n--- ESP32-C6 Wi-Fi & Time Manager ---");
  
  if (ssid != "") {
    Serial.print("Tentando conectar a rede: ");
    Serial.println(ssid);
    
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
      Serial.println("\nConectado com sucesso!");
      Serial.print("Endereço IP: ");
      Serial.println(WiFi.localIP());

      // ==========================================
      // CONFIGURAÇÃO DO RELÓGIO VIA INTERNET (NTP)
      // ==========================================
      Serial.println("\nSincronizando relogio via NTP...");
      
      // Inicia a sincronização de tempo em segundo plano
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      
      // Dá um pequeno tempo para a primeira sincronização acontecer
      delay(2000); 
      
      // Exibe a hora obtida
      printLocalTime();

    } else {
      Serial.println("\nFalha ao conectar. Verifique as credenciais salvas.");
    }
  } else {
    Serial.println("Nenhuma rede configurada na memoria.");
  }

  Serial.println("\n--- Comandos via Serial ---");
  Serial.println("Para gravar a rede, digite 'r' seguido do nome. Ex: rMinhaRedeWifi");
  Serial.println("Para gravar a senha, digite 's' seguido da senha. Ex: sMinhaSenha123");
}

void loop() {
  // Verifica se há dados chegando pela porta serial
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim(); 

    if (input.length() > 0) {
      char command = input.charAt(0);
      String value = input.substring(1); 

      // Se o comando for 'r' ou 'R', salva o SSID
      if (command == 'r' || command == 'R') {
        preferences.putString("ssid", value);
        Serial.print("SSID salvo na memoria FLASH com sucesso: ");
        Serial.println(value);
        Serial.println("Reinicie o ESP32 para conectar.");
      } 
      // Se o comando for 's' ou 'S', salva a senha
      else if (command == 's' || command == 'S') {
        preferences.putString("pass", value);
        Serial.print("Senha salva na memoria FLASH com sucesso: ");
        Serial.println(value);
        Serial.println("Reinicie o ESP32 para conectar.");
      } 
      else {
        Serial.println("Comando invalido. Use 'r' ou 's'.");
      }
    }
  }
}