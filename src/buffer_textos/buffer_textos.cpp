#include <buffer_textos/buffer_textos.hpp>
#include <mensaje/mensaje_vector/mensaje_vector.hpp>
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <new>
#include <Arduino.h>

#define INITIAL_CAPACITY 15

Buffer_textos::Buffer_textos(uint16_t _id, Mapa& _mapa) :
    mapa(_mapa),
    handler_arreglo_textos_threshold_saltos(memoria.acquire<vec_texto>(sizeof(vec_texto))),
    handler_arreglo_textos_mas_de_threshold_saltos(memoria.acquire<vec_texto>(sizeof(vec_texto))),
    handler_acks(memoria.acquire<set_hash>(sizeof(set_hash))),
    handler_textos_ya_vistos(memoria.acquire<set_hash>(sizeof(set_hash))),
    handler_set_evitar(memoria.acquire<set_hash>(sizeof(set_hash))) {

    // reserva de espacio
    handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()->reserve(INITIAL_CAPACITY);
    handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()->reserve(INITIAL_CAPACITY);
    handler_acks.get_elem<set_hash>()->reserve(INITIAL_CAPACITY);
    handler_textos_ya_vistos.get_elem<set_hash>()->reserve(INITIAL_CAPACITY);
    handler_set_evitar.get_elem<set_hash>()->reserve(INITIAL_CAPACITY);
    id = _id;
}

Buffer_textos::~Buffer_textos() {
    memoria.release<vec_texto>(handler_arreglo_textos_threshold_saltos);
    memoria.release<vec_texto>(handler_arreglo_textos_mas_de_threshold_saltos);
    memoria.release<set_hash>(handler_acks);
    memoria.release<set_hash>(handler_textos_ya_vistos);
    memoria.release<set_hash>(handler_set_evitar);
}

bool Buffer_textos::_comparador_textos_saltos(const Texto& a, const Texto& b) {
    return a.saltos < b.saltos;
}

unsigned Buffer_textos::size() {
    return handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()->size() + handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()->size();
}

void Buffer_textos::eliminar_texto() {
    if (handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()->size() > 0)
        handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()->erase(
            handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()->end() - 1
        );
    else if (handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()->size() > 0)
        handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()->erase(
            handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()->end() - 1
        );
}

bool Buffer_textos::texto_en_acks(const Texto& texto) const {
    return handler_acks.get_elem<set_hash>()->find(texto.hash()) != handler_acks.get_elem<set_hash>()->end();
}

bool Buffer_textos::texto_en_ya_vistos(const Texto& texto) const {
    return texto_en_ya_vistos(texto.hash());
}

bool Buffer_textos::texto_en_ya_vistos(const uint64_t hash) const {
    return handler_textos_ya_vistos.get_elem<set_hash>()->find(hash) != handler_textos_ya_vistos.get_elem<set_hash>()->end();
}

void Buffer_textos::agregar_ack(uint64_t ack) {
    if (handler_acks.get_elem<set_hash>()->find(ack) == handler_acks.get_elem<set_hash>()->end())
        handler_acks.get_elem<set_hash>()->insert(ack);

    auto se_remueve = [ack](Texto& texto) { return texto.hash() == ack; };

    auto it = std::remove_if(handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()->begin(), handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()->end(), se_remueve);
    handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()->erase(
        it,
        handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()->end()
    );

    it = std::remove_if(handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()->begin(), handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()->end(), se_remueve);
    handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()->erase(
        it,
        handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()->end()
    );
}

void Buffer_textos::agregar_ack(Memory_handler& handler_textos, std::size_t cantidad_textos) {
    for (std::size_t i = 0; i < cantidad_textos; i++)
        agregar_ack(handler_textos.get_elem<Texto>()[i].hash());
}

unsigned Buffer_textos::insertar_en_arr_bajo_thresold(const Texto& texto) {
    unsigned i = 0;
    while (i < handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()->size()) {
        if (texto.saltos < handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()->at(i).saltos) break;
        i++;
    }
    handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()->emplace(
        handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()->begin() + i,
        texto
    );
    return i;
}

unsigned Buffer_textos::insertar_en_arr_mas_de_thresold(const Texto& texto) {
    float prob = obtener_probabilidad_entrega(texto);
    unsigned i = 0;
    while (i < handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()->size()) {
        if (prob >= obtener_probabilidad_entrega(handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()->at(i))) break;
        i++;
    };
    handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()->emplace(
        handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()->begin() + i,
        texto
    );
    return i;
}

void Buffer_textos::agregar_texto(const Texto& texto) {
    if (texto_en_acks(texto)) return;
    if (texto_en_ya_vistos(texto)) return;
    Memory_handler& handler_texto = memoria.acquire_simple<Texto>(sizeof(Texto));
    handler_texto.congelar_bloque();
    new (handler_texto.get_elem<Texto>()) Texto(texto);
    Texto& nuevo_texto = *handler_texto.get_elem<Texto>();
    nuevo_texto.saltos += 1;
    agregar_a_ya_vistos(nuevo_texto);
    if (nuevo_texto.saltos <= saltos_threshold)
        insertar_en_arr_bajo_thresold(nuevo_texto);
    else
        insertar_en_arr_mas_de_thresold(nuevo_texto);
    handler_texto.descongelar_bloque();
    memoria.release<Texto>(handler_texto);
}

void Buffer_textos::agregar_texto(Memory_handler& handler_textos, unsigned cantidad_textos) {
    for (std::size_t i = 0; i < cantidad_textos; i++) {
        if (texto_en_ya_vistos(handler_textos.get_elem<Texto>()[i])) continue;
        if (texto_en_acks(handler_textos.get_elem<Texto>()[i])) continue;
        Memory_handler& handler_texto = memoria.acquire_simple<Texto>(sizeof(Texto));
        handler_texto.congelar_bloque();
        new (handler_texto.get_elem<Texto>()) Texto(handler_textos.get_elem<Texto>()[i]);
        Texto& nuevo_texto = *handler_texto.get_elem<Texto>();
        nuevo_texto.saltos += 1;
        agregar_a_ya_vistos(nuevo_texto);
        if (nuevo_texto.saltos <= saltos_threshold)
            insertar_en_arr_bajo_thresold(nuevo_texto);
        else
            insertar_en_arr_mas_de_thresold(nuevo_texto);
        handler_texto.descongelar_bloque();
        memoria.release<Texto>(handler_texto);
    }
}

/*
@brief entrega probabilidad de entregar al nodo destino del texto dado.
*/
float Buffer_textos::obtener_probabilidad_entrega(const Texto& texto) {
    return mapa.costo(texto.destinatario);
}

bool Buffer_textos::hay_mensajes_bajo_threshold() {
    return handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()->size() > 0;
}

bool Buffer_textos::hay_mensajes_sobre_threshold() {
    return handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()->size() > 0;
}

/*
@brief Almacena los textos en el arreglo textos de handler_arreglo_textos.
@warning handler_arreglo_texto debe poder contener todos los textos.
*/
void Buffer_textos::obtener_textos_destinatario(uint16_t id_destinatario, Memory_handler& handler_arreglo_textos) const {
    std::size_t i = 0, j = 0;
    for (Texto& texto : *handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()) {
        if (texto.destinatario == id_destinatario)
            new (handler_arreglo_textos.get_elem<Texto>() + i++) Texto(texto);
    }
    for (Texto& texto : *handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()) {
        if (texto.destinatario == id_destinatario)
            new (handler_arreglo_textos.get_elem<Texto>() + i + j++) Texto(texto);
    }
}

std::size_t Buffer_textos::obtener_cantidad_textos_destinatario(uint16_t id_destinatario) const {
    std::size_t suma = 0;
    for (Texto& texto : *handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()) {
        if (texto.destinatario == id_destinatario) ++suma;
    }
    for (Texto& texto : *handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()) {
        if (texto.destinatario == id_destinatario) ++suma;
    }
    return suma;
}

uint64_t Buffer_textos::obtener_hash_texto(const Texto& texto) {
    return texto.hash();
}

/*
@brief Elimina los textos en array de textos a eliminar
*/
void Buffer_textos::eliminar_textos(Memory_handler& handler_textos_a_eliminar, std::size_t cantidad_textos) {
    handler_set_evitar.get_elem<set_hash>()->clear();
    for (std::size_t i = 0; i < cantidad_textos; ++i)
        handler_set_evitar.get_elem<set_hash>()->insert(handler_textos_a_eliminar.get_elem<Texto>()[i].hash());


    auto se_remueve = [this](Texto& text) { return handler_set_evitar.get_elem<set_hash>()->find(text.hash()) != handler_set_evitar.get_elem<set_hash>()->end(); };

    auto it = std::remove_if(
        handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()->begin(),
        handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()->end(),
        se_remueve
    );
    handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()->erase(it, handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()->end());

    it = std::remove_if(
        handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()->begin(),
        handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()->end(),
        se_remueve
    );
    handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()->erase(it, handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()->end());
}

std::size_t Buffer_textos::obtener_cantidad_textos_bajo_threshold() const {
    return handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()->size();
}
std::size_t Buffer_textos::obtener_cantidad_textos_sobre_threshold() const {
    return handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()->size();
}
void Buffer_textos::obtener_textos_bajo_threshold(Memory_handler& handler_arreglo_textos, std::size_t& cantidad_textos, long long int excluir_id, Memory_handler* handler_excepciones, std::size_t cantidad_textos_excepciones) {
    std::size_t pos = 0;
    if (excluir_id < 0 && handler_excepciones == nullptr) {
        for (Texto& text : *handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>())
            new (handler_arreglo_textos.get_elem<vec_texto>() + pos++) Texto(text);
        cantidad_textos = pos;
        return;
    }

    handler_set_evitar.get_elem<set_hash>()->clear();
    if (handler_excepciones != nullptr && cantidad_textos_excepciones > 0) {
        for (std::size_t i = 0; i < cantidad_textos_excepciones; ++i)
            handler_set_evitar.get_elem<set_hash>()->insert(handler_excepciones->get_elem<uint64_t>()[i]);
    }

    for (Texto& texto : *handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()) {
        if (excluir_id == texto.creador || handler_set_evitar.get_elem<set_hash>()->find(texto.hash()) != handler_set_evitar.get_elem<set_hash>()->end())
            continue;
        new (handler_arreglo_textos.get_elem<vec_texto>() + pos++) Texto(texto);
    }
    cantidad_textos = pos;
}

void Buffer_textos::obtener_textos_sobre_threshold(Memory_handler& handler_arreglo_textos, std::size_t& cantidad_textos, long long int excluir_id, Memory_handler* handler_excepciones, std::size_t cantidad_textos_excepciones) {
    std::size_t pos = 0;
    if (excluir_id < 0 && handler_excepciones == nullptr) {
        for (Texto& text : *handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>())
            new (handler_arreglo_textos.get_elem<vec_texto>() + pos++) Texto(text);
        cantidad_textos = pos;
        return;
    }

    handler_set_evitar.get_elem<set_hash>()->clear();
    if (handler_excepciones != nullptr && cantidad_textos_excepciones > 0) {
        for (std::size_t i = 0; i < cantidad_textos_excepciones; ++i)
            handler_set_evitar.get_elem<set_hash>()->insert(handler_excepciones->get_elem<uint64_t>()[i]);
    }

    for (Texto& texto : *handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()) {
        if (excluir_id == texto.creador || handler_set_evitar.get_elem<set_hash>()->find(texto.hash()) != handler_set_evitar.get_elem<set_hash>()->end())
            continue;
        new (handler_arreglo_textos.get_elem<vec_texto>() + pos++) Texto(texto);
    }
    cantidad_textos = pos;
}

std::size_t Buffer_textos::cantidad_acks() const {
    return handler_acks.get_elem<set_hash>()->size();
}

/*
@brief Almacena en el arreglo subyacente de handler_array_acks los ack.
@warning El arreglo debe tener el espacio suficiente para almacenar la cantidad necesaria de uint64_t
*/
void Buffer_textos::obtener_acks(Memory_handler& handler_array_acks) const {
    std::size_t i = 0;
    for (uint64_t ack : *handler_acks.get_elem<set_hash>())
        handler_array_acks.get_elem<uint64_t>()[i++] = ack;
}

void Buffer_textos::print() const {
    Serial.println("Mensajes en buffer:");
    Serial.print("Cantidad mensajes en arreglo_textos_threshold_saltos: ");
    Serial.println(handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()->size());

    for (Texto& texto : *handler_arreglo_textos_threshold_saltos.get_elem<vec_texto>()) {
        texto.print();
        Serial.println("--------");
    }
    Serial.print("Cantidad mensajes en arreglo_textos_mas_de_threshold_saltos: ");
    Serial.println(handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()->size());
    for (Texto& texto : *handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()) {
        texto.print();
        Serial.println("--------");
    }
}

void Buffer_textos::eliminar_textos_sobre_threshold() {
    handler_arreglo_textos_mas_de_threshold_saltos.get_elem<vec_texto>()->clear();
}

void Buffer_textos::print_mapa() const {
    mapa.print();
}

void Buffer_textos::agregar_a_ya_vistos(uint64_t hash) {
    if (!texto_en_ya_vistos(hash))
        handler_textos_ya_vistos.get_elem<set_hash>()->insert(hash);
}

void Buffer_textos::agregar_a_ya_vistos(const Texto& texto) {
    if (!texto_en_ya_vistos(texto))
        handler_textos_ya_vistos.get_elem<set_hash>()->insert(texto.hash());
}

void Buffer_textos::agregar_a_ya_vistos(Memory_handler& arreglo_hashes_handler, std::size_t cantidad_hashes) {
    for (std::size_t i = 0; i < cantidad_hashes; ++i)
        agregar_a_ya_vistos(arreglo_hashes_handler.get_elem<uint64_t>()[i]);
}

std::size_t Buffer_textos::obtener_cantidad_textos_ya_vistos() const {
    return handler_textos_ya_vistos.get_elem<set_hash>()->size();
}

void Buffer_textos::obtener_textos_ya_vistos(Memory_handler& handler_arreglo_hashes, std::size_t& cantidad_hashes, long long excluir_id) const {
    uint16_t destinatario;
    std::size_t pos = 0;
    if (excluir_id >= 0) {
        for (uint64_t hash : *handler_textos_ya_vistos.get_elem<set_hash>()) {
            std::memcpy(&destinatario, (&hash) + 4, 2);
            if (destinatario != excluir_id)
                handler_arreglo_hashes.get_elem<uint64_t>()[pos++] = hash;
        }
    }
    else {
        for (uint64_t hash : *handler_textos_ya_vistos.get_elem<set_hash>()) {
            handler_arreglo_hashes.get_elem<uint64_t>()[pos++] = hash;
        }
    }

    cantidad_hashes = pos;
}