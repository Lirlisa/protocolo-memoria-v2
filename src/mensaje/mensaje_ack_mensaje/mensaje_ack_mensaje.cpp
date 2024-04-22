#include <mensaje/mensaje/mensaje.hpp>
#include <mensaje/mensaje_ack_mensaje/mensaje_ack_mensaje.hpp>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <Arduino.h>

Mensaje_ack_mensaje::Mensaje_ack_mensaje() : Mensaje() {
    tipo_payload = Mensaje::PAYLOAD_ACK_MENSAJE;
    cantidad_acks = payload_handler.get_elem<uint8_t>()[0];
}

Mensaje_ack_mensaje::Mensaje_ack_mensaje(
    uint32_t _ttr, uint16_t _emisor, uint16_t _receptor, uint16_t _nonce,
    Memory_handler& payload_externo_handler, int _payload_size
) : Mensaje(_ttr, _emisor, _receptor, _nonce, Mensaje::PAYLOAD_ACK_MENSAJE, payload_externo_handler, _payload_size) {
    cantidad_acks = payload_handler.get_elem<uint8_t>()[0];
}

/*
@brief Hay que asegurarse de que el payload tenga el formato correcto
*/
Mensaje_ack_mensaje::Mensaje_ack_mensaje(from_msg, Memory_handler& handler_msg) : Mensaje(handler_msg) {
    tipo_payload = Mensaje::PAYLOAD_ACK_MENSAJE;
    cantidad_acks = payload_handler.get_elem<uint8_t>()[0];
}


Mensaje_ack_mensaje::Mensaje_ack_mensaje(
    from_msg_ack_msg, Memory_handler& handler_msg_ack_msg
) : Mensaje_ack_mensaje(
    handler_msg_ack_msg.get_elem<Mensaje_ack_mensaje>()->getTTR(),
    handler_msg_ack_msg.get_elem<Mensaje_ack_mensaje>()->getEmisor(),
    handler_msg_ack_msg.get_elem<Mensaje_ack_mensaje>()->getReceptor(),
    handler_msg_ack_msg.get_elem<Mensaje_ack_mensaje>()->getNonce(),
    handler_msg_ack_msg.get_elem<Mensaje_ack_mensaje>()->payload_handler,
    handler_msg_ack_msg.get_elem<Mensaje_ack_mensaje>()->getPayloadSize()
) {
    cantidad_acks = payload_handler.get_elem<uint8_t>()[0];
}


Mensaje_ack_mensaje::~Mensaje_ack_mensaje() { }

/*
@brief Almacena en el arreglo de uint8_t todos los ack (cada ack tiene largo 6 bytes)
*/
void Mensaje_ack_mensaje::obtener_acks(Memory_handler& buffer) const {
    for (uint8_t i = 0; i < cantidad_acks; i++) {
        std::memcpy(
            buffer.get_elem<uint8_t>() + i * ack_size,
            payload_handler.get_elem<uint8_t>() + sizeof(uint8_t) + i * ack_size,
            sizeof(uint16_t)
        );
        std::memcpy(
            buffer.get_elem<uint8_t>() + i * ack_size + sizeof(uint16_t),
            payload_handler.get_elem<uint8_t>() + sizeof(uint8_t) + i * ack_size + sizeof(uint16_t),
            sizeof(uint16_t)
        );
        std::memcpy(
            buffer.get_elem<uint8_t>() + i * ack_size + 2 * sizeof(uint16_t),
            payload_handler.get_elem<uint8_t>() + sizeof(uint8_t) + i * ack_size + 2 * sizeof(uint16_t),
            sizeof(uint16_t)
        );
    }
}

uint8_t Mensaje_ack_mensaje::obtener_cantidad_acks() const {
    return cantidad_acks;
}

Mensaje_ack_mensaje_generator::Mensaje_ack_mensaje_generator(
    uint32_t _ttr, uint16_t _emisor, uint16_t _receptor,
    Memory_handler& _fuente_handler, Memory_handler& _arreglo_acks_handler,
    std::size_t _cantidad_acks
) : fuente_handler(_fuente_handler), arreglo_acks_handler(_arreglo_acks_handler) {
    ttr = _ttr;
    emisor = _emisor;
    receptor = _receptor;
    pos_ack_actual = 0;
    pos_ack_final = _cantidad_acks - 1;
    cantidad_acks_restantes = _cantidad_acks;
}

/*
@brief Modifica la fuente entregada en el constructor para que el mensaje contenga los nuevos ack.
@warning El nonce no es válido.
@returns La cantidad de acks restantes.
*/
std::size_t Mensaje_ack_mensaje_generator::next() {
    if (cantidad_acks_restantes == 0 || pos_ack_actual >= pos_ack_final) return 0;

    uint8_t acks_por_enviar = std::min((unsigned)(Mensaje::payload_max_size - 1) / Mensaje_ack_mensaje::ack_size, cantidad_acks_restantes);;
    Memory_handler& handler_payload = memoria.acquire_simple<uint8_t>(acks_por_enviar * Mensaje_ack_mensaje::ack_size);
    uint8_t* _payload = handler_payload.get_elem<uint8_t>();
    uint8_t* arreglo_acks = arreglo_acks_handler.get_elem<uint8_t>();
    std::size_t pos_byte_ack;
    _payload[0] = acks_por_enviar;
    for (uint8_t i = 0; i < acks_por_enviar; i++) {
        pos_byte_ack = (pos_ack_actual + i) * Mensaje::payload_max_size;
        std::memcpy(
            _payload + 1 + i * Mensaje_ack_mensaje::ack_size,
            arreglo_acks + pos_byte_ack,
            sizeof(uint16_t)
        );
        std::memcpy(
            _payload + 1 + i * Mensaje_ack_mensaje::ack_size + sizeof(uint16_t),
            arreglo_acks + pos_byte_ack + sizeof(uint16_t),
            sizeof(uint16_t)
        );
        std::memcpy(
            _payload + 1 + i * Mensaje_ack_mensaje::ack_size + 2 * sizeof(uint16_t),
            arreglo_acks + pos_byte_ack + 2 * sizeof(uint16_t),
            sizeof(uint16_t)
        );
    }
    pos_ack_actual += acks_por_enviar;
    cantidad_acks_restantes -= acks_por_enviar;
    fuente_handler.get_elem<Mensaje_ack_mensaje>()->update(
        ttr, emisor, receptor, 0, Mensaje::PAYLOAD_ACK_MENSAJE, handler_payload, 1 + acks_por_enviar * Mensaje_ack_mensaje::ack_size
    );
    memoria.release<uint8_t>(handler_payload);
    return cantidad_acks_restantes;
}