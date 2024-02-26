#ifndef MEMORY_ALLOCATOR
#define MEMORY_ALLOCATOR

#include <memory_allocator/memory_handler/memory_handler.hpp>
#include <memory_allocator/memory_allocator.hpp>
#include <new>
#include <cstring>
#include <algorithm>
#include <cstdint>
#include <Arduino.h>


template <std::size_t N>
class Memory_allocator {
private:
    const static std::size_t max_size = N, capacidad_handlers = N / 4;
    std::size_t bytes_ocupados = 0, cantidad_handlers = 0, cantidad_bloques = 0, memoria_disponible = max_size;
    uint8_t buffer[max_size];
    Block_pointer bloques_reservados[capacidad_handlers];
    Memory_handler handlers[capacidad_handlers], handler_invalido;

    void move_memory(uint8_t* dest, uint8_t* src, std::size_t count) {
        Serial.println("Flag move_memory 1");
        if (dest == src) return;
        Serial.println("Flag move_memory 2");
        std::memmove(dest, src, count);
        Serial.println("Flag move_memory 3");
    }
    void desfragmentar() {
        Serial.println("Flag desfragmentar 1");
        uint8_t* ancla = buffer, * dir_bloque;
        std::size_t block_size;
        Serial.println("Flag desfragmentar 2");
        std::sort(bloques_reservados, bloques_reservados + cantidad_bloques, _comparar_bloques);
        cantidad_bloques = cantidad_handlers;
        Serial.println("Flag desfragmentar 3");

        for (std::size_t i = 0; i < cantidad_bloques; ++i) {
            Serial.println("Flag desfragmentar 4");
            bloques_reservados[i].get_parent()->hacer_valido(bloques_reservados + i);
            Serial.println("Flag desfragmentar 5");
            dir_bloque = static_cast<uint8_t*>(bloques_reservados[i].get_data());
            Serial.println("Flag desfragmentar 6");
            block_size = bloques_reservados[i].get_block_size();
            Serial.println("Flag desfragmentar 7");
            if (ancla != dir_bloque) {
                Serial.println("Flag desfragmentar 8");
                move_memory(ancla, dir_bloque, block_size);
                Serial.println("Flag desfragmentar 9");
                bloques_reservados[i].set_data(static_cast<void*>(ancla));
                Serial.println("Flag desfragmentar 10");
            }
            Serial.println("Flag desfragmentar 11");
            ancla += block_size;
        }
        Serial.println("Flag desfragmentar 12");
        ptrdiff_t dist = static_cast<uint8_t*>(bloques_reservados[cantidad_bloques - 1].get_data()) - buffer;
        Serial.println("Flag desfragmentar 13");
        memoria_disponible = max_size - dist - bloques_reservados[cantidad_bloques - 1].get_block_size();
        Serial.println("Flag desfragmentar 14");
    }
    static bool _comparar_bloques(const Block_pointer& bloque1, const Block_pointer& bloque2) {
        return bloque2.get_parent() == nullptr || bloque1.get_parent() <= bloque2.get_parent();
    }
public:
    Memory_allocator() = default;
    ~Memory_allocator() = default;

    template <typename T, typename... Args>
    Memory_handler& acquire(Args... args) {
        Serial.println("Flag get_memory 1");
        if (cantidad_handlers >= capacidad_handlers) return handler_invalido;
        Serial.println("Flag get_memory 2");
        std::size_t cantidad_bytes = sizeof(T);
        if (memoria_disponible < cantidad_bytes) {
            Serial.println("Flag get_memory 3");
            if (bytes_ocupados + cantidad_bytes <= max_size) {
                Serial.println("Flag get_memory 4");
                desfragmentar();
            }
            else {
                Serial.println("Flag get_memory 5");
                return handler_invalido;
            }
            Serial.println("Flag get_memory 6");
        }
        Serial.println("Flag get_memory 7");
        // acá quedan handlers y hay memoria suficiente al final del buffer
        Memory_handler* handler_a_usar = nullptr;
        Serial.println("Flag get_memory 8");
        for (std::size_t i = 0; i < capacidad_handlers; ++i) {
            Serial.println("Flag get_memory 9");
            if (!handlers[i].es_valido()) {
                Serial.println("Flag get_memory 10");
                handler_a_usar = handlers + i;
                break;
            }
        }
        Serial.println("Flag get_memory 11");
        cantidad_handlers++;
        uint8_t* memoria_a_asignar;
        if (cantidad_bloques == 0)
            memoria_a_asignar = buffer;
        else
            memoria_a_asignar = static_cast<uint8_t*>(
                bloques_reservados[cantidad_bloques - 1].get_data()
                ) + bloques_reservados[cantidad_bloques - 1].get_block_size();
        Serial.println("Flag get_memory 12");
        Block_pointer* bloque_a_usar = bloques_reservados + cantidad_bloques++;
        Serial.println("Flag get_memory 13");
        handler_a_usar->hacer_valido(bloque_a_usar);
        Serial.println("Flag get_memory 14");
        bloque_a_usar->set_parent(handler_a_usar);
        Serial.println("Flag get_memory 15");
        bloque_a_usar->set_data(static_cast<void*>(memoria_a_asignar));
        Serial.println("Flag get_memory 16");
        bloque_a_usar->set_block_size(cantidad_bytes);
        Serial.println("Flag get_memory 17");
        Serial.print("cantidad_bloques = ");
        Serial.println(cantidad_bloques);
        new (memoria_a_asignar) T(args...);
        Serial.println("Flag get_memory 18");
        return *handler_a_usar;
    }
    void release(Memory_handler& handler) {
        Memory_handler* pos = &handler;
        if (pos < handlers || handlers + capacidad_handlers < pos) return; // este handler no es de acá
        handler.get_child().anular();
        handler.hacer_invalido();
        cantidad_handlers--;
    }
};

#endif