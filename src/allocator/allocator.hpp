#ifndef ALLOCATOR_HPP
#define ALLOCATOR_HPP

#include <memory_allocator/memory_allocator.hpp>
#include <memory_allocator/memory_handler/memory_handler.hpp>
#include <global/memoria.hpp>
#include <Arduino.h>

template <class T>
class CAllocator {
private:
    Memory_allocator& memoria = Memoria::memoria;
public:
    using value_type = T;

    CAllocator() = default;

    template <class U>
    CAllocator(const CAllocator<U>&) { }

    ~CAllocator() = default;

    T* allocate(size_t n, const void* hint = nullptr) {
        Serial.println("Flag allocate 1");
        Memory_handler& handler_memoria = memoria.acquire_simple<T>(n * sizeof(T));
        Serial.print("Handler handler_memoria valido? ");
        Serial.println(handler_memoria.es_valido() ? "Sí" : "No");
        Serial.print("Handler alojado: ");
        Serial.println((uintptr_t)&handler_memoria, HEX);
        Serial.print("Memoria alojada: ");
        Serial.println((uintptr_t)handler_memoria.get_elem<T>(), HEX);
        Serial.print("n: ");
        Serial.println(n);
        Serial.print("Tamaño data type: ");
        Serial.println(sizeof(T));
        Serial.print("Memoria disponible: ");
        Serial.println(memoria.available_memory());
        Serial.print("Bloques usados: ");
        Serial.println(memoria.get_cantidad_bloques());
        Serial.print("Handlers usados: ");
        Serial.println(memoria.get_cantidad_handlers());
        Serial.print("Bloques vacíos: ");
        Serial.println(memoria.get_cantidad_vacios());
        Serial.println("Flag allocate 2");
        handler_memoria.congelar_bloque();
        Serial.println("Flag allocate 3");
        return handler_memoria.get_elem<T>();
    }

    void deallocate(T* p, size_t n) {
        Serial.println("Flag deallocate 1");
        Serial.print("Memoria a desalojar: ");
        Serial.println((uintptr_t)p, HEX);
        memoria.release<T>(p, n);
        Serial.println("Flag deallocate 2");
    }

    template<class U, class... Args>
    void construct(U* p, Args&& ...args) {
        Serial.println("Flag construct 1");
        new (p) T(std::forward<Args>(args)...);
        Serial.println("Flag construct 2");
    }

    template<class U>
    void destroy(U* p) {
        Serial.println("Flag destroy 1");
        p->~U();
        Serial.println("Flag destroy 2");
    }

    // Rebind helper for allocator_traits
    template<class U>
    struct rebind {
        using other = CAllocator<U>;
    };
};

#endif