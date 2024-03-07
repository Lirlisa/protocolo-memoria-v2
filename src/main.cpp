#include <memory_allocator/memory_allocator.hpp>
#include <memory_allocator/memory_handler/memory_handler.hpp>
#include <mensaje/mensaje/mensaje.hpp>
#include <mensaje/mensaje_texto/mensaje_texto.hpp>
#include <texto/texto.hpp>
#include <Arduino.h>
#include <memory>
#include <new>


void setup() {
    Serial.begin(9600);
    while (!Serial) {};
    Serial.println("Serial begin");

    Memory_allocator memoria(2000);
    Serial.println("Flag 1");
    Serial.println(memoria.get_cantidad_handlers());
    Serial.println(memoria.get_cantidad_bloques());
    Memory_handler& handler_payload = memoria.acquire<char>(212);
    char* payload = handler_payload.get_elem<char>();
    std::strcpy(payload, "Texto al límite?ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789{}[]?.,;:-_+*~'\"|°!#$%&/()=ABCDEFGABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789{}[]?.,;:-_+*~'\"|°!#$%&/()=ABCDEFG");

    Memory_handler& handler_texto = memoria.acquire<Texto>(sizeof(Texto), 1, 2, 3, 4, 212, handler_payload, memoria);
    Serial.print("Pos Texto: ");
    Serial.println((uintptr_t)handler_texto.get_elem<Texto>(), HEX);

    std::size_t payload_size = handler_texto.get_elem<Texto>()->transmission_size();
    Memory_handler& handler_texto_transmision = memoria.acquire<uint8_t>(payload_size);
    handler_texto.get_elem<Texto>()->parse_to_transmission(handler_texto_transmision);
    Memory_handler& handler_mensaje_texto = memoria.acquire<Mensaje_texto>(
        sizeof(Mensaje_texto),
        1, 2, 3,
        4, handler_texto_transmision, payload_size,
        memoria
    );
    memoria.release<uint8_t>(handler_texto_transmision, payload_size);
    memoria.release<Texto>(handler_texto);
    memoria.release<char>(handler_payload, 212);
    Serial.println("----- Mensaje inicial -----");
    handler_mensaje_texto.get_elem<Mensaje_texto>()->print();
    Serial.println("----- Fin Mensaje inicial -----");
    memoria.force_defragmentation();

    payload_size = handler_mensaje_texto.get_elem<Mensaje_texto>()->get_transmission_size();
    Memory_handler& handler_mensaje_transmision = memoria.acquire<uint8_t>(payload_size);
    handler_mensaje_texto.get_elem<Mensaje_texto>()->parse_to_transmission(handler_mensaje_transmision);
    Memory_handler& handler_mensaje_desde_transmision = memoria.acquire<Mensaje>(
        sizeof(Mensaje),
        handler_mensaje_transmision,
        payload_size,
        memoria
    );
    Memory_handler& handler_mensaje_texto_desde_transmision = memoria.acquire<Mensaje_texto>(
        sizeof(Mensaje_texto),
        handler_mensaje_desde_transmision,
        memoria
    );
    memoria.release<uint8_t>(handler_mensaje_transmision);
    memoria.release<Mensaje>(handler_mensaje_desde_transmision);
    Serial.println("----- Mensaje Final -----");
    handler_mensaje_texto_desde_transmision.get_elem<Mensaje_texto>()->print();
    Serial.println("----- Fin Mensaje Final xd -----");
}

void loop() {
    // put your main code here, to run repeatedly:

}
