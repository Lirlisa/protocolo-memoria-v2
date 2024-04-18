

#include <mensaje/mensaje/mensaje.hpp>
#include <mensaje/mensaje_vector/mensaje_vector.hpp>
#include <cstdint>
#include <Arduino.h>
#include <cstring>
#include <utility>

Mensaje_vector::Mensaje_vector() : Mensaje() {
    tipo_payload = Mensaje::PAYLOAD_VECTOR;
}

Mensaje_vector::Mensaje_vector(
    uint32_t _ttr, uint16_t _emisor, uint16_t _receptor,
    uint16_t _nonce, Memory_handler& payload_externo_handler,
    int payload_size
) : Mensaje(
    _ttr, _emisor, _receptor, _nonce, Mensaje::PAYLOAD_VECTOR, payload_externo_handler, payload_size
) {
    init_pares();
}

Mensaje_vector::Mensaje_vector(Memory_handler& mensaje_origen) : Mensaje(mensaje_origen) {
    init_pares();
}

Mensaje_vector::~Mensaje_vector() {
    if (handler_pares != nullptr && cantidad_pares > 0)
        memoria.release<par_costo_id>(*handler_pares, cantidad_pares);
    handler_pares = nullptr;
}

void Mensaje_vector::init_pares() {
    cantidad_pares = *payload_handler.get_elem<uint8_t>();
    Serial.print("Cantidad pares en mensaje vector: ");
    Serial.println(cantidad_pares);
    handler_pares = &memoria.acquire_simple<par_costo_id>(sizeof(par_costo_id) * cantidad_pares);
    uint8_t offset = sizeof(uint8_t), * payload = payload_handler.get_elem<uint8_t>();
    par_costo_id* pares = handler_pares->get_elem<par_costo_id>();
    uint16_t id_nodo;
    float peso;
    Serial.println("----- init pares -----");
    for (uint8_t i = 0; i < cantidad_pares; ++i) {
        std::memcpy(&id_nodo, payload + offset + i * par_costo_id_size, sizeof(uint16_t));
        std::memcpy(&peso, payload + offset + i * par_costo_id_size + sizeof(uint16_t), sizeof(float));
        Serial.print("id: ");
        Serial.println(id_nodo);
        Serial.print("peso: ");
        Serial.println(peso);
        new (pares + i) par_costo_id({ peso, id_nodo });
    }
    Serial.println("----- Fin init pares -----");
}

Memory_handler* Mensaje_vector::get_pares() const {
    return handler_pares;
}

void Mensaje_vector::print(unsigned int cant_bytes) const {
    Mensaje::print(cant_bytes);
    Serial.print("Cantidad de pesos: ");
    Serial.println(cantidad_pares);
    par_costo_id* pares = handler_pares->get_elem<par_costo_id>();
    for (std::size_t i = 0; i < cantidad_pares; ++i) {
        Serial.print("Nodo ");
        Serial.print(pares[i].second);
        Serial.print(": ");
        Serial.println(pares[i].first);
    }
}

void Mensaje_vector::update(
    uint32_t _ttr, uint16_t _emisor, uint16_t _receptor,
    uint16_t _nonce, Memory_handler& payload_externo_handler,
    int _payload_size
) {
    ttr = _ttr;
    emisor = _emisor;
    receptor = _receptor;
    nonce = _nonce;
    tipo_payload = Mensaje_vector::PAYLOAD_VECTOR;
    payload_size = std::min((unsigned)_payload_size, payload_max_size);
    if (handler_pares != nullptr)
        memoria.release<par_costo_id>(*handler_pares, cantidad_pares);
    if (payload_size <= 0) return;
    std::memcpy(payload_handler.get_elem<uint8_t>(), payload_externo_handler.get_elem<uint8_t>(), payload_size);
    transmission_size = message_without_payload_size + payload_size;

    init_pares();
}

void Mensaje_vector::update(const Mensaje& mensaje) {
    update(
        mensaje.getTTR(), mensaje.getEmisor(), mensaje.getReceptor(), mensaje.getNonce(),
        mensaje.payload_handler, mensaje.getPayloadSize()
    );
}

uint8_t Mensaje_vector::get_cantidad_pares() const {
    return cantidad_pares;
}