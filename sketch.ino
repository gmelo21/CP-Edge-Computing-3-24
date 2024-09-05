#include <WiFi.h>            // Biblioteca para funcionalidade Wi-Fi
#include <PubSubClient.h>    // Biblioteca para comunicação MQTT

// Definindo variáveis de configuração padrão
const char* default_SSID = "Wokwi-GUEST";        // SSID do Wi-Fi
const char* default_PASSWORD = "";               // Senha do Wi-Fi
const char* default_BROKER_MQTT = "191.235.32.167"; // IP do Broker MQTT
const int default_BROKER_PORT = 1883;            // Porta do Broker MQTT
const char* default_TOPIC_SUBSCRIBE = "/TEF/lampEDGE1/cmd";   // Tópico MQTT para assinatura
const char* default_TOPIC_PUBLISH_1 = "/TEF/lampEDGE1/attrs"; // Tópico MQTT para publicar o estado
const char* default_TOPIC_PUBLISH_2 = "/TEF/lampEDGE1/attrs/l"; // Tópico MQTT para publicar a luminosidade
const char* default_ID_MQTT = "fiware_EDGE1";    // ID do cliente MQTT
const int default_PIN = 2;                       // GPIO para o LED onboard
const char* topicPrefix = "lampEDGE1";           // Prefixo para os tópicos MQTT

// Variáveis configuráveis para Wi-Fi, MQTT e pinos
char* SSID = const_cast<char*>(default_SSID);
char* PASSWORD = const_cast<char*>(default_PASSWORD);
char* BROKER_MQTT = const_cast<char*>(default_BROKER_MQTT);
int BROKER_PORT = default_BROKER_PORT;
char* TOPIC_SUBSCRIBE = const_cast<char*>(default_TOPIC_SUBSCRIBE);
char* TOPIC_PUBLISH_1 = const_cast<char*>(default_TOPIC_PUBLISH_1);
char* TOPIC_PUBLISH_2 = const_cast<char*>(default_TOPIC_PUBLISH_2);
char* ID_MQTT = const_cast<char*>(default_ID_MQTT);
int PIN = default_PIN;

WiFiClient espClient;        // Cliente Wi-Fi para ESP32
PubSubClient MQTT(espClient); // Cliente MQTT
char OutputState = '0';      // Estado do LED onboard

// Inicializa a comunicação serial
void initSerial() {
    Serial.begin(115200);
}

// Inicializa a conexão Wi-Fi
void initWiFi() {
    delay(10);
    Serial.println("------Conexão Wi-Fi------");
    Serial.print("Conectando à rede: ");
    Serial.println(SSID);
    Serial.println("Por favor, aguarde");
    reconnectWiFi(); // Tentativa de reconectar ao Wi-Fi
}

// Inicializa a conexão MQTT com configurações do servidor
void initMQTT() {
    MQTT.setServer(BROKER_MQTT, BROKER_PORT); // Define o broker MQTT e a porta
    MQTT.setCallback(mqtt_callback);          // Define a função de callback para o MQTT
}

// Função principal de configuração
void setup() {
    initOutput();    // Inicializa o output do LED onboard
    initSerial();    // Inicia a comunicação serial
    initWiFi();      // Conecta ao Wi-Fi
    initMQTT();      // Conecta ao broker MQTT
    delay(5000);     // Aguarda para estabilizar as conexões
    MQTT.publish(TOPIC_PUBLISH_1, "s|on"); // Publica o estado inicial para o broker MQTT
}

// Função principal de loop, executa continuamente
void loop() {
    checkWiFiAndMQTTConnections(); // Verifica e mantém as conexões Wi-Fi e MQTT
    sendOutputStateToMQTT();       // Envia o estado atual do LED para o MQTT
    handleLuminosity();            // Lê e publica dados de luminosidade
    MQTT.loop();                   // Processa mensagens MQTT recebidas
}

// Reconecta ao Wi-Fi se estiver desconectado
void reconnectWiFi() {
    if (WiFi.status() == WL_CONNECTED)
        return;
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("Conectado com sucesso à rede ");
    Serial.print(SSID);
    Serial.println("Endereço IP obtido: ");
    Serial.println(WiFi.localIP());

    // Garante que o LED comece apagado
    digitalWrite(PIN, LOW);
}

// Função de callback MQTT para lidar com mensagens recebidas
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    String msg;
    for (int i = 0; i < length; i++) {
        char c = (char)payload[i];
        msg += c;
    }
    Serial.print("- Mensagem recebida: ");
    Serial.println(msg);

    // Cria padrões de tópicos para comparação
    String onTopic = String(topicPrefix) + "@on|";
    String offTopic = String(topicPrefix) + "@off|";

    // Liga ou desliga o LED com base na mensagem recebida
    if (msg.equals(onTopic)) {
        digitalWrite(PIN, HIGH); // Liga o LED
        OutputState = '1';       // Atualiza o estado
    }

    if (msg.equals(offTopic)) {
        digitalWrite(PIN, LOW);  // Desliga o LED
        OutputState = '0';       // Atualiza o estado
    }
}

// Verifica conexões Wi-Fi e MQTT, reconecta se necessário
void checkWiFiAndMQTTConnections() {
    if (!MQTT.connected())
        reconnectMQTT(); // Reconecta ao MQTT se desconectado
    reconnectWiFi();     // Reconecta ao Wi-Fi se desconectado
}

// Publica o estado atual do LED para o broker MQTT
void sendOutputStateToMQTT() {
    if (OutputState == '1') {
        MQTT.publish(TOPIC_PUBLISH_1, "s|on");
        Serial.println("- LED Ligado");
    }

    if (OutputState == '0') {
        MQTT.publish(TOPIC_PUBLISH_1, "s|off");
        Serial.println("- LED Desligado");
    }
    Serial.println("- Estado do LED onboard enviado para o broker!");
    delay(1000); // Pequeno atraso para estabilidade
}

// Inicializa o output do LED onboard e pisca durante a configuração
void initOutput() {
    pinMode(PIN, OUTPUT);  // Define o modo do pino para o LED
    digitalWrite(PIN, HIGH); // Inicialmente desliga o LED
    boolean toggle = false;

    // Pisca o LED 10 vezes para indicar inicialização
    for (int i = 0; i <= 10; i++) {
        toggle = !toggle;
        digitalWrite(PIN, toggle);
        delay(200);
    }
}

// Reconecta ao broker MQTT se a conexão for perdida
void reconnectMQTT() {
    while (!MQTT.connected()) {
        Serial.print("* Tentando conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) { // Tenta conectar usando o ID do cliente
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPIC_SUBSCRIBE); // Inscreve no tópico de comandos
        } else {
            Serial.println("Falha ao reconectar ao broker.");
            Serial.println("Tentando novamente em 2 segundos");
            delay(2000); // Espera antes de tentar novamente
        }
    }
}

// Lê dados de luminosidade de um sensor e publica no MQTT
void handleLuminosity() {
    const int potPin = 34;           // Define o pino analógico para o sensor
    int sensorValue = analogRead(potPin);  // Lê o valor analógico do sensor
    int luminosity = map(sensorValue, 0, 4095, 0, 100); // Mapeia o valor do sensor para uma porcentagem
    String message = String(luminosity);   // Converte o valor de luminosidade para uma string
    Serial.print("Valor de luminosidade: ");
    Serial.println(message.c_str());
    MQTT.publish(TOPIC_PUBLISH_2, message.c_str()); // Publica a luminosidade para o broker MQTT
}
