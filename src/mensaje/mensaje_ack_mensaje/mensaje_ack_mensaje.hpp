#ifndef MENSAJE_ACK_MENSAJE_HPP
#define MENSAJE_ACK_MENSAJE_HPP

#include <global/memoria.hpp>
#include <mensaje/mensaje/mensaje.hpp>
#include <memory_allocator/memory_allocator.hpp>
#include <memory_allocator/memory_handler/memory_handler.hpp>
#include <cstdint>
#include <vector>


class Mensaje_ack_mensaje : public Mensaje {
private:

    uint8_t cantidad_acks = 0;
public:
    struct from_msg final {};
    struct from_msg_ack_msg final {};

    Mensaje_ack_mensaje();
    Mensaje_ack_mensaje(
        uint32_t _ttr, uint16_t _emisor, uint16_t _receptor, uint16_t _nonce,
        Memory_handler& payload_externo_handler, int _payload_size
    );
    Mensaje_ack_mensaje(from_msg, Memory_handler& handler_msg);
    Mensaje_ack_mensaje(from_msg_ack_msg, Memory_handler& handler_msg_ack_msg);

    ~Mensaje_ack_mensaje();

    static const uint8_t ack_size = 3 * sizeof(uint16_t);
    void obtener_acks(Memory_handler& buffer) const;
    uint8_t obtener_cantidad_acks() const;
};

class Mensaje_ack_mensaje_generator {
private:
    Memory_allocator& memoria = Memoria::memoria;
    Memory_handler& fuente_handler; // No liberar, fuente externa (handler de Mensaje_ack_mensaje)
    Memory_handler& arreglo_acks_handler;
    uint32_t ttr;
    uint16_t emisor;
    uint16_t receptor;
    std::size_t pos_ack_actual;
    std::size_t pos_ack_final;
    std::size_t cantidad_acks_restantes;
public:
    Mensaje_ack_mensaje_generator(
        Mensaje_ack_mensaje* _fuente, uint32_t _ttr, uint16_t _emisor,
        uint16_t _receptor, Memory_handler& _fuente_handler, Memory_handler& _arreglo_acks_handler,
        std::size_t _cantidad_acks
    );

    std::size_t next();
};

#endif