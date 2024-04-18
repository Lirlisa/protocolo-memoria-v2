#include <memory_allocator/memory_allocator.hpp>
#include <memory_allocator/memory_handler/memory_handler.hpp>
#include <global/memoria.hpp>
#include <mapa/mapa.hpp>
#include <Arduino.h>
#include <unordered_map>
#include <utility>
#include <LoRa.h>
#include <new>

using par_id_costo = std::pair<uint16_t, float>;
using tipo_tabla = std::unordered_map<
    uint16_t,
    float,
    std::hash<uint16_t>,
    std::equal_to<uint16_t>,
    CAllocator<par_id_costo>
>;
using par_id_vecinos = std::pair<uint16_t, tipo_tabla>;
using tipo_grafo = std::unordered_map<
    uint16_t,
    tipo_tabla,
    std::hash<uint16_t>,
    std::equal_to<uint16_t>,
    CAllocator<par_id_vecinos>
>;

void setup() {
    Serial.begin(9600);
    while (!Serial) {};
    Serial.println("Serial iniciado");
    if (!LoRa.begin(915E6)) {
        Serial.println("Starting LoRa failed!");
        while (1);
    }
    Serial.println("Flag 1");
    Memory_allocator& memoria = Memoria::memoria;
    Serial.print("Memoria disponible: ");
    Serial.println(memoria.available_memory());
    Serial.print("Bloques usados: ");
    Serial.println(memoria.get_cantidad_bloques());
    Serial.print("Handlers usados: ");
    Serial.println(memoria.get_cantidad_handlers());
    Serial.println("----------");

    Memory_handler& mapa_handler = memoria.acquire<Mapa>(sizeof(Mapa), 420);
    Serial.print("Handler mapa_handler válido? ");
    Serial.println(mapa_handler.es_valido() ? "Sí" : "No");
    Serial.println("Flag 2");
    mapa_handler.get_elem<Mapa>()->actualizar_propias_probabilidades(5);
    mapa_handler.get_elem<Mapa>()->actualizar_propias_probabilidades(4);
    mapa_handler.get_elem<Mapa>()->actualizar_propias_probabilidades(4);

    unsigned cant_bytes = mapa_handler.get_elem<Mapa>()->get_size_vector_probabilidad();
    Memory_handler& vector_prob_handler = memoria.acquire<uint8_t>(cant_bytes);
    mapa_handler.get_elem<Mapa>()->obtener_vector_probabilidad(vector_prob_handler);
    uint16_t total_pares;
    std::memcpy(&total_pares, vector_prob_handler.get_elem<uint8_t>(), sizeof(uint16_t));
    unsigned cant_mensajes = total_pares / Mensaje_vector::max_cantidad_pares_por_payload + 1;
    Memory_handler& mensaje_vector_handler = memoria.acquire_simple<Mensaje_vector>(sizeof(Mensaje_vector) * cant_mensajes);
    Memory_handler& payload_aux = memoria.acquire_simple<uint8_t>(Mensaje::payload_max_size);
    uint16_t ultimo_byte_agregado = 1;
    uint8_t i;
    for (std::size_t mensaje_n = 0; mensaje_n < cant_mensajes; ++mensaje_n) {
        for (i = 0; i < Mensaje_vector::max_cantidad_pares_por_payload && 1 + i * (sizeof(float) + sizeof(uint16_t)) + ultimo_byte_agregado < cant_bytes; ++i) {
            std::memcpy(
                payload_aux.get_elem<uint8_t>() + 1 + i * (sizeof(float) + sizeof(uint16_t)),
                vector_prob_handler.get_elem<uint8_t>() + ultimo_byte_agregado + 1 + i * (sizeof(float) + sizeof(uint16_t)),
                sizeof(uint16_t)
            );
            std::memcpy(
                payload_aux.get_elem<uint8_t>() + 1 + i * (sizeof(float) + sizeof(uint16_t)) + sizeof(uint16_t),
                vector_prob_handler.get_elem<uint8_t>() + ultimo_byte_agregado + 1 + i * (sizeof(float) + sizeof(uint16_t)) + sizeof(uint16_t),
                sizeof(float)
            );
        }
        *payload_aux.get_elem<uint8_t>() = i;
        ultimo_byte_agregado += i * (sizeof(float) + sizeof(uint16_t));
        new (mensaje_vector_handler.get_elem<Mensaje_vector>()) Mensaje_vector(1, 2, 3, 5, payload_aux, 1 + i * (sizeof(float) + sizeof(uint16_t)));
    }
    memoria.release<uint8_t>(vector_prob_handler);
    memoria.release<uint8_t>(payload_aux);
    Serial.println("Flag 3");
    Serial.print("Memoria disponible: ");
    Serial.println(memoria.available_memory());
    Serial.print("Bloques usados: ");
    Serial.println(memoria.get_cantidad_bloques());
    Serial.print("Handlers usados: ");
    Serial.println(memoria.get_cantidad_handlers());

    cant_bytes = mensaje_vector_handler.get_elem<Mensaje_vector>()->get_transmission_size();
    Memory_handler& transmission_handler = memoria.acquire<uint8_t>(cant_bytes);
    mensaje_vector_handler.get_elem<Mensaje_vector>()->parse_to_transmission(transmission_handler);
    Memory_handler& mensaje_handler = memoria.acquire_simple<Mensaje>(sizeof(Mensaje));
    new (mensaje_handler.get_elem<Mensaje>()) Mensaje(transmission_handler, cant_bytes);

    Serial.println("Flag 4");
    Serial.print("Memoria disponible: ");
    Serial.println(memoria.available_memory());
    Serial.print("Bloques usados: ");
    Serial.println(memoria.get_cantidad_bloques());
    Serial.print("Handlers usados: ");
    Serial.println(memoria.get_cantidad_handlers());

    Memory_handler& mensaje_vector_handler2 = memoria.acquire_simple<Mensaje_vector>(sizeof(Mensaje_vector));
    new (mensaje_vector_handler2.get_elem<Mensaje_vector>()) Mensaje_vector(mensaje_handler);
    uint8_t cant_pares = mensaje_vector_handler2.get_elem<Mensaje_vector>()->get_cantidad_pares();

    Memory_handler& mapa_handler2 = memoria.acquire<Mapa>(sizeof(Mapa), 30);
    Serial.print("Handler mapa_handler válido? ");
    Serial.println(mapa_handler2.es_valido() ? "Sí" : "No");
    Serial.println("Flag 5");
    mapa_handler2.get_elem<Mapa>()->actualizar_propias_probabilidades(1);
    mapa_handler2.get_elem<Mapa>()->actualizar_propias_probabilidades(2);
    mapa_handler2.get_elem<Mapa>()->actualizar_propias_probabilidades(3);
    mapa_handler2.get_elem<Mapa>()->actualizar_probabilidades(
        420,
        *mensaje_vector_handler2.get_elem<Mensaje_vector>()->get_pares(),
        cant_pares
    );
    mapa_handler2.get_elem<Mapa>()->print();
    Serial.println("Flag 6");
    Serial.print("Memoria disponible: ");
    Serial.println(memoria.available_memory());
    Serial.print("Bloques usados: ");
    Serial.println(memoria.get_cantidad_bloques());
    Serial.print("Handlers usados: ");
    Serial.println(memoria.get_cantidad_handlers());

    memoria.release<Mapa>(mapa_handler);
    memoria.release<Mapa>(mapa_handler2);
    memoria.release<Mensaje_vector>(mensaje_vector_handler);
    memoria.release<Mensaje_vector>(mensaje_vector_handler2);
    memoria.release<uint8_t>(transmission_handler);
    memoria.release<Mensaje>(mensaje_handler);
    memoria.force_defragmentation();
    Serial.println("Flag 7");
    Serial.print("Memoria disponible: ");
    Serial.println(memoria.available_memory());
    Serial.print("Bloques usados: ");
    Serial.println(memoria.get_cantidad_bloques());
    Serial.print("Handlers usados: ");
    Serial.println(memoria.get_cantidad_handlers());
}

void loop() {
    // put your main code here, to run repeatedly:

}
