#include <mensaje/mensaje/mensaje.hpp>
#include <mensaje/mensaje_textos_vistos/mensaje_textos_vistos.hpp>
#include <algorithm>
#include <cstring>

Mensaje_textos_vistos::Mensaje_textos_vistos() : Mensaje() {
    tipo_payload = Mensaje::PAYLOAD_TEXTO_VISTO;
}

Mensaje_textos_vistos::Mensaje_textos_vistos(
    uint32_t _ttr, uint16_t _emisor, uint16_t _receptor, uint16_t _nonce,
    Memory_handler& payload_externo_handler, int _payload_size
) : Mensaje(_ttr, _emisor, _receptor, _nonce, Mensaje::PAYLOAD_TEXTO_VISTO, payload_externo_handler, _payload_size) { }

Mensaje_textos_vistos::Mensaje_textos_vistos(from_msg, Memory_handler& handler_msg) : Mensaje(handler_msg) {
    tipo_payload = Mensaje::PAYLOAD_TEXTO_VISTO;
}

Mensaje_textos_vistos::Mensaje_textos_vistos(from_msg_txt_vistos, Memory_handler& handler_txt_visto_msg) : Mensaje_textos_vistos(
    handler_txt_visto_msg.get_elem<Mensaje_textos_vistos>()->getEmisor(),
    handler_txt_visto_msg.get_elem<Mensaje_textos_vistos>()->getTTR(),
    handler_txt_visto_msg.get_elem<Mensaje_textos_vistos>()->getReceptor(),
    handler_txt_visto_msg.get_elem<Mensaje_textos_vistos>()->getNonce(),
    handler_txt_visto_msg.get_elem<Mensaje_textos_vistos>()->payload_handler,
    handler_txt_visto_msg.get_elem<Mensaje_textos_vistos>()->getPayloadSize()
) { }


Mensaje_textos_vistos::~Mensaje_textos_vistos() { }

/*
@brief Almacena en el buffer de uint8_t todos los hashes (cada hash tiene largo 6 bytes)
@warning El buffer debe poder almacenar la cantidad de bytes asociadas.
*/
void Mensaje_textos_vistos::obtener_hashes(Memory_handler& buffer) const {
    uint8_t cantidad_hashes = payload_handler.get_elem<uint8_t>()[0];
    for (uint8_t i = 0; i < cantidad_hashes; i++) {
        std::memcpy(
            buffer.get_elem<uint8_t>() + i * hash_size,
            payload_handler.get_elem<uint8_t>() + sizeof(uint8_t) + i * hash_size,
            sizeof(uint16_t)
        );
        std::memcpy(
            buffer.get_elem<uint8_t>() + i * hash_size + sizeof(uint16_t),
            payload_handler.get_elem<uint8_t>() + sizeof(uint8_t) + i * hash_size + sizeof(uint16_t),
            sizeof(uint16_t)
        );
        std::memcpy(
            buffer.get_elem<uint8_t>() + i * hash_size + 2 * sizeof(uint16_t),
            payload_handler.get_elem<uint8_t>() + sizeof(uint8_t) + i * hash_size + 2 * sizeof(uint16_t),
            sizeof(uint16_t)
        );
    }
}

Mensaje_textos_vistos_generator::Mensaje_textos_vistos_generator(
    uint32_t _ttr, uint16_t _emisor, uint16_t _receptor,
    Memory_handler& _fuente_handler, Memory_handler& _arreglo_hashes_handler,
    std::size_t _cantidad_hashes
) : fuente_handler(_fuente_handler), arreglo_hashes_handler(_arreglo_hashes_handler) {
    ttr = _ttr;
    emisor = _emisor;
    receptor = _receptor;
    pos_hash_actual = 0;
    pos_hash_final = _cantidad_hashes - 1;
    cantidad_hashes_restantes = _cantidad_hashes;
}

std::size_t Mensaje_textos_vistos_generator::next() {
    if (cantidad_hashes_restantes == 0 || pos_hash_actual >= pos_hash_final) return 0;

    uint8_t acks_por_enviar = std::min((unsigned)(Mensaje::payload_max_size - 1) / Mensaje_textos_vistos::hash_size, cantidad_hashes_restantes);;
    Memory_handler& handler_payload = memoria.acquire_simple<uint8_t>(1 + acks_por_enviar * Mensaje_textos_vistos::hash_size);
    uint8_t* _payload = handler_payload.get_elem<uint8_t>();
    uint8_t* arreglo_hashes = arreglo_hashes_handler.get_elem<uint8_t>();
    std::size_t pos_byte_hash;
    _payload[0] = acks_por_enviar;
    for (uint8_t i = 0; i < acks_por_enviar; i++) {
        pos_byte_hash = (pos_hash_actual + i) * Mensaje::payload_max_size;
        std::memcpy(
            _payload + 1 + i * Mensaje_textos_vistos::hash_size,
            arreglo_hashes + pos_byte_hash,
            sizeof(uint16_t)
        );
        std::memcpy(
            _payload + 1 + i * Mensaje_textos_vistos::hash_size + sizeof(uint16_t),
            arreglo_hashes + pos_byte_hash + sizeof(uint16_t),
            sizeof(uint16_t)
        );
        std::memcpy(
            _payload + 1 + i * Mensaje_textos_vistos::hash_size + 2 * sizeof(uint16_t),
            arreglo_hashes + pos_byte_hash + 2 * sizeof(uint16_t),
            sizeof(uint16_t)
        );
    }
    pos_hash_actual += acks_por_enviar;
    cantidad_hashes_restantes -= acks_por_enviar;
    fuente_handler.get_elem<Mensaje_textos_vistos>()->update(
        ttr, emisor, receptor, 0, Mensaje::PAYLOAD_TEXTO_VISTO, handler_payload, 1 + acks_por_enviar * Mensaje_textos_vistos::hash_size
    );
    memoria.release<uint8_t>(handler_payload);
    return cantidad_hashes_restantes;
}