Feito por Guilherme Melo e Matheus Gushi.

Este projeto configura um ambiente integrado para controlar um dispositivo ESP32 simulado no Wokwi, utilizando um servidor MQTT personalizado hospedado em uma máquina virtual (VM) na Microsoft Azure. O código permite que o ESP32 se conecte a este servidor MQTT, onde ele publica e recebe mensagens para controlar suas funcionalidades, como ligar ou desligar e ajustar a luminosidade. A comunicação é gerenciada através do protocolo MQTT e o controle do dispositivo é realizado via comandos enviados por meio do Postman. O Docker é utilizado para orquestrar os serviços necessários na VM, simplificando a configuração e manutenção do ambiente. Com este setup, é possível simular, testar e controlar dispositivos IoT remotamente, fornecendo uma plataforma versátil para desenvolvimento e experimentação de soluções baseadas em IoT.

Foram usados no projeto:

Docker em uma Virtual Machine aberta no Microsoft Azure.
Postman com projeto disponibilizado pelo professor.
Simulador Wokwi.
