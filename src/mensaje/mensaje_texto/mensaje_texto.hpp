#ifndef MENSAJE_TEXTO_HPP
#define MENSAJE_TEXTO_HPP

#include <global/memoria.hpp>
#include <texto/texto.hpp>
#include <mensaje/mensaje/mensaje.hpp>
#include <memory_allocator/memory_allocator.hpp>
#include <memory_allocator/memory_handler/memory_handler.hpp>
#include <cstdint>
#include <cstring>
#include <vector>
#include <Arduino.h>

class Mensaje_texto : public Mensaje {
private:
    Memory_handler* arreglo_textos_handler = nullptr;
    std::size_t cantidad_textos = 0;
public:
    struct from_msg_texto final {};
    struct from_msg final {};

    Mensaje_texto();
    Mensaje_texto(
        uint32_t _ttr, uint16_t _emisor, uint16_t _receptor,
        uint16_t _nonce, Memory_handler& payload_externo_handler, int _payload_size
    );
    Mensaje_texto(from_msg_texto, Memory_handler& mensaje_texto_origen_handler);
    Mensaje_texto(from_msg, Memory_handler& mensaje_origen_handler);
    ~Mensaje_texto();

    void print();

    Memory_handler& obtener_textos();

    uint8_t get_cantidad_textos() const;

    void update(
        uint32_t _ttr, uint16_t _emisor, uint16_t _receptor,
        uint16_t _nonce, Memory_handler& payload_externo_handler, unsigned _payload_size
    );
    void update(const Mensaje_texto& origen);
    void update(const Mensaje& origen);
};

class Mensaje_texto_generator {
private:
    Memory_allocator& memoria = Memoria::memoria;
    Memory_handler& fuente_handler;
    Memory_handler& handler_arreglo_textos;
    uint32_t ttr;
    uint16_t emisor;
    uint16_t receptor;
    uint16_t nonce;
    std::size_t pos_texto_actual;
    std::size_t pos_texto_final;
    std::size_t cantidad_textos_restantes;
public:
    Mensaje_texto_generator(
        Memory_handler& _fuente_handler, uint32_t _ttr, uint16_t _emisor, uint16_t _receptor,
        uint16_t _nonce, Memory_handler& _handler_arreglo_textos, std::size_t _cantidad_textos
    );

    std::size_t next();
};

#endif