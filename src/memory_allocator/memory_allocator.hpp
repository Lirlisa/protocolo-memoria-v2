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
#include <Arduino.h>


class Memory_allocator {
private:
    const std::size_t max_size, capacidad_handlers;
    std::size_t bytes_ocupados = 0, // Dice exactamente cuantos bytes están ocupados.
        cantidad_handlers = 0,
        cantidad_bloques = 0,
        cantidad_vacios; // Dice cuantos bloques vacíos disponibles hay.
    uint8_t* buffer;
    Block_pointer* bloques_reservados;
    Memory_handler* handlers, handler_invalido;

    void move_block(std::size_t pos_dest, std::size_t pos_origen) {
        std::size_t i;
        for (i = pos_dest; i < pos_origen; ++i) {
            std::swap(bloques_reservados[i], bloques_reservados[pos_origen]);
            bloques_reservados[i].get_parent()->set_child(bloques_reservados + i);
        }
        bloques_reservados[pos_origen].get_parent()->set_child(bloques_reservados + pos_origen);
    }

    void move_memory_in_block(void* dest, Block_pointer& bloque_a_mover) {
        if (dest == bloque_a_mover.get_block_start()) return;

        void* block_start, * data_start;
        std::size_t data_size, espacio_disponible = max_size, espacio_final, diff;
        data_size = bloque_a_mover.get_data_size();
        block_start = dest;
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
        std::memmove(block_start, data_start, data_size);
    }

    /*
    @brief Entrega la posición de un handler que no esté en uso
    */
    std::size_t obtener_handler() const {
        std::size_t i = 0;
        while (i < capacidad_handlers) {
            if (!handlers[i].es_valido()) break;
            ++i;
        }
        return i;
    }

    void desfragmentar() {
        if (cantidad_handlers == 0) return;
        uint8_t* ancla = buffer, * dir_bloque, * ancla_vacios;
        std::size_t block_size, i, j, espacio_inter_bloque, mem, pos_handler_disponible;
        ptrdiff_t dist;
        Serial.println(cantidad_bloques);
        std::sort(bloques_reservados, bloques_reservados + cantidad_bloques, _comparar_bloques);
        for (i = cantidad_handlers - cantidad_vacios; i < cantidad_bloques; ++i) {
            if (bloques_reservados[i].get_parent() == nullptr) continue;
            bloques_reservados[i].get_parent()->hacer_invalido();
            bloques_reservados[i].anular();
        }
        cantidad_handlers -= cantidad_vacios;
        cantidad_bloques = cantidad_handlers;

        cantidad_vacios = 0;
        i = 0;
        while (i < cantidad_bloques) {
            bloques_reservados[i].get_parent()->hacer_valido(bloques_reservados + i);

            dir_bloque = static_cast<uint8_t*>(bloques_reservados[i].get_block_start());
            if (ancla != dir_bloque) {
                if (bloques_reservados[i].esta_congelado()) {
                    espacio_inter_bloque = dir_bloque - ancla;
                    for (j = i + 1; j < cantidad_bloques && espacio_inter_bloque > 0; ++j) {
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
                    if (espacio_inter_bloque > 0 && cantidad_handlers < capacidad_handlers) {
                        pos_handler_disponible = obtener_handler();
                        handlers[pos_handler_disponible].hacer_valido(bloques_reservados + cantidad_bloques);
                        handlers[pos_handler_disponible].set_vacio(true);
                        handlers[pos_handler_disponible].descongelar_bloque();
                        bloques_reservados[cantidad_bloques].set_parent(handlers + pos_handler_disponible);
                        if (i == 0)
                            ancla_vacios = buffer;
                        else
                            ancla_vacios = static_cast<uint8_t*>(bloques_reservados[i - 1].get_block_start()) +
                            bloques_reservados[i - 1].get_block_size();
                        bloques_reservados[cantidad_bloques].set_block_start(ancla_vacios);
                        bloques_reservados[cantidad_bloques].set_data(ancla_vacios);
                        bloques_reservados[cantidad_bloques].set_block_size(espacio_inter_bloque);
                        bloques_reservados[cantidad_bloques].set_data_size(espacio_inter_bloque);
                        bloques_reservados[cantidad_bloques].set_alineamiento(alignof(uint8_t));
                        bloques_reservados[cantidad_bloques].set_data_type_size(sizeof(uint8_t));

                        move_block(i, cantidad_bloques);
                        ++cantidad_bloques;
                        ++cantidad_handlers;
                        ++i;
                        ++cantidad_vacios;
                    }
                    ancla = static_cast<uint8_t*>(bloques_reservados[i].get_block_start());
                }
                else {
                    move_memory_in_block(ancla, bloques_reservados[i]);
                }
            }
            block_size = bloques_reservados[i].get_block_size();
            ancla += block_size;
            ++i;
        }
        if (cantidad_handlers >= capacidad_handlers) {
            return;
        }
        if (cantidad_bloques == 0) {
            dist = 0;
            mem = max_size;
            ancla = buffer;
        }
        else {
            dist = static_cast<uint8_t*>(bloques_reservados[cantidad_bloques - 1].get_block_start()) - buffer;
            mem = max_size - dist - bloques_reservados[cantidad_bloques - 1].get_block_size();
            ancla = (
                static_cast<uint8_t*>(bloques_reservados[cantidad_bloques - 1].get_block_start()) +
                bloques_reservados[cantidad_bloques - 1].get_block_size()
                );
        }
        i = 0;
        while (i < capacidad_handlers) {
            if (!handlers[i].es_valido()) break;
            ++i;
        }
        if (i == capacidad_handlers) {
            return;
        }

        handlers[i].hacer_valido(bloques_reservados + cantidad_bloques);
        handlers[i].set_vacio(true);
        bloques_reservados[cantidad_bloques].set_parent(handlers + i);
        bloques_reservados[cantidad_bloques].set_block_start(ancla);
        bloques_reservados[cantidad_bloques].set_data(ancla);
        bloques_reservados[cantidad_bloques].set_block_size(mem);
        bloques_reservados[cantidad_bloques].set_data_size(mem);
        bloques_reservados[cantidad_bloques].set_alineamiento(alignof(uint8_t));
        bloques_reservados[cantidad_bloques].set_data_type_size(sizeof(uint8_t));

        ++cantidad_bloques;
        ++cantidad_handlers;
        ++cantidad_vacios;
    }

    static bool _comparar_bloques(const Block_pointer& bloque1, const Block_pointer& bloque2) {
        const bool valido_1 = bloque1.get_parent() != nullptr && !bloque1.get_parent()->esta_vacio() && bloque1.get_parent()->es_valido();
        const bool valido_2 = bloque2.get_parent() != nullptr && !bloque2.get_parent()->esta_vacio() && bloque2.get_parent()->es_valido();
        if (valido_1 && valido_2) return (uintptr_t)bloque1.get_block_start() < (uintptr_t)bloque2.get_block_start();
        return valido_1;
    }

    void vaciar_bloque(Memory_handler& handler, std::size_t cantidad_elementos) {
        bytes_ocupados -= handler.get_child().get_block_size();

        Block_pointer& bloque = handler.get_child();
        handler.set_vacio(true);
        handler.hacer_valido(&bloque);
        handler.descongelar_bloque();

        bloque.set_data(bloque.get_block_start());
        bloque.set_alineamiento(alignof(uint8_t));
        bloque.set_data_type_size(sizeof(uint8_t));
        bloque.set_data_size(bloque.get_block_size());

        ++cantidad_vacios;
    }

    /*
    @brief Une 2 bloques que estén vacíos y desactiva el bloque a unir
    */
    bool unir_bloques_vacios(Block_pointer& bloque_base, Block_pointer& bloque_a_unir) {
        if (bloque_base.esta_congelado() || bloque_a_unir.esta_congelado() ||
            bloque_base.get_parent() == nullptr || bloque_a_unir.get_parent() == nullptr ||
            !bloque_base.get_parent()->es_valido() || !bloque_base.get_parent()->esta_vacio() ||
            !bloque_a_unir.get_parent()->es_valido() || !bloque_a_unir.get_parent()->esta_vacio()) {
            return false;
        }
        if (
            static_cast<uint8_t*>(bloque_base.get_block_start()) +
            bloque_base.get_block_size() !=
            static_cast<uint8_t*>(bloque_a_unir.get_block_start()) &&
            static_cast<uint8_t*>(bloque_a_unir.get_block_start()) +
            bloque_a_unir.get_block_size() !=
            static_cast<uint8_t*>(bloque_base.get_block_start())
            ) {
            return false;
        }
        void* block_start = bloque_base.get_block_start() < bloque_a_unir.get_block_start() ?
            bloque_base.get_block_start() : bloque_a_unir.get_block_start();
        std::size_t block_size = bloque_base.get_block_size() + bloque_a_unir.get_block_size();

        bloque_base.set_block_start(block_start);
        bloque_base.set_data(block_start);
        bloque_base.set_block_size(block_size);
        bloque_base.set_data_size(block_size);
        bloque_base.set_alineamiento(alignof(uint8_t));
        bloque_base.set_data_type_size(sizeof(uint8_t));

        bloque_a_unir.get_parent()->hacer_invalido();
        bloque_a_unir.anular();
        --cantidad_handlers;
        --cantidad_vacios;
        return true;
    }

    template <typename T>
    Memory_handler& acquire_helper(std::size_t cantidad_bytes) {
        std::size_t intentos = 0;
        if (cantidad_vacios < 1) {
            desfragmentar();
            if (cantidad_vacios < 1) {
                return handler_invalido;
            }
            ++intentos;
        }
        if (bytes_ocupados + cantidad_bytes > max_size) {
            return handler_invalido;
        }
        // acá quedan handlers y hay memoria suficiente al final del buffer
        void* ptr;
        std::size_t espacio_final;
        std::size_t i, j;
        Memory_handler* handler_a_usar;
        Block_pointer* bloque_a_usar;
        while (true) {
            for (i = cantidad_bloques; i > 0; --i) {
                Block_pointer& bloque = bloques_reservados[i - 1];
                if (bloque.get_parent() == nullptr || bloque.get_parent() < handlers || bloque.get_parent() >= handlers + capacidad_handlers) continue;
                Memory_handler& handler = *bloque.get_parent();
                if (!handler.es_valido() || !handler.esta_vacio() || bloque.get_block_size() < cantidad_bytes) {
                    continue;
                }
                ptr = bloque.get_block_start();
                espacio_final = bloque.get_block_size();
                if (std::align(alignof(T), sizeof(T), ptr, espacio_final) == nullptr) continue;
                if (espacio_final < cantidad_bytes || ptr == nullptr) continue;

                handler_a_usar = &handler;
                bloque_a_usar = &bloque;

                handler_a_usar->set_vacio(false);
                bloque_a_usar->set_alineamiento(alignof(T));
                bloque_a_usar->set_data_type_size(sizeof(T));
                bloque_a_usar->set_data(ptr);
                bloque_a_usar->set_data_size(cantidad_bytes);
                bloque_a_usar->set_block_size(cantidad_bytes + bloque_a_usar->get_block_size() - espacio_final);
                bytes_ocupados += bloque_a_usar->get_block_size();

                espacio_final -= bloque_a_usar->get_data_size();
                if (espacio_final == 0 || cantidad_bloques >= capacidad_handlers || cantidad_handlers >= capacidad_handlers) --cantidad_vacios;
                else {
                    j = obtener_handler();
                    Memory_handler& handler_vacio = handlers[j];
                    Block_pointer& bloque_vacio = bloques_reservados[cantidad_bloques];
                    handler_vacio.hacer_valido(&bloque_vacio);
                    handler_vacio.set_vacio(true);
                    bloque_vacio.set_parent(&handler_vacio);
                    bloque_vacio.set_block_start(static_cast<uint8_t*>(bloque_a_usar->get_block_start()) + bloque_a_usar->get_block_size());
                    bloque_vacio.set_data(bloque_vacio.get_block_start());
                    bloque_vacio.set_block_size(espacio_final);
                    bloque_vacio.set_data_size(espacio_final);
                    bloque_vacio.set_alineamiento(alignof(uint8_t));
                    bloque_vacio.set_data_type_size(sizeof(uint8_t));

                    ++cantidad_bloques;
                    ++cantidad_handlers;
                }
                return *handler_a_usar;
            }
            if (intentos++ > 0) break;
            desfragmentar();
        }
        return handler_invalido;
    }
public:
    Memory_allocator(std::size_t _max_size)
        : max_size(_max_size), capacidad_handlers(_max_size / 10) {
        buffer = new uint8_t[max_size];
        bloques_reservados = new Block_pointer[capacidad_handlers];
        handlers = new Memory_handler[capacidad_handlers];

        // bloque de memoria vacío, disponible para usar, que abarca todo el buffer
        cantidad_bloques = 1;
        cantidad_handlers = 1;
        cantidad_vacios = 1;
        handlers[0].hacer_valido(bloques_reservados);
        handlers[0].set_vacio(true);
        bloques_reservados[0].set_parent(handlers);
        bloques_reservados[0].set_block_start(buffer);
        bloques_reservados[0].set_data(buffer);
        bloques_reservados[0].set_block_size(max_size);
        bloques_reservados[0].set_data_size(max_size);
        bloques_reservados[0].set_alineamiento(alignof(uint8_t));
        bloques_reservados[0].set_data_type_size(sizeof(uint8_t));
    }

    Memory_allocator(Memory_allocator& instance) = delete;
    Memory_allocator& operator=(const Memory_allocator& instance) = delete;
    ~Memory_allocator() {
        delete[] buffer;
        delete[] bloques_reservados;
        delete[] handlers;
    }

    template <typename T, typename... Args>
    Memory_handler& acquire(std::size_t cantidad_bytes, Args&&... args) {
        Memory_handler& handler = acquire_helper<T>(cantidad_bytes);
        if (!handler.es_valido()) {
            Serial.println("Entregando handler invalido");
            return handler_invalido;
        }
        handler.congelar_bloque();
        new (handler.get_elem<T>()) T(std::forward<Args>(args)...);
        handler.descongelar_bloque();
        return handler;
    }

    template <typename T>
    Memory_handler& acquire_simple(std::size_t cantidad_bytes) {
        Memory_handler& handler = acquire_helper<T>(cantidad_bytes);
        if (!handler.es_valido()) Serial.println("Entregando handler invalido");
        return handler;
    }

    template <typename T>
    void release(Memory_handler& handler, std::size_t cantidad_elementos = 1) {
        Memory_handler* pos = &handler;
        if (pos == nullptr || pos == &handler_invalido) return;
        if (!handler.es_valido()) return;
        Block_pointer* block_pos = &handler.get_child();
        if (block_pos == nullptr) return;
        if (pos < handlers || handlers + capacidad_handlers <= pos) return; // este handler no es de acá
        if (block_pos < bloques_reservados || bloques_reservados + capacidad_handlers <= block_pos) return; // este bloque no es de acá
        T* data_handler = handler.get_elem<T>();
        for (std::size_t i = 0; i < cantidad_elementos; ++i)
            data_handler[i].~T();
        vaciar_bloque(handler, cantidad_elementos);
        Block_pointer* block;
        //iterar izquierda
        for (block = block_pos - 1; block >= bloques_reservados; --block) {
            if (!unir_bloques_vacios(*block_pos, *block)) break;
        }
        //iterar derecha
        for (block = block_pos + 1; block < bloques_reservados + cantidad_bloques; ++block) {
            if (!unir_bloques_vacios(*block_pos, *block)) break;
        }
    }

    template <typename T>
    void release(T* ptr, std::size_t cantidad_elementos = 1) {
        Serial.println("Liberando desde ptr");
        std::size_t i;
        for (i = 0; i < cantidad_bloques; ++i) {
            if (static_cast<T*>(bloques_reservados[i].get_data()) == ptr && bloques_reservados[i].get_parent() != nullptr) break;
        }
        if (i == cantidad_bloques) {
            Serial.println("handler no encontrado");
            return;
        }
        Memory_handler& handler = *bloques_reservados[i].get_parent();
        Memory_handler* pos = &handler;
        if (pos == nullptr) return;
        if (!handler.es_valido()) return;
        Block_pointer* block_pos = &handler.get_child();
        if (block_pos == nullptr) return;
        if (pos < handlers || handlers + capacidad_handlers <= pos) return; // este handler no es de acá
        if (block_pos < bloques_reservados || bloques_reservados + capacidad_handlers <= block_pos) return; // este bloque no es de acá
        vaciar_bloque(handler, cantidad_elementos);
        Block_pointer* block;
        //iterar izquierda
        for (block = block_pos - 1; block >= bloques_reservados; --block) {
            if (!unir_bloques_vacios(*block_pos, *block)) break;
        }
        //iterar derecha
        for (block = block_pos + 1; block < bloques_reservados + cantidad_bloques; ++block) {
            if (!unir_bloques_vacios(*block_pos, *block)) break;
        }
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

    std::size_t get_cantidad_vacios() const {
        return cantidad_vacios;
    }

    void force_defragmentation() {
        desfragmentar();
    }

    void print() const {
        Serial.println("----- Memory allocator -----");
        Serial.print("Max size: ");
        Serial.println(max_size);
        Serial.print("Memoria disponible: ");
        Serial.println(available_memory());
        Serial.print("Bytes ocupados: ");
        Serial.println(bytes_ocupados);
        Serial.print("Cantidad handlers: ");
        Serial.println(cantidad_handlers);
        Serial.print("Cantidad bloques: ");
        Serial.println(cantidad_bloques);
        std::size_t contador = 0;
        for (std::size_t i = 0; i < cantidad_bloques; ++i) {
            if (bloques_reservados[i].get_parent() == nullptr || !bloques_reservados[i].get_parent()->es_valido()) continue;
            Serial.print("Bloque ");
            Serial.print(contador);
            Serial.print(" vacío? ");
            Serial.println(bloques_reservados[i].get_parent()->esta_vacio() ? "Sí" : "No");
            Serial.print("Dirección handler ");
            Serial.print(contador);
            Serial.print(": ");
            Serial.println((uintptr_t)bloques_reservados[i].get_parent(), HEX);
            Serial.print("Comienzo bloque ");
            Serial.print(contador);
            Serial.print(": ");
            Serial.println((uintptr_t)bloques_reservados[i].get_block_start(), HEX);
            Serial.print("Comienzo data ");
            Serial.print(contador);
            Serial.print(": ");
            Serial.println((uintptr_t)bloques_reservados[i].get_data(), HEX);
            Serial.print("Fin bloque ");
            Serial.print(contador);
            Serial.print(": ");
            Serial.println(
                (uintptr_t)(
                    static_cast<uint8_t*>(bloques_reservados[i].get_block_start()) +
                    bloques_reservados[i].get_block_size() - 1
                    ),
                HEX
            );
            Serial.print("Tamaño bloque ");
            Serial.print(contador++);
            Serial.print(": ");
            Serial.println(bloques_reservados[i].get_block_size());
        }
        Serial.println("----- Memory allocator -----");
    }

    Memory_handler& get_handler_from_ptr(void* ptr) {
        for (std::size_t i = 0; i < cantidad_bloques; ++i) {
            if (
                bloques_reservados[i].get_parent() != nullptr &&
                bloques_reservados[i].get_data() <= ptr &&
                ptr <= static_cast<void*>(static_cast<uint8_t*>(bloques_reservados[i].get_data()) + bloques_reservados[i].get_data_size() - 1)
                ) {
                return *bloques_reservados[i].get_parent();
            }
        }
        return handler_invalido;
    }

    void print_vacios() const {
        std::size_t contador = 0;
        for (std::size_t i = 0; i < cantidad_bloques; ++i) {
            if (
                bloques_reservados[i].get_parent() == nullptr ||
                !bloques_reservados[i].get_parent()->es_valido() ||
                !bloques_reservados[i].get_parent()->esta_vacio()
                ) continue;
            Serial.print("Bloque ");
            Serial.print(contador);
            Serial.print(" vacío? ");
            Serial.println(bloques_reservados[i].get_parent()->esta_vacio() ? "Sí" : "No");
            Serial.print("Dirección handler ");
            Serial.print(contador);
            Serial.print(": ");
            Serial.println((uintptr_t)bloques_reservados[i].get_parent(), HEX);
            Serial.print("Comienzo bloque ");
            Serial.print(contador);
            Serial.print(": ");
            Serial.println((uintptr_t)bloques_reservados[i].get_block_start(), HEX);
            Serial.print("Comienzo data ");
            Serial.print(contador);
            Serial.print(": ");
            Serial.println((uintptr_t)bloques_reservados[i].get_data(), HEX);
            Serial.print("Fin bloque ");
            Serial.print(contador);
            Serial.print(": ");
            Serial.println(
                (uintptr_t)(
                    static_cast<uint8_t*>(bloques_reservados[i].get_block_start()) +
                    bloques_reservados[i].get_block_size() - 1
                    ),
                HEX
            );
            Serial.print("Tamaño bloque ");
            Serial.print(contador++);
            Serial.print(": ");
            Serial.println(bloques_reservados[i].get_block_size());
        }
    }

};

#endif