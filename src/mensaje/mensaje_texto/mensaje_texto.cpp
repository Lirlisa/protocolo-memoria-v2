#include <mensaje/mensaje_texto/mensaje_texto.hpp>
#include <texto/texto.hpp>
#include <memory_allocator/memory_allocator.hpp>
#include <memory_allocator/memory_handler/memory_handler.hpp>
#include <cstdint>
#include <Arduino.h>
#include <algorithm>
#include <unishox2.h>

Mensaje_texto::Mensaje_texto(Memory_allocator& _memoria) : Mensaje(_memoria) {
    tipo_payload = Mensaje::PAYLOAD_TEXTO;
}

Mensaje_texto::Mensaje_texto(
    uint32_t _ttr, uint16_t _emisor, uint16_t _receptor,
    uint16_t _nonce, Memory_handler& payload_externo_handler, int _payload_size,
    Memory_allocator& _memoria
) : Mensaje(
    _ttr, _emisor, _receptor,
    _nonce, Mensaje::PAYLOAD_TEXTO, payload_externo_handler, _payload_size,
    _memoria
) {
    uint16_t aux_nonce, aux_creador, aux_destinatario;
    uint8_t aux_saltos, * _payload = payload_handler.get_elem<uint8_t>(), pos_actual = 0;
    int largo_texto_comprimido, pos_inicio_texto, _cantidad_textos = 0, pos_size_contenido = Texto::size_variables_transmission - 1;
    char* payload_txt;
    do {
        if (
            _payload[pos_size_contenido] <= 0 ||
            _payload[pos_size_contenido] > Texto::max_largo_contenido_comprimido
            ) break; // está corrupto y no se puede saber que sigue con certeza
        _cantidad_textos++;
        pos_size_contenido += _payload[pos_size_contenido] + Texto::size_variables_transmission;
    } while (pos_size_contenido < payload_size);

    cantidad_textos = _cantidad_textos;
    arreglo_textos_handler = &memoria.acquire<Texto>(sizeof(Texto) * cantidad_textos, memoria);
    pos_inicio_texto = 0;
    _payload = payload_handler.get_elem<uint8_t>();
    for (uint8_t i = 0; i < cantidad_textos; i++) {
        largo_texto_comprimido = _payload[pos_inicio_texto + Texto::size_variables_transmission - 1];
        if (largo_texto_comprimido >= 0) {
            memcpy(&aux_nonce, _payload + pos_inicio_texto, 2);
            memcpy(&aux_creador, _payload + pos_inicio_texto + 2, 2);
            memcpy(&aux_destinatario, _payload + pos_inicio_texto + 4, 2);
            aux_saltos = _payload[pos_inicio_texto + 6]; // 1 byte
            Memory_handler& payload_txt_handler = memoria.acquire<char>(largo_texto_comprimido);
            payload_txt = payload_txt_handler.get_elem<char>();
            _payload = payload_handler.get_elem<uint8_t>();
            std::memcpy(
                payload_txt,
                reinterpret_cast<char*>(_payload + pos_inicio_texto + Texto::size_variables_transmission),
                largo_texto_comprimido
            );
            new (arreglo_textos_handler->get_elem<Texto>() + pos_actual) Texto(memoria);
            arreglo_textos_handler->get_elem<Texto>()[pos_actual++].update(
                aux_nonce, aux_creador, aux_destinatario, aux_saltos, largo_texto_comprimido,
                payload_txt_handler, true
            );
            memoria.release<char>(payload_txt_handler, largo_texto_comprimido);
        }
        pos_inicio_texto += Texto::size_variables_transmission + (largo_texto_comprimido > 0 ? largo_texto_comprimido : 0);
    }
}

/*
@brief Crea una instancia de mensaje_texto basada en un mensaje base
*/
Mensaje_texto::Mensaje_texto(Memory_handler& mensaje_origen_handler, Memory_allocator& _memoria)
    : Mensaje(mensaje_origen_handler, _memoria) {
    uint16_t aux_nonce, aux_creador, aux_destinatario;
    uint8_t aux_saltos, * _payload = payload_handler.get_elem<uint8_t>(), pos_actual = 0;
    int largo_texto_comprimido, pos_inicio_texto, _cantidad_textos = 0, pos_size_contenido = Texto::size_variables_transmission - 1;
    char* payload_txt;
    do {
        if (
            _payload[pos_size_contenido] <= 0 ||
            _payload[pos_size_contenido] > Texto::max_largo_contenido_comprimido
            ) break; // está corrupto y no se puede saber que sigue con certeza
        _cantidad_textos++;
        pos_size_contenido += _payload[pos_size_contenido] + Texto::size_variables_transmission;
    } while (pos_size_contenido < payload_size);

    cantidad_textos = _cantidad_textos;
    arreglo_textos_handler = &memoria.acquire<Texto>(sizeof(Texto) * cantidad_textos, memoria);

    pos_inicio_texto = 0;
    for (uint8_t i = 0; i < _cantidad_textos; i++) {
        largo_texto_comprimido = _payload[pos_inicio_texto + Texto::size_variables_transmission - 1];
        if (largo_texto_comprimido >= 0) {
            memcpy(&aux_nonce, _payload + pos_inicio_texto, 2);
            memcpy(&aux_creador, _payload + pos_inicio_texto + 2, 2);
            memcpy(&aux_destinatario, _payload + pos_inicio_texto + 4, 2);
            aux_saltos = _payload[pos_inicio_texto + 6]; // 1 byte
            Memory_handler& payload_txt_handler = memoria.acquire<char>(largo_texto_comprimido);
            payload_txt = payload_txt_handler.get_elem<char>();
            _payload = payload_handler.get_elem<uint8_t>();
            std::memcpy(
                payload_txt,
                reinterpret_cast<char*>(_payload + pos_inicio_texto + Texto::size_variables_transmission),
                largo_texto_comprimido
            );
            new (arreglo_textos_handler->get_elem<Texto>() + pos_actual) Texto(memoria);
            arreglo_textos_handler->get_elem<Texto>()[pos_actual++].update(
                aux_nonce, aux_creador, aux_destinatario, aux_saltos, largo_texto_comprimido,
                payload_txt_handler, true
            );
            memoria.release<char>(payload_txt_handler, largo_texto_comprimido);
        }
        pos_inicio_texto += Texto::size_variables_transmission + (largo_texto_comprimido > 0 ? largo_texto_comprimido : 0);
    }
}

/*
@brief Crea una instancia de mensaje_texto basada en otro mensaje_texto
*/
Mensaje_texto::Mensaje_texto(Memory_allocator& _memoria, Memory_handler& mensaje_texto_origen_handler)
    : Mensaje(
        mensaje_texto_origen_handler.get_elem<Mensaje_texto>()->ttr,
        mensaje_texto_origen_handler.get_elem<Mensaje_texto>()->emisor,
        mensaje_texto_origen_handler.get_elem<Mensaje_texto>()->receptor,
        mensaje_texto_origen_handler.get_elem<Mensaje_texto>()->nonce,
        Mensaje_texto::PAYLOAD_TEXTO,
        mensaje_texto_origen_handler.get_elem<Mensaje_texto>()->payload_handler,
        mensaje_texto_origen_handler.get_elem<Mensaje_texto>()->payload_size, _memoria
    ) {
    if (mensaje_texto_origen_handler.get_elem<Mensaje_texto>()->get_cantidad_textos() <= 0) return;
    cantidad_textos = mensaje_texto_origen_handler.get_elem<Mensaje_texto>()->get_cantidad_textos();
    arreglo_textos_handler = &memoria.acquire<Texto>(sizeof(Texto) * cantidad_textos, memoria);
    for (unsigned i = 0; i < mensaje_texto_origen_handler.get_elem<Mensaje_texto>()->get_cantidad_textos(); ++i) {
        new (arreglo_textos_handler->get_elem<Texto>() + i) Texto(memoria);
        arreglo_textos_handler->get_elem<Texto>()[i].update(
            mensaje_texto_origen_handler.get_elem<Mensaje_texto>()->arreglo_textos_handler->get_elem<Texto>() + i
        );
    }
}

Mensaje_texto::~Mensaje_texto() {
    if (arreglo_textos_handler != nullptr)
        memoria.release<Texto>(*arreglo_textos_handler, cantidad_textos);
    arreglo_textos_handler = nullptr;
}

void Mensaje_texto::print() {
    Texto* textos;
    Mensaje::print();
    Serial.print("Textos (");
    Serial.print(cantidad_textos);
    Serial.println("):");
    for (unsigned i = 0; i < cantidad_textos; ++i) {
        Serial.print("\tTexto ");
        Serial.print(i + 1);
        Serial.print("/");
        Serial.println(cantidad_textos);
        textos = arreglo_textos_handler->get_elem<Texto>();
        textos[i].print();
    }
}

Memory_handler& Mensaje_texto::obtener_textos() {
    return *arreglo_textos_handler;
}

uint8_t Mensaje_texto::get_cantidad_textos() const {
    return cantidad_textos;
}

void Mensaje_texto::update(
    uint32_t _ttr, uint16_t _emisor, uint16_t _receptor,
    uint16_t _nonce, Memory_handler& payload_externo_handler, unsigned _payload_size
) {
    ttr = _ttr;
    emisor = _emisor;
    receptor = _receptor;
    nonce = _nonce;
    payload_size = _payload_size;
    std::memcpy(payload_handler.get_elem<uint8_t>(), payload_externo_handler.get_elem<uint8_t>(), payload_size);

    uint16_t aux_nonce, aux_creador, aux_destinatario;
    uint8_t aux_saltos, * _payload = payload_handler.get_elem<uint8_t>(), pos_actual = 0;
    int largo_texto_comprimido, pos_inicio_texto, _cantidad_textos = 0, pos_size_contenido = Texto::size_variables_transmission - 1;
    char* payload_txt;
    do {
        if (
            _payload[pos_size_contenido] <= 0 ||
            _payload[pos_size_contenido] > Texto::max_largo_contenido_comprimido
            ) break; // está corrupto y no se puede saber que sigue con certeza
        _cantidad_textos++;
        pos_size_contenido += _payload[pos_size_contenido] + Texto::size_variables_transmission;
    } while (pos_size_contenido < payload_size);

    cantidad_textos = _cantidad_textos;
    arreglo_textos_handler = &memoria.acquire<Texto>(sizeof(Texto) * cantidad_textos, memoria);

    pos_inicio_texto = 0;
    for (uint8_t i = 0; i < _cantidad_textos; i++) {
        largo_texto_comprimido = _payload[pos_inicio_texto + Texto::size_variables_transmission - 1];
        if (largo_texto_comprimido >= 0) {
            memcpy(&aux_nonce, _payload + pos_inicio_texto, 2);
            memcpy(&aux_creador, _payload + pos_inicio_texto + 2, 2);
            memcpy(&aux_destinatario, _payload + pos_inicio_texto + 4, 2);
            aux_saltos = _payload[pos_inicio_texto + 6]; // 1 byte
            Memory_handler& payload_txt_handler = memoria.acquire<char>(largo_texto_comprimido);
            payload_txt = payload_txt_handler.get_elem<char>();
            _payload = payload_handler.get_elem<uint8_t>();
            std::memcpy(
                payload_txt,
                reinterpret_cast<char*>(_payload + pos_inicio_texto + Texto::size_variables_transmission),
                largo_texto_comprimido
            );
            new (arreglo_textos_handler->get_elem<Texto>() + pos_actual) Texto(memoria);
            arreglo_textos_handler->get_elem<Texto>()[pos_actual++].update(
                aux_nonce, aux_creador, aux_destinatario, aux_saltos, largo_texto_comprimido,
                payload_txt_handler, true
            );
            memoria.release<char>(payload_txt_handler, largo_texto_comprimido);
        }
        pos_inicio_texto += Texto::size_variables_transmission + (largo_texto_comprimido > 0 ? largo_texto_comprimido : 0);
    }
}

void Mensaje_texto::update(const Mensaje_texto& origen) {
    update(
        origen.getTTR(), origen.getEmisor(), origen.getReceptor(),
        origen.getNonce(), origen.payload_handler, origen.getPayloadSize()
    );
}

void Mensaje_texto::update(const Mensaje& origen) {
    update(
        origen.getTTR(), origen.getEmisor(), origen.getReceptor(),
        origen.getNonce(), origen.payload_handler, origen.getPayloadSize()
    );
}

Mensaje_texto_generator::Mensaje_texto_generator(
    Memory_handler& _fuente_handler, uint32_t _ttr, uint16_t _emisor, uint16_t _receptor,
    uint16_t _nonce, Memory_handler& _handler_arreglo_textos, std::size_t _cantidad_textos,
    Memory_allocator& _memoria
) : fuente_handler(_fuente_handler), memoria(_memoria), handler_arreglo_textos(_handler_arreglo_textos) {
    ttr = _ttr;
    emisor = _emisor;
    receptor = _receptor;
    nonce = _nonce;
    pos_texto_actual = 0;
    pos_texto_final = _cantidad_textos - 1;
    cantidad_textos_restantes = _cantidad_textos;
}

/*
@brief Modifica la fuente entregada en el constructor para que contenga los nuevos textos.
@return La cantidad de textos pendientes.
*/
std::size_t Mensaje_texto_generator::next() {
    if (cantidad_textos_restantes == 0 || pos_texto_actual >= pos_texto_final) return 0;

    int textos_restantes = cantidad_textos_restantes;
    int bytes_restantes;
    std::size_t curr_texto_pos = pos_texto_actual;
    std::size_t ultimo_texto_a_enviar_pos = 0;
    unsigned contador;
    uint8_t size_text;
    bytes_restantes = Mensaje_texto::payload_max_size;
    Memory_handler& payload_handler = memoria.acquire<uint8_t>(bytes_restantes);
    Memory_handler& data_handler = memoria.acquire<uint8_t>(Texto::max_largo_contenido_comprimido);
    uint8_t* _payload = payload_handler.get_elem<uint8_t>();
    uint8_t* data = data_handler.get_elem<uint8_t>();

    Texto* arreglo_textos = handler_arreglo_textos.get_elem<Texto>();
    for (std::size_t pos_texto = curr_texto_pos; pos_texto != pos_texto_final; ++pos_texto) {
        if (bytes_restantes - arreglo_textos[pos_texto].transmission_size() < 0 || textos_restantes <= 0) break;
        bytes_restantes -= arreglo_textos[pos_texto].transmission_size();
        textos_restantes--;
        ultimo_texto_a_enviar_pos = pos_texto;
    }

    contador = 0;
    for (std::size_t pos_texto = curr_texto_pos; pos_texto != ultimo_texto_a_enviar_pos + 1; pos_texto++) {
        size_text = arreglo_textos[pos_texto].transmission_size();
        arreglo_textos[pos_texto].parse_to_transmission(data_handler);
        std::memcpy(_payload + contador, data, size_text);
        contador += size_text;
    }
    fuente_handler.get_elem<Mensaje_texto>()->update(
        ttr, emisor, receptor,
        nonce, payload_handler, Mensaje_texto::payload_max_size - bytes_restantes
    );

    pos_texto_actual = ultimo_texto_a_enviar_pos + 1;
    cantidad_textos_restantes = textos_restantes;

    memoria.release<uint8_t>(payload_handler, Mensaje_texto::payload_max_size);
    memoria.release<uint8_t>(data_handler, Texto::max_largo_contenido_comprimido);

    return cantidad_textos_restantes;
}