#include <WiFi.h>
#include <Preferences.h>

// Instancia o objeto Preferences
Preferences preferences;

String ssid = "";
String password = "";

void setup() {
  // Inicia a comunicação serial
  Serial.begin(115200);
  delay(1000);

  // Inicia a biblioteca Preferences com o namespace "wifi_config"
  // O segundo parâmetro "false" indica que abriremos no modo Leitura/Escrita
  preferences.begin("wifi_config", false);

  // Lê o SSID e a senha salvos na memória flash
  // O segundo parâmetro ("") é o valor padrão caso a chave ainda não exista
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("pass", "");

  Serial.println("\n--- ESP32-C6 Wi-Fi Manager ---");
  
  if (ssid != "") {
    Serial.print("Tentando conectar a rede: ");
    Serial.println(ssid);
    
    // Inicia a tentativa de conexão
    WiFi.begin(ssid.c_str(), password.c_str());
    
    // Aguarda a conexão com um limite de tentativas para não travar o loop principal
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
    // Lê a string até detectar uma quebra de linha (Enter)
    String input = Serial.readStringUntil('\n');
    
    // Remove espaços em branco ou quebras de linha (como \r) no início e no fim
    input.trim(); 

    if (input.length() > 0) {
      // Separa o comando (primeiro caractere) do valor (resto da string)
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