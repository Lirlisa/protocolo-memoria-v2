#include <memory_allocator/memory_allocator.hpp>
#include <memory_allocator/memory_handler/memory_handler.hpp>
#include <mensaje/mensaje/mensaje.hpp>
#include <Arduino.h>


void setup() {
    Serial.begin(9600);
    while (!Serial) {};
    Serial.println("Serial begin");
    Memory_allocator memoria(sizeof(Mensaje) + 2 * 191);
    Serial.println("Flag 1");
    Memory_handler& handler_payload = memoria.acquire<uint8_t>(3);
    uint8_t* payload = handler_payload.get_elem<uint8_t>();
    payload[0] = 10;
    payload[1] = 11;
    payload[2] = 12;
    Serial.println("Flag 2");
    uint8_t tipo = Mensaje::PAYLOAD_BEACON;
    Memory_handler& handler1 = memoria.acquire<Mensaje>(sizeof(Mensaje), 1, 2, 3, 4, tipo, handler_payload, 3, memoria);
    Serial.println(handler1.es_valido() ? "True" : "False");
    Mensaje* num1 = handler1.get_elem<Mensaje>();
    num1->print();
    Serial.println("Flag 3");
}

void loop() {
    // put your main code here, to run repeatedly:

}
