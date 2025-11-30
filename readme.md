# 🪴 Sistema de Monitoreo de Planta IoT con ESP32 y Discord

##  Project Summary

This project describes the development of a **Plant Monitoring System** based on the **ESP32** microcontroller, functioning as an Internet of Things (IoT) solution. Its primary goal is to address the challenge of maintaining optimal environmental conditions for sensitive plant species by providing automated vigilance and **real-time alerts** to the user via the **Discord** platform.

The system continuously measures critical variables such as temperature, humidity, and light intensity, and immediately notifies of any adverse condition that requires external intervention.

## Resumen del Proyecto

Este proyecto consiste en un **Sistema de Monitoreo de Plantas** basado en el microcontrolador **ESP32**. Su objetivo principal es resolver el desafío de mantener condiciones ambientales óptimas para especies vegetales sensibles, proporcionando una vigilancia automatizada y **alertas en tiempo real** al usuario a través de la plataforma **Discord**.

El sistema mide continuamente variables críticas como la temperatura, la humedad y la intensidad lumínica, y notifica inmediatamente cualquier condición adversa que requiera intervención externa.

## 📺 Demo del Proyecto

Puedes ver el funcionamiento en detalle y la demostración en el siguiente enlace:

[Ver Video Demostrativo en YouTube](https://youtu.be/BPcWUMO_6Ww)

##  Características Principales

* **Monitoreo Continuo:** Lectura de datos ambientales en ciclos definidos.
* **Microcontrolador:** Uso del **ESP32** por su capacidad de procesamiento y conectividad Wi-Fi integrada.
* **Alertas Instantáneas:** Envío de notificaciones detalladas a un canal de Discord (a través de Webhooks) cuando se detectan condiciones fuera de los rangos ideales.
* **Conectividad IoT:** Utiliza conexión Wi-Fi para la comunicación remota.
* **Indicadores Locales:** LEDs de estado para una rápida visualización local de la temperatura y humedad.
* **Sistema Abierto:** Mide, compara y alerta; la corrección del error depende de una intervención externa (no tiene capacidad de auto-corrección).

## 🛠️ Componentes de Hardware

| Componente | Función |
| :--- | :--- |
| **Microcontrolador** | ESP32 |
| **Sensor de Temperatura/Humedad** | DHT11/DHT22 |
| **Sensor de Luz** | LDR (Resistencia Dependiente de la Luz) |
| **Indicadores de Estado** | LEDs Rojos y Verdes (para Temperatura y Humedad) |

## 💻 Requisitos de Software y Dependencias

Para la correcta ejecución del código y la comunicación, se requieren las siguientes librerías y configuración:

1.  **Arduino IDE **
2.  **Librerías :**
    * `WiFi.h` (Para la conectividad Wi-Fi)
    * `DHT.h` (Para el sensor de temperatura y humedad)
    * `WiFiClientSecure.h` (Crucial para asegurar la conexión HTTPS requerida por los Webhooks de Discord)
3.  **Servicio Externo:**
    * Configuración de un **Webhook de Discord** para recibir las alertas.

## Desafíos y Soluciones Implementadas

Durante el desarrollo, se presentaron algunas complicaciones que fueron resueltas de la siguiente manera:

| Complicación Detectada | Solución Aplicada |
| :--- | :--- |
| **Discord rechazaba peticiones de Webhook.** | Se incluyó la librería `WiFiClientSecure.h` y se utilizó el método `client.setInsecure()` para manejar correctamente las conexiones **HTTPS** requeridas por Discord. |
| **Lectura inversa del sensor LDR.** | Se **invirtió la lógica de comparación** en el código para que las alertas correspondan al bajo nivel de luz. |
| **Múltiples alertas se solapaban en Discord.** | Se implementó una **variable acumuladora** para recopilar todas las alertas detectadas en cada ciclo, enviando un **único mensaje completo**, evitando el spam de notificaciones. |
| **Disponibilidad de sensores en el simulador.** | Se adaptó el código para usar el **DHT22** en lugar del DHT11. |

## 🎓 Contexto Académico

Este proyecto se desarrolló como parte de la evaluación para la asignatura:
Tecnologías para la automatización de la carrera Ingeniería en Sistemas de Información en Universidad Tecnológica Nacional (UTN) - Facultad Regional Concepción del Uruguay

## 👥 Integrantes del Grupo

* Emmanuel Davezac
* Esteban Gay
* Felipe Palazzi
* Mauricio Nahuel Salto