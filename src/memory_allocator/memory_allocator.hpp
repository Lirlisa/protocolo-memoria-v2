#ifndef MEMORY_ALLOCATOR
#define MEMORY_ALLOCATOR

#include <memory_allocator/memory_handler/memory_handler.hpp>
#include <memory_allocator/memory_allocator.hpp>
#include <new>
#include <cstring>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <utility>
#include <memory>
#include <new>
#include <Arduino.h>


class Memory_allocator {
private:
    const std::size_t max_size, capacidad_handlers;
    std::size_t bytes_ocupados = 0, // Dice exactamente cuantos bytes están ocupados.
        cantidad_handlers = 0,
        cantidad_bloques = 0,
        memoria_disponible; // Dice cuantos bytes hay disponibles después del último elemento del buffer.
    uint8_t* buffer;
    Block_pointer* bloques_reservados;
    Memory_handler* handlers, handler_invalido;

    void move_block(std::size_t pos_dest, std::size_t pos_origen) {
        for (std::size_t i = pos_dest; i < pos_origen; ++i)
            std::swap(bloques_reservados[i], bloques_reservados[pos_origen]);
    }

    void move_memory_in_block(uint8_t* dest, Block_pointer& bloque_a_mover) {
        if (dest == bloque_a_mover.get_block_start()) return;

        void* block_start, * data_start;
        std::size_t data_size, espacio_disponible = max_size, espacio_final, diff;
        data_size = bloque_a_mover.get_data_size();
        block_start = static_cast<void*>(dest);
        bloque_a_mover.set_block_start(block_start);
        espacio_final = espacio_disponible;
        std::align(
            bloque_a_mover.get_alineamiento(),
            bloque_a_mover.get_data_type_size(),
            block_start,
            espacio_final
        );
        data_start = bloque_a_mover.get_data();
        bloque_a_mover.set_data(block_start);
        diff = espacio_disponible - espacio_final;
        bloque_a_mover.set_block_size(data_size + diff);
        bloque_a_mover.set_data(block_start);
        std::memmove(block_start, data_start, data_size);
    }
    void desfragmentar() {
        if (cantidad_handlers == 0) return;
        uint8_t* ancla = buffer, * dir_bloque;
        std::size_t block_size, i, j, espacio_inter_bloque;
        std::sort(bloques_reservados, bloques_reservados + cantidad_bloques, _comparar_bloques);
        cantidad_bloques = cantidad_handlers;

        i = 0;
        while (i < cantidad_bloques) {
            bloques_reservados[i].get_parent()->hacer_valido(bloques_reservados + i);

            dir_bloque = static_cast<uint8_t*>(bloques_reservados[i].get_block_start());
            if (ancla != dir_bloque) {
                if (bloques_reservados[i].esta_congelado()) {
                    espacio_inter_bloque = dir_bloque - ancla;
                    for (j = i + 1; j < cantidad_bloques && espacio_inter_bloque>0; ++j) {
                        if (!bloques_reservados[j].esta_congelado() && bloques_reservados[j].get_block_size() <= espacio_inter_bloque) {
                            move_memory_in_block(ancla, bloques_reservados[j]);
                            block_size = bloques_reservados[j].get_block_size();
                            espacio_inter_bloque -= block_size;
                            ancla += block_size;
                            move_block(i, j);
                            ++i;
                            ++j;
                        }
                    }
                    ancla = static_cast<uint8_t*>(bloques_reservados[i].get_block_start());
                }
                else move_memory_in_block(ancla, bloques_reservados[i]);
            }
            block_size = bloques_reservados[i].get_block_size();
            ancla += block_size;
            ++i;
        }
        ptrdiff_t dist = static_cast<uint8_t*>(bloques_reservados[cantidad_bloques - 1].get_data()) - buffer;
        memoria_disponible = max_size - dist - bloques_reservados[cantidad_bloques - 1].get_block_size();
    }

    static bool _comparar_bloques(const Block_pointer& bloque1, const Block_pointer& bloque2) {
        return bloque2.get_parent() == nullptr ||
            (bloque1.get_parent() != nullptr && (uintptr_t)bloque1.get_block_start() <= (uintptr_t)bloque2.get_block_start());
    }
public:
    Memory_allocator(std::size_t _max_size)
        : max_size(_max_size), capacidad_handlers(_max_size / 100), memoria_disponible(_max_size) {
        Serial.println("Flag Memory_allocator 1");
        buffer = new uint8_t[max_size];
        Serial.println("Flag Memory_allocator 1");
        bloques_reservados = new Block_pointer[capacidad_handlers];
        Serial.println("Flag Memory_allocator 1");
        handlers = new Memory_handler[capacidad_handlers];
        Serial.println("Flag Memory_allocator 1");
    }
    ~Memory_allocator() {
        delete[] buffer;
        delete[] bloques_reservados;
        delete[] handlers;
    }

    template <typename T, typename... Args>
    Memory_handler& acquire(std::size_t cantidad_bytes, Args&&... args) {
        if (cantidad_handlers >= capacidad_handlers) return handler_invalido;
        if (memoria_disponible < cantidad_bytes) {
            if (bytes_ocupados + cantidad_bytes <= max_size) {
                desfragmentar();
                if (memoria_disponible < cantidad_bytes)
                    return handler_invalido;
            }
            else return handler_invalido;
        }

        // acá quedan handlers y hay memoria suficiente al final del buffer
        T* memoria_a_asignar;
        void* ptr;
        std::size_t espacio_final = memoria_disponible;
        if (cantidad_bloques == 0) {
            // Serial.println("cantidad_bloques == 0");
            memoria_a_asignar = reinterpret_cast<T*>(buffer);
        }
        else {
            memoria_a_asignar = reinterpret_cast<T*>(
                static_cast<uint8_t*>(bloques_reservados[cantidad_bloques - 1].get_block_start()) +
                bloques_reservados[cantidad_bloques - 1].get_block_size()
                );
        }
        ptr = static_cast<void*>(memoria_a_asignar);
        std::align(alignof(T), sizeof(T), ptr, espacio_final);
        Memory_handler* handler_a_usar = nullptr;
        for (std::size_t i = 0; i < capacidad_handlers; ++i) {
            if (!handlers[i].es_valido()) {
                handler_a_usar = handlers + i;
                break;
            }
        }
        cantidad_handlers++;
        Block_pointer* bloque_a_usar = bloques_reservados + cantidad_bloques++;
        handler_a_usar->hacer_valido(bloque_a_usar);
        bloque_a_usar->set_parent(handler_a_usar);
        bloque_a_usar->set_alineamiento(alignof(T));
        bloque_a_usar->set_data_type_size(sizeof(T));
        bloque_a_usar->set_data(ptr);
        bloque_a_usar->set_data_size(cantidad_bytes);
        bloque_a_usar->set_block_start(static_cast<void*>(memoria_a_asignar));
        bloque_a_usar->set_block_size(cantidad_bytes + memoria_disponible - espacio_final);
        memoria_disponible -= bloques_reservados[cantidad_bloques - 1].get_block_size();
        bytes_ocupados += bloques_reservados[cantidad_bloques - 1].get_block_size();
        new (ptr) T(std::forward<Args>(args)...);
        return *handler_a_usar;
    }
    template <typename T>
    void release(Memory_handler& handler, std::size_t cantidad_elementos = 1) {
        Memory_handler* pos = &handler;
        if (pos < handlers || handlers + capacidad_handlers <= pos) return; // este handler no es de acá
        T* data_handler = handler.get_elem<T>();
        for (std::size_t i = 0; i < cantidad_elementos; ++i)
            data_handler[i].~T();
        bytes_ocupados -= handler.get_child().get_block_size();
        handler.get_child().anular();
        handler.hacer_invalido();
        cantidad_handlers--;
    }
    std::size_t available_memory() const {
        return max_size - bytes_ocupados;
    }

    std::size_t get_cantidad_handlers() const {
        return cantidad_handlers;
    }

    std::size_t get_cantidad_bloques() const {
        return cantidad_bloques;
    }

    void force_defragmentation() {
        desfragmentar();
    }


};

#endif