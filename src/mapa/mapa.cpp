#include <mapa/mapa.hpp>
#include <mensaje/mensaje_vector/mensaje_vector.hpp>
#include <allocator/allocator.hpp>
#include <cstdint>
#include <unordered_map>
#include <limits>
#include <cstring>
#include <unordered_set>
#include <vector>
#include <queue>
#include <Arduino.h>

#define CANT_RESERVE 15

Mapa::Mapa(uint16_t _id) :
    handler_grafo(memoria.acquire<tipo_grafo>(sizeof(tipo_grafo))),
    handler_tabla_distancias_desde_inicio(memoria.acquire<tipo_tabla>(sizeof(tipo_tabla))),
    handler_custom_priority_queue(memoria.acquire<Custom_priority_queue>(sizeof(Custom_priority_queue))) {
    Serial.println("Flag Mapa constructor 1");
    Serial.print("Memoria disponible: ");
    Serial.println(memoria.available_memory());
    Serial.print("Bloques usados: ");
    Serial.println(memoria.get_cantidad_bloques());
    Serial.print("Handlers usados: ");
    Serial.println(memoria.get_cantidad_handlers());
    id = _id;
    handler_grafo.get_elem<tipo_grafo>()->reserve(CANT_RESERVE);
    Serial.println("Flag Mapa constructor 2");
    Serial.println((uintptr_t)&memoria, HEX);
    Serial.print("Memoria disponible: ");
    Serial.println(memoria.available_memory());
    Serial.print("Bloques usados: ");
    Serial.println(memoria.get_cantidad_bloques());
    Serial.print("Handlers usados: ");
    Serial.println(memoria.get_cantidad_handlers());
    handler_grafo.get_elem<tipo_grafo>()->insert({ id, tipo_tabla() });
    Serial.println("Flag Mapa constructor 3");
    Serial.println((uintptr_t)&memoria, HEX);
    Serial.print("Memoria disponible: ");
    Serial.println(memoria.available_memory());
    Serial.print("Bloques usados: ");
    Serial.println(memoria.get_cantidad_bloques());
    Serial.print("Handlers usados: ");
    Serial.println(memoria.get_cantidad_handlers());
    handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->reserve(CANT_RESERVE);
    Serial.println("Flag Mapa constructor 4");
    Serial.print("Memoria disponible: ");
    Serial.println(memoria.available_memory());
    Serial.print("Bloques usados: ");
    Serial.println(memoria.get_cantidad_bloques());
    Serial.print("Handlers usados: ");
    Serial.println(memoria.get_cantidad_handlers());

    handler_custom_priority_queue.get_elem<Custom_priority_queue>()->reserve(CANT_RESERVE);
}

Mapa::~Mapa() {
    memoria.release<tipo_grafo>(handler_grafo);
    memoria.release<tipo_tabla>(handler_tabla_distancias_desde_inicio);
    memoria.release<Custom_priority_queue>(handler_custom_priority_queue);
}

void Mapa::dijkstra() {
    Serial.println("Flag dijkstra 1");
    // Serial.println("----- Data tabla -----");
    // print_tabla();
    // Serial.println("----- Fin data tabla -----");
    // Serial.println("----- Data grafo -----");
    // print_grafo();
    // Serial.println("----- Fin data grafo -----");
    // print();
    // handler_custom_priority_queue.get_elem<Custom_priority_queue>();
    Memory_handler& handler_pq = handler_custom_priority_queue;
    Serial.println("Flag dijkstra 2");
    handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->clear();
    Serial.println("Flag dijkstra 3");
    Serial.println(handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->bucket_count());
    Serial.println(handler_grafo.get_elem<tipo_grafo>()->size());
    if (handler_grafo.get_elem<tipo_grafo>()->at(id).size() > handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->bucket_count())
        handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->reserve(
            handler_grafo.get_elem<tipo_grafo>()->at(id).size()
        );
    Serial.println(handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->bucket_count());
    Serial.println("Flag dijkstra 4");
    handler_pq.get_elem<Custom_priority_queue>()->push({ 0, id });
    Serial.println("Flag dijkstra 5");
    handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->insert({ id, 0 });
    Serial.println("Flag dijkstra 6");
    std::unordered_map<uint16_t, float>::iterator i;

    while (!handler_pq.get_elem<Custom_priority_queue>()->empty()) {
        Serial.println("Flag dijkstra 7");
        uint16_t u = handler_pq.get_elem<Custom_priority_queue>()->top().second;
        Serial.println("Flag dijkstra 8");
        handler_pq.get_elem<Custom_priority_queue>()->pop();
        Serial.println("Flag dijkstra 9");
        if (handler_grafo.get_elem<tipo_grafo>()->find(u) == handler_grafo.get_elem<tipo_grafo>()->end()) continue;
        for (i = handler_grafo.get_elem<tipo_grafo>()->at(u).begin(); i != handler_grafo.get_elem<tipo_grafo>()->at(u).end(); ++i) {
            Serial.println("Flag dijkstra 10");
            uint16_t v = i->first;
            float weight = i->second;
            Serial.println("Flag dijkstra 11");
            float dist_v = handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->find(v) == handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->end() ?
                std::numeric_limits<float>::infinity() :
                handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->at(v);
            Serial.println("Flag dijkstra 12");
            Serial.print("u = ");
            Serial.println(u);
            float dist_u = handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->find(u) == handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->end() ?
                std::numeric_limits<float>::infinity() :
                handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->at(u);
            Serial.println(dist_u);
            Serial.println("Flag dijkstra 13");
            if (dist_v > dist_u + weight) {
                Serial.println("Flag dijkstra 14");
                Serial.println("Flag dijkstra 15");
                Serial.print("Cantidad elementos en tabla_distancias_desde_inicio: ");
                Serial.println(handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->bucket_count());
                handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->insert({ v, 0 });
                Serial.println("Flag dijkstra 15.1");

                Serial.println("Flag dijkstra 16");
                (*handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>())[v] = dist_u + weight;
                Serial.println((*handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>())[v]);
                Serial.println("Flag dijkstra 17");
                handler_pq.get_elem<Custom_priority_queue>()->push({ (*handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>())[v], v });
                Serial.println("Flag dijkstra 18");
            }
            Serial.println("Flag dijkstra 19");
        }
        Serial.println("Flag dijkstra 20");
    }
    Serial.println("Flag dijkstra 21");
}

void Mapa::actualizar_probabilidades(uint16_t origen, Memory_handler& handler_pares, std::size_t cantidad_pares) {
    if (handler_grafo.get_elem<tipo_grafo>()->find(origen) == handler_grafo.get_elem<tipo_grafo>()->end())
        handler_grafo.get_elem<tipo_grafo>()->insert({ origen, {} });

    handler_pares.congelar_bloque();
    par_costo_id* pares = handler_pares.get_elem<par_costo_id>();
    for (uint8_t i = 0; i < cantidad_pares; ++i)
        (*handler_grafo.get_elem<tipo_grafo>())[origen].insert({ pares[i].second, pares[i].first });

    handler_pares.descongelar_bloque();
    dijkstra();
}

void Mapa::actualizar_propias_probabilidades(uint16_t nodo_visto) {
    if (handler_grafo.get_elem<tipo_grafo>()->at(id).find(nodo_visto) == handler_grafo.get_elem<tipo_grafo>()->at(id).end()) {
        handler_grafo.get_elem<tipo_grafo>()->at(id).insert({ nodo_visto, 0 });
    }
    (*handler_grafo.get_elem<tipo_grafo>())[id][nodo_visto] += 1;

    float suma = 0;
    Serial.println(handler_grafo.get_elem<tipo_grafo>()->at(id).size());
    for (
        tipo_tabla::const_iterator v = handler_grafo.get_elem<tipo_grafo>()->at(id).begin();
        v != handler_grafo.get_elem<tipo_grafo>()->at(id).end();
        ++v
        ) {
        suma += v->second;
    }
    for (auto& v : handler_grafo.get_elem<tipo_grafo>()->at(id)) {
        (*handler_grafo.get_elem<tipo_grafo>())[id][v.first] /= suma;
    }
    dijkstra();
}

void Mapa::actualizar_propias_probabilidades(uint16_t* nodos_vistos, uint16_t cant_nodos) {
    for (uint16_t i = 0; i < cant_nodos; ++i) {
        if (handler_grafo.get_elem<tipo_grafo>()->at(id).find(nodos_vistos[i]) == handler_grafo.get_elem<tipo_grafo>()->at(id).end())
            handler_grafo.get_elem<tipo_grafo>()->at(id).insert({ nodos_vistos[i], 0 });

        (*handler_grafo.get_elem<tipo_grafo>())[id][nodos_vistos[i]] += 1;
    }

    float suma = 0;
    for (auto& v : handler_grafo.get_elem<tipo_grafo>()->at(id))
        suma += v.second;

    for (auto& v : handler_grafo.get_elem<tipo_grafo>()->at(id))
        (*handler_grafo.get_elem<tipo_grafo>())[id][v.first] /= suma;

    dijkstra();
}

/*
@brief Almacena en el arreglo entregado todos los pares de costo e id de los nodos vecinos para el nodo base.
Los primeros 2 bytes corresponden a la cantidad de pares dentro del arreglo
*/
void Mapa::obtener_vector_probabilidad(Memory_handler& pares_handler) const {
    uint16_t i = 0, cantidad_pares = handler_grafo.get_elem<tipo_grafo>()->at(id).size();
    std::memcpy(pares_handler.get_elem<uint8_t>(), &cantidad_pares, sizeof(uint16_t));
    for (
        tipo_tabla::const_iterator vecino = handler_grafo.get_elem<tipo_grafo>()->at(id).begin();
        vecino != handler_grafo.get_elem<tipo_grafo>()->at(id).end();
        ++vecino
        ) {
        std::memcpy(
            pares_handler.get_elem<uint8_t>() + sizeof(uint16_t) + i * (sizeof(float) + sizeof(uint16_t)),
            &(vecino->first),
            sizeof(uint16_t)
        );
        std::memcpy(
            pares_handler.get_elem<uint8_t>() + sizeof(uint16_t) + i * (sizeof(float) + sizeof(uint16_t)) + sizeof(uint16_t),
            &(vecino->second),
            sizeof(float)
        );
        ++i;
    }
}

float Mapa::costo(uint16_t destino) const {
    return handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->find(destino) == handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->end() ?
        std::numeric_limits<float>::infinity() :
        handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->at(destino);
}

unsigned Mapa::get_size_vector_probabilidad() const {
    return sizeof(uint16_t) + handler_grafo.get_elem<tipo_grafo>()->find(id)->second.size() * (sizeof(float) + sizeof(uint16_t));
}

void Mapa::print() const {
    Serial.println("----- Mapa -----");
    Serial.print("Cantidad nodos: ");
    Serial.println(handler_grafo.get_elem<tipo_grafo>()->size());
    for (
        tipo_grafo::const_iterator nodoA = handler_grafo.get_elem<tipo_grafo>()->begin();
        nodoA != handler_grafo.get_elem<tipo_grafo>()->end();
        nodoA++
        ) {
        Serial.print("Cantidad nodos (");
        Serial.print(nodoA->first);
        Serial.print("): ");
        Serial.println(nodoA->second.size());
        for (tipo_tabla::const_iterator nodoB = nodoA->second.begin(); nodoB != nodoA->second.end();nodoB++) {
            Serial.print("Nodo ");
            Serial.print(nodoA->first);
            Serial.print(" -> Nodo ");
            Serial.print(nodoB->first);
            Serial.print(": ");
            Serial.println(nodoB->second, 4);
        }
    }
    Serial.println("----- Fin Mapa -----");
}

void Mapa::print_tabla() const {
    Serial.print("Cantidad Elementos: ");
    Serial.println(handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->size());
    for (unsigned i = 0; i < handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->bucket_count(); ++i) {
        // if (handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->bucket_size(i) <= 0) continue;
        uint8_t* ptr1 = reinterpret_cast<uint8_t*>(&(*(
            handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->begin(i)
            )) + handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->bucket_size(i)) - 1;
        Memory_handler& handler = memoria.get_handler_from_ptr(static_cast<void*>(&(*(handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->begin(i)))));
        uint8_t* ptr2 = static_cast<uint8_t*>(handler.get_child().get_block_start()) + handler.get_child().get_block_size() - 1;
        if (handler.es_valido() && ptr1 != ptr2) Serial.println("Incongruencia detectada!");
        Serial.print("----- Bucket ");
        Serial.print(i + 1);
        Serial.print("/");
        Serial.print(handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->bucket_count());
        Serial.println(" -----");
        if (handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->bucket_size(i) > 0) {
            Serial.print("llave: ");
            Serial.println(handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->begin(i)->first);
        }
        Serial.print("Dir begin tabla: ");
        Serial.println((uintptr_t) & (*(handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->begin(i))), HEX);
        Serial.print("Dir end tabla: ");
        Serial.println((uintptr_t)(reinterpret_cast<uint8_t*>(&(*(
            handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->begin(i)
            )) + handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->bucket_size(i)) - 1), HEX);
        Serial.print("cantidad elementos: ");
        Serial.println(handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->bucket_size(i));
        handler.print();
        if (handler.es_valido())
            handler.get_child().print();
        Serial.print("----- Fin Bucket ");
        Serial.print(i + 1);
        Serial.print("/");
        Serial.print(handler_tabla_distancias_desde_inicio.get_elem<tipo_tabla>()->bucket_count());
        Serial.println(" -----");
    }
}

void Mapa::print_grafo() const {
    // for (unsigned i = 0; i < handler_grafo.get_elem<tipo_tabla>()->bucket_count(); ++i) {
    //     if (handler_grafo.get_elem<tipo_grafo>()->bucket_size(i) <= 0) continue;
    //     Serial.print("----- Bucket ");
    //     Serial.print(i + 1);
    //     Serial.print("/");
    //     Serial.print(handler_grafo.get_elem<tipo_grafo>()->bucket_count());
    //     Serial.println(" -----");

    //     Serial.print("Dir begin tabla: ");
    //     Serial.println((uintptr_t) & (*(handler_grafo.get_elem<tipo_grafo>()->begin(i))), HEX);
    //     Serial.print("Dir end tabla: ");
    //     Serial.println((uintptr_t)(reinterpret_cast<uint8_t*>(&(*(
    //         handler_grafo.get_elem<tipo_grafo>()->begin(i)
    //         )) + handler_grafo.get_elem<tipo_grafo>()->bucket_size(i)) - 1), HEX);
    //     Serial.print("cantidad elementos: ");
    //     Serial.println(handler_grafo.get_elem<tipo_grafo>()->bucket_size(i));
    //     Memory_handler& handler = memoria.get_handler_from_ptr(static_cast<void*>(&(*(handler_grafo.get_elem<tipo_grafo>()->begin(i)))));
    //     handler.print();
    //     if (handler.es_valido())
    //         handler.get_child().print();

    //     Serial.print("----- Fin Bucket ");
    //     Serial.print(i + 1);
    //     Serial.print("/");
    //     Serial.print(handler_grafo.get_elem<tipo_grafo>()->bucket_count());
    //     Serial.println(" -----");
    // }
}