#ifndef MAPA_HPP
#define MAPA_HPP

#include <global/memoria.hpp>
#include <mensaje/mensaje_vector/mensaje_vector.hpp>
#include <allocator/allocator.hpp>
#include <allocator/test_allocator.hpp>
#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>
#include <queue>

using par_id_costo = std::pair<uint16_t, float>;
using tipo_tabla = std::unordered_map<
    uint16_t,
    float,
    std::hash<uint16_t>,
    std::equal_to<uint16_t>,
    CAllocator<par_id_costo>
>;
using par_id_vecinos = std::pair<uint16_t, tipo_tabla>;
using tipo_grafo = std::unordered_map<
    uint16_t,
    tipo_tabla,
    std::hash<uint16_t>,
    std::equal_to<uint16_t>,
    CAllocator<par_id_vecinos>
>;
using vec_pares = std::vector<par_costo_id, CAllocator<par_costo_id>>;
using custom_pq = std::priority_queue<par_costo_id, vec_pares, std::greater<par_costo_id>>;

class Custom_priority_queue : public custom_pq {
public:
    Custom_priority_queue() = default;
    ~Custom_priority_queue() = default;

    void reserve(std::size_t reserve_size) {
        this->c.reserve(reserve_size);
    }
};

class Mapa {
private:
    Memory_allocator& memoria = Memoria::memoria;


    uint16_t id;
    //las llaves son nodos fuente, los valores otros map cuyas llaves son nodos destino cuyo valor es el peso del arco.
    Memory_handler& handler_grafo;
    Memory_handler& handler_tabla_distancias_desde_inicio;
    Memory_handler& handler_custom_priority_queue;

    void print_tabla() const;

    void print_grafo() const;

    // std::unordered_map<uint16_t, std::unordered_map<uint16_t, float>> grafo;
    // std::unordered_map<uint16_t, float> tabla_distancias_desde_inicio;

public:
    Mapa(uint16_t _id);
    ~Mapa();
    void dijkstra();
    void actualizar_probabilidades(uint16_t origen, Memory_handler& handler_pares, std::size_t cantidad_pares);
    void actualizar_propias_probabilidades(uint16_t nodo_visto);
    void actualizar_propias_probabilidades(uint16_t* nodos_vistos, uint16_t cant_nodos);

    void obtener_vector_probabilidad(Memory_handler& pares_handler) const;
    float costo(uint16_t destino) const;
    unsigned get_size_vector_probabilidad() const;

    void print() const;
};

#endif