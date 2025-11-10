#include <DHT.h>           // Librería para interactuar con el sensor DHT (Temperatura y Humedad)
#include <WiFi.h>          // Librería para la conexión WiFi
#include <HTTPClient.h>    // Librería para realizar peticiones HTTP (necesaria para el Webhook)
#include <WiFiClientSecure.h> // Librería para manejar conexiones HTTPS (seguras) como la de Discord

// =================================================================
// 1. CONFIGURACIÓN DE RED Y WEBHOOK
// =================================================================
#define SSID "Wokwi-GUEST"                  // Nombre (SSID) de la red WiFi a la que se conectará el ESP32
#define PASSWORD ""                         // Contraseña de la red (vacía para Wokwi-GUEST)
// URL completa del Webhook de Discord para enviar notificaciones de alerta
#define DISCORD_WEBHOOK_URL "https://discord.com/api/webhooks/1420131501483491501/z-Hdgn5SIQ-4id5jkWvKb98r2-RxKyiOobjE0X7uc441p3MkwR4PPk13mJN2Gyu6-DPh" 

// =================================================================
// 2. CONFIGURACIÓN DE HARDWARE Y LÓGICA
// =================================================================
const byte LED_ROJO_PIN = 4;        // Pin del LED Rojo: Indica estado crítico de TEMPERATURA
const byte LED_VERDE_PIN = 2;       // Pin del LED Verde: Indica estado OK de TEMPERATURA
const byte LED_ROJO2_PIN = 5;       // Pin del segundo LED Rojo: Indica estado crítico de HUMEDAD
const byte LED_VERDE2_PIN = 17;     // Pin del segundo LED Verde: Indica estado OK de HUMEDAD
const byte DHT_SENSOR_PIN = 16;     // Pin de datos donde está conectado el sensor DHT
const byte LDR_PIN= 34;             // Pin analógico donde está conectado el sensor de luz (LDR)
#define DHT_SENSOR_TYPE DHT22       // Especificación del tipo de sensor DHT utilizado (DHT22)

// Inicialización del objeto DHT, pasándole el pin y el tipo de sensor
DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE); 

// LÍMITES CRÍTICOS PARA LA PLANTA 
const float TEMP_CRITICA_BAJA = 10.0;    // Temperatura mínima tolerable para la planta (°C)
const float TEMP_CRITICA_ALTA = 20.0;    // Temperatura máxima tolerable para la planta (°C)
const float HUMEDAD_CRITICA_BAJA = 35.0; // Humedad relativa mínima tolerable para la planta (%)
const float PORCENTAJE_LUZ_PERMITIDO=50; // Umbral de luz definido (50% de la luz máxima)
// Cálculo del valor analógico que corresponde al 50% de luz. 
// El ESP32 tiene un ADC de 12 bits, por lo que el valor máximo es 4095.
const float VALOR_MAXIMO_LUZ_PERMITIDO = (4095.0 / 100.0) * PORCENTAJE_LUZ_PERMITIDO;

// Intervalos de tiempo de lectura
const long INTERVALO_LECTURA_M = 0.15;      // Frecuencia de lectura en minutos (0.15 min = 9 segundos)
const long INTERVALO_LECTURA_S = INTERVALO_LECTURA_M *60; // Conversión del intervalo a segundos
const long INTERVALO_LECTURA_MS = INTERVALO_LECTURA_S*1000;// Conversión del intervalo a milisegundos para usar en delay()


// =================================================================
// FUNCIÓN PARA ENVIAR NOTIFICACIONES A NUESTRO CANAL DE DISCORD
// =================================================================
void enviarMensajeDiscord(String mensaje) {
    if (WiFi.status() == WL_CONNECTED) { // Verifica si hay conexión WiFi
        WiFiClientSecure client; 
        client.setInsecure(); // Permite conexiones SSL/TLS sin validación estricta de certificado (útil en entornos como Wokwi)
        HTTPClient http;
        http.begin(client, DISCORD_WEBHOOK_URL); // Inicializa la conexión HTTP al Webhook
        http.setConnectTimeout(30000);           // Configura un tiempo de espera para la conexión (30 segundos)
        http.addHeader("Content-Type", "application/json"); // Especifica que el cuerpo del mensaje es JSON
        
        // Construye el payload JSON con el mensaje a enviar
        String payload = "{\"content\": \"" + mensaje + "\"}"; 
        
        Serial.print(F("Enviando a Discord: "));
        Serial.println(payload);

        int httpCode = http.POST(payload); // Envía la petición POST

        if (httpCode > 0) {
            if (httpCode == 204) { // Código 204 No Content: Respuesta exitosa de Discord para Webhooks
                Serial.println(F("Mensaje enviado con éxito."));
            } else {
                Serial.printf("[HTTP] Código de respuesta: %d\n", httpCode);
                Serial.println(http.getString());
            }
        } else {
            // Manejo de errores de conexión HTTP
            Serial.printf("[HTTP] Falló la conexión, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end(); // Cierra la conexión HTTP
    } else {
        Serial.println(F("No hay conexión WiFi para enviar el mensaje."));
    }
}


// =================================================================
// INICIALIZACION
// =================================================================
void setup() {
    Serial.begin(115200); // Inicializa la comunicación serial a 115200 baudios
    Serial.println(F("Sistema de monitoreo de planta iniciado"));
    delay(100);
    
    // Configuración de los pines de LED como SALIDA
    pinMode(LED_ROJO_PIN, OUTPUT);
    pinMode(LED_VERDE_PIN, OUTPUT);
    pinMode(LED_ROJO2_PIN, OUTPUT);
    pinMode(LED_VERDE2_PIN, OUTPUT);
    
    // Configuración del pin LDR como ENTRADA (analógica por defecto en ESP32, pero buena práctica declararlo)
    pinMode(LDR_PIN, INPUT);
    
    dht_sensor.begin(); // Inicializa el sensor DHT

    // Conexión a Internet
    Serial.print(F("Conectando a red: "));
    Serial.println(SSID);
    WiFi.begin(SSID, PASSWORD); // Intenta conectar a la red
    while (WiFi.status() != WL_CONNECTED) { // Bucle de espera hasta que la conexión se establezca
        delay(500);
        Serial.print(".");
    }
    Serial.println(F("¡Conexión Wi-Fi establecida!"));
    
    // Envía un mensaje de inicio a Discord confirmando el funcionamiento
    enviarMensajeDiscord("Sistema de monitoreo de planta iniciado");
    delay(2000); 
}

// =================================================================
// PROGRAMA PRINCIPAL
// =================================================================
void loop() {
    // Lectura de datos del sensor DHT
    float humedad = dht_sensor.readHumidity();
    float temperatura = dht_sensor.readTemperature();
    String mensajeAlerta = ""; // Cadena para acumular los mensajes de alerta si las condiciones son críticas

    // 1. Verificación de lectura del sensor (errores comunes del DHT)
    if (isnan(temperatura) || isnan(humedad)) {
        Serial.println(F("Error al leer datos del Sensor DHT"));
        // Enciende todos los LEDs (o parpadean, etc.) para indicar un fallo de lectura del sensor
        digitalWrite(LED_ROJO_PIN, HIGH);
        digitalWrite(LED_VERDE_PIN, HIGH); 
        digitalWrite(LED_ROJO2_PIN, HIGH);
        digitalWrite(LED_VERDE2_PIN, HIGH); 
    } else {
        // 2. Lógica de Alertas y Acumulación de Mensajes
        
        // Verificar Temperatura Baja
        if (temperatura < TEMP_CRITICA_BAJA) {
            mensajeAlerta += "**TEMPERATURA BAJA:** T=" + String(temperatura, 1) + "°C (Mín. permitida:" + String(TEMP_CRITICA_BAJA) + "°C). ";
        } 
        // Verificar Temperatura Alta (el 'else if' asegura que no se evalúen las dos condiciones a la vez)
        else if (temperatura > TEMP_CRITICA_ALTA) {
            
            // Lógica combinada: Temperatura Alta + Luz
            // El valor del LDR es inversamente proporcional a la luz (mayor luz = menor valor analógico).
            if(analogRead(LDR_PIN) < VALOR_MAXIMO_LUZ_PERMITIDO) { 
                // La planta está muy iluminada (posiblemente al sol) y hace calor
                mensajeAlerta = mensajeAlerta + "**TEMPERATURA ALTA Y PLANTA EN EL SOL:** T=" + String(temperatura, 1) + "°C (Máx.permitida: " + String(TEMP_CRITICA_ALTA) + "°C). POR FAVOR ALEJE LA PLANTA DEL SOL ";
            }
            else {
                // La planta tiene temperatura alta, pero no por exceso de luz
                mensajeAlerta = mensajeAlerta + "**TEMPERATURA ALTA:** T=" + String(temperatura, 1) + "°C (Máx. permitida: " + String(TEMP_CRITICA_ALTA) + "°C). ";
            }
        }

        // Verificar Humedad Baja (esta verificación es independiente de las de temperatura)
        if (humedad < HUMEDAD_CRITICA_BAJA) {
            mensajeAlerta = mensajeAlerta + "**HUMEDAD BAJA:** H=" + String(humedad, 1) + "% (Mín. permitida: " + String(HUMEDAD_CRITICA_BAJA) + "%). ";
        }

        // 3. Control de LEDs y Envío de Discord
        if (mensajeAlerta.length() > 0) { // Si hay alguna alerta acumulada
            // CONDICIÓN MALA (Activación de LEDs Rojos)
            
            // Control de LEDs de TEMPERATURA
            if ((temperatura < TEMP_CRITICA_BAJA) || (temperatura > TEMP_CRITICA_ALTA) ) {
                digitalWrite(LED_ROJO_PIN, HIGH); // Rojo ON
                digitalWrite(LED_VERDE_PIN, LOW);  // Verde OFF
            }
            // NOTA: Si la temperatura está OK, pero hay alerta por humedad, los LEDs de Temp quedan en el estado anterior. 
            // Esto se corrige en la sección 'CONDICIÓN BUENA'.
            
            // Control de LEDs de HUMEDAD
            if (humedad < HUMEDAD_CRITICA_BAJA) {
                digitalWrite(LED_ROJO2_PIN, HIGH); // Rojo ON
                digitalWrite(LED_VERDE2_PIN, LOW); // Verde OFF
            }
            // Importante: Si la humedad está OK, el código entra en el 'else' y los LEDs se ponen en verde.
            else { 
                digitalWrite(LED_ROJO2_PIN, LOW);
                digitalWrite(LED_VERDE2_PIN, HIGH);
            }
            
            Serial.print(F("ALERTA : "));
            Serial.println(mensajeAlerta);
            enviarMensajeDiscord("ALERTA DE PLANTA: " + mensajeAlerta); // Envía el mensaje de alerta a Discord
        } else {
            // CONDICIÓN BUENA (No hay alertas, LEDs Verdes ON)
            
            // LEDs de TEMPERATURA OK
            digitalWrite(LED_ROJO_PIN, LOW);
            digitalWrite(LED_VERDE_PIN, HIGH);
            
            // LEDs de HUMEDAD OK
            digitalWrite(LED_ROJO2_PIN, LOW);
            digitalWrite(LED_VERDE2_PIN, HIGH);
            
            Serial.print(F("Condiciones OK"));
        }
        
        // Impresión de las lecturas actuales por el Serial Monitor
        Serial.print(F(" T: "));
        Serial.print(temperatura, 1);
        Serial.print(F("°C, H: "));
        Serial.print(humedad, 1);
        Serial.println(F("%"));
    }
    
    // Espera antes de la próxima lectura (controla la frecuencia de muestreo)
    delay(INTERVALO_LECTURA_MS); 
}