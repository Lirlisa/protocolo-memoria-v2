#ifndef MENSAJE_VECTOR_HPP
#define MENSAJE_VECTOR_HPP

#include <mensaje/mensaje/mensaje.hpp>
#include <memory_allocator/memory_allocator.hpp>
#include <memory_allocator/memory_handler/memory_handler.hpp>
#include <cstdint>
#include <utility>

typedef std::pair<float, uint16_t> par_costo_id;

class Mensaje_vector : public Mensaje {
private:
    Memory_handler* handler_pares = nullptr;
    uint8_t cantidad_pares = 0;

    void init_pares();
public:
    Mensaje_vector();
    Mensaje_vector(
        uint32_t _ttr, uint16_t _emisor, uint16_t _receptor,
        uint16_t _nonce, Memory_handler& payload_externo_handler,
        int _payload_size
    );
    Mensaje_vector(Memory_handler& mensaje_origen);
    ~Mensaje_vector();
    const static unsigned par_costo_id_size = sizeof(float) + sizeof(uint16_t);
    const static unsigned max_cantidad_pares_por_payload = (payload_max_size - 1) / par_costo_id_size;


    Memory_handler* get_pares() const;

    void print(unsigned int cant_bytes = 6) const;

    void update(
        uint32_t _ttr, uint16_t _emisor, uint16_t _receptor,
        uint16_t _nonce, Memory_handler& payload_externo_handler,
        int payload_size
    );

    void update(const Mensaje& mensaje);

    uint8_t get_cantidad_pares() const;
};

#endif