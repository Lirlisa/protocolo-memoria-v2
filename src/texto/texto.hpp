#ifndef TEXTO_HPP
#define TEXTO_HPP

#include <memory_allocator/memory_allocator.hpp>
#include <memory_allocator/memory_handler/memory_handler.hpp>
#include <unishox2.h>
#include <cstdint>
#include <cstring>
#include <Arduino.h>

struct Texto {
    uint16_t nonce = 0, creador = 0, destinatario = 0;
    uint8_t saltos = 0, largo_texto_comprimido = 0;
    unsigned largo_texto = 0;

    Memory_handler* contenido_comprimido_handler = nullptr;
    Memory_allocator& memoria;
    bool valido = false;

    const static unsigned max_largo_contenido_comprimido = 182;

    // tamaño de las variables al transmitir (sin contenido)
    const static unsigned size_variables_transmission = 8;
    const static unsigned max_size_transmision = max_largo_contenido_comprimido + size_variables_transmission;

    Texto(Memory_allocator& _memoria);
    Texto(
        uint16_t _nonce, uint16_t _creador, uint16_t _destinatario,
        uint8_t _saltos, int _largo_texto,
        Memory_handler& contenido_handler, Memory_allocator& _memoria, bool comprimido = false
    );
    ~Texto();

    Texto& operator=(const Texto& other);

    void parse_to_transmission(Memory_handler& destino_handler) const;
    uint64_t hash() const;
    uint8_t transmission_size() const;

    void print();

    bool operator==(const Texto& texto) const;
    bool operator!=(const Texto& texto) const;
    bool es_valido();

    void get_contenido(Memory_handler& buffer_handler) const;

    void update(
        uint16_t _nonce, uint16_t _creador, uint16_t _destinatario,
        uint8_t _saltos, int _largo_texto,
        Memory_handler& contenido_handler, bool comprimido
    );
    void update(const Texto* texto);
};

#endif