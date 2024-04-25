#ifndef BUFFER_TEXTOS_HPP
#define BUFFER_TEXTOS_HPP

#include <texto/texto.hpp>
#include <mapa/mapa.hpp>
#include <allocator/allocator.hpp>
#include <vector>
#include <unordered_set>
#include <cstdint>


class Buffer_textos {
private:
    Memory_allocator& memoria = Memoria::memoria;
    Mapa& mapa;

    using vec_texto = std::vector<Texto, CAllocator<Texto>>;
    using set_hash = std::unordered_set<uint64_t, std::hash<uint64_t>, std::equal_to<uint64_t>, CAllocator<uint64_t>>;

    Memory_handler& handler_arreglo_textos_threshold_saltos; // vec_texto
    Memory_handler& handler_arreglo_textos_mas_de_threshold_saltos; // vec_texto
    Memory_handler& handler_acks; // set_hash
    Memory_handler& handler_textos_ya_vistos; // set_hash
    Memory_handler& handler_set_evitar; // set_hash


    uint16_t id;
    uint8_t saltos_threshold = 1;

    bool _comparador_textos_saltos(const Texto& a, const Texto& b);
    bool texto_en_acks(const Texto& texto) const;
    bool texto_en_ya_vistos(const Texto& texto) const;
    bool texto_en_ya_vistos(const uint64_t hash) const;
    float obtener_probabilidad_entrega(const Texto& texto);
    static uint64_t obtener_hash_texto(const Texto& texto);

    unsigned insertar_en_arr_bajo_thresold(const Texto& texto);
    unsigned insertar_en_arr_mas_de_thresold(const Texto& texto);

    void agregar_a_ya_vistos(uint64_t hash);
    void agregar_a_ya_vistos(const Texto& texto);
    void agregar_a_ya_vistos(Memory_handler& arreglo_hashes_handler, std::size_t cantidad_hashes);

public:
    Buffer_textos(uint16_t _id, Mapa& _mapa);
    Buffer_textos(Buffer_textos&) = delete;
    Buffer_textos& operator=(const Buffer_textos&) = delete;
    ~Buffer_textos();

    std::size_t size();
    bool hay_mensajes_bajo_threshold();
    bool hay_mensajes_sobre_threshold();

    void agregar_ack(uint64_t ack);
    void agregar_ack(Memory_handler& handler_textos, std::size_t cantidad_textos);
    void agregar_texto(const Texto& texto);
    void agregar_texto(Memory_handler& handler_textos, std::size_t cantidad_textos);
    void obtener_textos_destinatario(uint16_t id_destinatario, Memory_handler& handler_arreglo_textos) const;
    std::size_t obtener_cantidad_textos_destinatario(uint16_t id_destinatario) const;

    void eliminar_texto();
    void eliminar_textos(Memory_handler& handler_textos_a_eliminar, std::size_t cantidad_textos);

    void obtener_textos_bajo_threshold(Memory_handler& handler_arreglo_textos, std::size_t& cantidad_textos, long long int excluir_id = -1, Memory_handler* handler_excepciones = nullptr, std::size_t cantidad_textos_excepciones = 0);
    void obtener_textos_sobre_threshold(Memory_handler& handler_arreglo_textos, std::size_t& cantidad_textos, long long int excluir_id = -1, Memory_handler* handler_excepciones = nullptr, std::size_t cantidad_textos_excepciones = 0);
    std::size_t obtener_cantidad_textos_bajo_threshold() const;
    std::size_t obtener_cantidad_textos_sobre_threshold() const;

    std::size_t cantidad_acks() const;
    void obtener_acks(Memory_handler& handler_array_acks) const;

    void print() const;

    void eliminar_textos_sobre_threshold();

    void print_mapa() const;

    void obtener_textos_ya_vistos(Memory_handler& handler_arreglo_hashes, std::size_t& cantidad_hashes, long long excluir_id = -1) const;
    std::size_t obtener_cantidad_textos_ya_vistos() const;
};

#endif