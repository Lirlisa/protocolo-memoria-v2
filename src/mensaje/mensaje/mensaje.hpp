#ifndef MENSAJE_HPP
#define MENSAJE_HPP

#include <memory_allocator/memory_allocator.hpp>
#include <memory_allocator/memory_handler/memory_handler.hpp>
#include <cstdint>


class Mensaje {
protected:
    uint32_t ttr = 0;
    uint16_t emisor = 0, receptor = 0, nonce = 0;
    uint8_t tipo_payload = 0, payload_size = 0;

    Memory_allocator& memoria;

    unsigned transmission_size = 0;
    const static unsigned message_without_payload_size = 9;
    const static uint16_t BROADCAST_CHANNEL_ID = 0xffff;
public:
    const static uint8_t PAYLOAD_TEXTO = 0;
    const static uint8_t PAYLOAD_VECTOR = 1;
    const static uint8_t PAYLOAD_ACK_MENSAJE = 2;
    const static uint8_t PAYLOAD_ACK_COMUNICACION = 3;
    const static uint8_t PAYLOAD_BEACON = 4;
    const static uint8_t PAYLOAD_TEXTO_VISTO = 5;
    const static uint8_t PAYLOAD_BEACON_CENTRAL = 6;

    const static unsigned payload_max_size = 191;
    const static unsigned raw_message_max_size = message_without_payload_size + payload_max_size;
    Memory_handler& payload_handler;

    Mensaje(Memory_allocator& _memoria);

    Mensaje(
        uint32_t _ttr, uint16_t _emisor, uint16_t _receptor,
        uint16_t _nonce, uint8_t _tipo_payload, Memory_handler& payload_externo_handler,
        unsigned _payload_size, Memory_allocator& _memoria
    );
    Mensaje(Memory_handler& mensaje_original_handler, Memory_allocator& _memoria);
    Mensaje(Memory_handler& data_handler, uint8_t largo_data, Memory_allocator& _memoria);
    virtual ~Mensaje();

    Mensaje& operator=(const Mensaje& other);
    bool operator!=(const Mensaje& other) const;
    bool operator==(const Mensaje& other) const;

    void parse_to_transmission(Memory_handler& destino_handler) const;

    void peek(unsigned cant_bytes = 6) const;
    void print(unsigned cant_bytes = 6) const;

    void setEmisor(uint16_t _emisor);
    void setReceptor(uint16_t _receptor);
    void setNonce(uint16_t _nonce);
    void setTTR(uint32_t _ttr);

    uint16_t getEmisor() const;
    uint16_t getReceptor() const;
    uint16_t getNonce() const;
    uint8_t getTipoPayload() const;
    uint32_t getTTR() const;
    uint8_t getPayloadSize() const;

    unsigned get_transmission_size() const;

    void update(
        uint32_t _ttr, uint16_t _emisor, uint16_t _receptor,
        uint16_t _nonce, uint8_t _tipo_payload, Memory_handler& payload_externo_handler,
        unsigned _payload_size
    );

    void update(
        uint32_t _ttr, uint16_t _emisor, uint16_t _receptor,
        uint16_t _nonce
    );

    void update(Memory_handler& data_handler, uint8_t largo_data);
    void update(const Mensaje& mensaje);
};

#endif