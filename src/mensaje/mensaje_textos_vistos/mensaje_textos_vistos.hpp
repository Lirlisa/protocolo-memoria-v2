#ifndef MENSAJE_TEXTOS_VISTOS
#define MENSAJE_TEXTOS_VISTOS

#include <global/memoria.hpp>
#include <mensaje/mensaje/mensaje.hpp>
#include <memory_allocator/memory_allocator.hpp>
#include <memory_allocator/memory_handler/memory_handler.hpp>
#include <cstdint>


class Mensaje_textos_vistos : public Mensaje {
public:
    struct from_msg final {};
    struct from_msg_txt_vistos final {};

    static const uint8_t hash_size = 3 * sizeof(uint16_t);
    Mensaje_textos_vistos();
    Mensaje_textos_vistos(
        uint32_t _ttr, uint16_t _emisor, uint16_t _receptor, uint16_t _nonce,
        Memory_handler& payload_externo_handler, int _payload_size
    );
    Mensaje_textos_vistos(from_msg, Memory_handler& handler_msg);
    Mensaje_textos_vistos(from_msg_txt_vistos, Memory_handler& handler_txt_visto_msg);

    ~Mensaje_textos_vistos();

    void obtener_hashes(Memory_handler& buffer) const;
    uint8_t obtener_cantidad_hashes() const;
};

class Mensaje_textos_vistos_generator {
private:
    Memory_allocator& memoria = Memoria::memoria;
    Memory_handler& fuente_handler; // No liberar, fuente externa (handler de Mensaje_ack_mensaje)
    Memory_handler& arreglo_hashes_handler;
    uint32_t ttr;
    uint16_t emisor;
    uint16_t receptor;
    std::size_t pos_hash_actual;
    std::size_t pos_hash_final;
    std::size_t cantidad_hashes_restantes;
public:
    Mensaje_textos_vistos_generator(
        uint32_t _ttr, uint16_t _emisor, uint16_t _receptor,
        Memory_handler& _fuente_handler, Memory_handler& _arreglo_hashes_handler,
        std::size_t _cantidad_hashes
    );

    std::size_t next();
};

#endif