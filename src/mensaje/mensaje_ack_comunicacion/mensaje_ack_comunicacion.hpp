#ifndef MENSAJE_ACK_COMUNICACION_HPP
#define MENSAJE_ACK_COMUNICACION_HPP

#include <mensaje/mensaje/mensaje.hpp>
#include <memory_allocator/memory_handler/memory_handler.hpp>
#include <cstdint>

class Mensaje_ack_comunicacion : public Mensaje {
private:
    uint16_t nonce_mensaje_original;

    const static int raw_message_max_size = 15;
    const static int payload_max_size = 6;
public:
    struct from_msg_ack_com final {};
    struct from_msg final {};

    Mensaje_ack_comunicacion();
    Mensaje_ack_comunicacion(
        uint32_t _ttr, uint16_t _emisor, uint16_t _receptor,
        uint16_t _nonce, uint16_t nonce_msj_original
    );
    Mensaje_ack_comunicacion(from_msg_ack_com, Memory_handler& handler_msg_ack_com);
    Mensaje_ack_comunicacion(from_msg, Memory_handler& handler_msg_origen);
    ~Mensaje_ack_comunicacion();

    bool confirmar_ack(uint16_t nonce_original, uint16_t emisor_original, uint16_t receptor_original) const;
    void print();

    void update(
        uint32_t _ttr, uint16_t _emisor, uint16_t _receptor,
        uint16_t _nonce, uint16_t _nonce_mensaje_original
    );

    uint16_t get_nonce_original();

    void update(const Mensaje& mensaje);
};

#endif