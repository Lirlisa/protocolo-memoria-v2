#include "texto.hpp"
#include <Arduino.h>

Texto::Texto(Memory_allocator& _memoria) : memoria(_memoria) { }

Texto::Texto(
    uint16_t _nonce, uint16_t _creador, uint16_t _destinatario,
    uint8_t _saltos, int _largo_texto,
    Memory_handler& contenido_handler, Memory_allocator& _memoria, bool comprimido
) : memoria(_memoria) {
    Serial.print("Pos Interna Texto: ");
    Serial.println((uintptr_t)this, HEX);
    nonce = _nonce;
    creador = _creador;
    destinatario = _destinatario;
    saltos = _saltos;
    if (_largo_texto < 1) return;
    if (comprimido) { //si el contenido viene o no comprimido
        if (_largo_texto > (int)max_largo_contenido_comprimido) {
            largo_texto_comprimido = largo_texto = 0;
            contenido_comprimido_handler = nullptr;
            return;
        }
        largo_texto_comprimido = _largo_texto;
        contenido_comprimido_handler = &memoria.acquire<char>(largo_texto_comprimido);
        std::memcpy(contenido_comprimido_handler->get_elem<char>(), contenido_handler.get_elem<char>(), largo_texto_comprimido);

        //descompresión
        Memory_handler& temp_descomprimido_handler = memoria.acquire<char>(4 * largo_texto_comprimido);
        largo_texto = unishox2_decompress_simple(
            contenido_handler.get_elem<char>(),
            largo_texto_comprimido,
            temp_descomprimido_handler.get_elem<char>()
        );
        memoria.release<char>(temp_descomprimido_handler, 4 * largo_texto_comprimido);
    }
    else {
        //compresión
        largo_texto = _largo_texto;
        Memory_handler& temp_comprimido_handler = memoria.acquire<char>(2 * largo_texto);
        int _largo_texto_comprimido = unishox2_compress_simple(
            contenido_handler.get_elem<char>(),
            largo_texto,
            temp_comprimido_handler.get_elem<char>()
        );
        if (_largo_texto_comprimido > (int)max_largo_contenido_comprimido) {
            largo_texto_comprimido = largo_texto = 0;
            contenido_comprimido_handler = nullptr;
            memoria.release<char>(temp_comprimido_handler, 2 * largo_texto);
            return;
        }
        largo_texto_comprimido = _largo_texto_comprimido;
        contenido_comprimido_handler = &memoria.acquire<char>(largo_texto_comprimido);
        std::memcpy(
            contenido_comprimido_handler->get_elem<char>(),
            temp_comprimido_handler.get_elem<char>(),
            largo_texto_comprimido
        );
        memoria.release<char>(temp_comprimido_handler, 2 * largo_texto);
    }
    valido = true;
}

Texto::~Texto() {
    if (contenido_comprimido_handler != nullptr)
        memoria.release<char>(*contenido_comprimido_handler, largo_texto_comprimido);
    valido = false;
    largo_texto = 0;
    largo_texto_comprimido = 0;
}

Texto& Texto::operator=(const Texto& other) {
    if (this == &other)
        return *this;

    if (contenido_comprimido_handler != nullptr)
        memoria.release<char>(*contenido_comprimido_handler, largo_texto_comprimido);
    nonce = other.nonce;
    creador = other.creador;
    destinatario = other.destinatario;
    saltos = other.saltos;
    valido = other.valido;
    largo_texto = other.largo_texto;
    largo_texto_comprimido = other.largo_texto_comprimido;

    if (valido && largo_texto_comprimido > 0) {
        Serial.println("Flag Texto::operator= 12");
        contenido_comprimido_handler = &memoria.acquire<char>(largo_texto_comprimido);
        Serial.println("Flag Texto::operator= 13");
        Serial.println(largo_texto_comprimido);
        Serial.println(memoria.available_memory());
        Serial.println((uintptr_t)contenido_comprimido_handler->get_elem<char>(), HEX);
        Serial.println((uintptr_t)other.contenido_comprimido_handler->get_elem<char>(), HEX);
        std::memcpy(
            contenido_comprimido_handler->get_elem<char>(),
            other.contenido_comprimido_handler->get_elem<char>(),
            largo_texto_comprimido
        );
        Serial.println("Flag Texto::operator= 14");
    }
    Serial.println("Flag Texto::operator= 15");
    return *this;
}

/*
@brief convierte los datos en un arreglo de bytes listos para transmitir. Destino debe tener al menos 'size_variables_transmission' + 'largo_texto_comprimido' bytes disponibles.
*/
void Texto::parse_to_transmission(Memory_handler& destino_handler) const {
    uint8_t* destino = destino_handler.get_elem<uint8_t>();
    std::memcpy(destino, &nonce, 2);
    std::memcpy(destino + 2, &creador, 2);
    std::memcpy(destino + 4, &destinatario, 2);
    std::memcpy(destino + 6, &saltos, 1);
    std::memcpy(destino + 7, &largo_texto_comprimido, 1);
    std::memcpy(destino + 8, contenido_comprimido_handler->get_elem<char>(), largo_texto_comprimido);
}

/*
@brief Obtiene el hash del texto.
*/
uint64_t Texto::hash() const {
    uint64_t data = 0;
    data |= ((uint64_t)nonce) << 48;
    data |= ((uint64_t)creador) << 32;
    data |= ((uint64_t)destinatario) << 16;
    return data;
}

uint8_t Texto::transmission_size() const {
    return size_variables_transmission + largo_texto_comprimido;
}

void Texto::print() {
    Memory_handler& contenido_handler = memoria.acquire<char>(largo_texto + 1);
    char* _contenido = contenido_handler.get_elem<char>();
    unishox2_decompress_simple(contenido_comprimido_handler->get_elem<char>(), largo_texto_comprimido, _contenido);
    _contenido[largo_texto] = '\0';
    Serial.print("\tValido: ");
    Serial.println(valido ? "True" : "False");
    Serial.print("\tCreador: ");
    Serial.println(creador);
    Serial.print("\tDestinatario: ");
    Serial.println(destinatario);
    Serial.print("\tNonce: ");
    Serial.println(nonce);
    Serial.print("\tSaltos: ");
    Serial.println(saltos);
    Serial.print("\tLargo texto: ");
    Serial.println(largo_texto);
    Serial.print("\tLargo texto comprimido: ");
    Serial.println(largo_texto_comprimido);
    Serial.print("\tDirección memoria contenido: ");
    Serial.println((uintptr_t)contenido_comprimido_handler->get_elem<char>(), HEX);
    Serial.print("\tContenido: ");
    Serial.println(_contenido);
    memoria.release<char>(contenido_handler, largo_texto + 1);
}

bool Texto::operator==(const Texto& texto) const {
    return nonce == texto.nonce && creador == texto.creador && destinatario == texto.destinatario;
}

bool Texto::operator!=(const Texto& texto) const {
    return nonce != texto.nonce || creador != texto.creador || destinatario != texto.destinatario;
}

bool Texto::es_valido() {
    return valido;
}

void Texto::update(
    uint16_t _nonce, uint16_t _creador, uint16_t _destinatario,
    uint8_t _saltos, int _largo_texto,
    Memory_handler& contenido_handler, bool comprimido
) {
    nonce = _nonce;
    creador = _creador;
    destinatario = _destinatario;
    saltos = _saltos;
    if (_largo_texto < 1) return;
    if (comprimido) { //si el contenido viene o no comprimido
        if (_largo_texto > (int)max_largo_contenido_comprimido) {
            largo_texto_comprimido = largo_texto = 0;
            contenido_comprimido_handler = nullptr;
            return;
        }
        largo_texto_comprimido = _largo_texto;
        contenido_comprimido_handler = &memoria.acquire<char>(largo_texto_comprimido);
        std::memcpy(contenido_comprimido_handler->get_elem<char>(), contenido_handler.get_elem<char>(), largo_texto_comprimido);

        //descompresión
        Memory_handler& temp_descomprimido_handler = memoria.acquire<char>(4 * largo_texto_comprimido);
        largo_texto = unishox2_decompress_simple(
            contenido_handler.get_elem<char>(),
            largo_texto_comprimido,
            temp_descomprimido_handler.get_elem<char>()
        );

        Serial.println("----- Contenido durante update de Texto -----");
        Serial.println(temp_descomprimido_handler.get_elem<char>());
        Serial.println("----- Fin Contenido durante update de Texto -----");
        memoria.release<char>(temp_descomprimido_handler, 4 * largo_texto_comprimido);
    }
    else {
        //compresión
        largo_texto = _largo_texto;
        Memory_handler& temp_comprimido_handler = memoria.acquire<char>(2 * largo_texto);
        int _largo_texto_comprimido = unishox2_compress_simple(
            contenido_handler.get_elem<char>(),
            largo_texto,
            temp_comprimido_handler.get_elem<char>()
        );
        if (_largo_texto_comprimido > (int)max_largo_contenido_comprimido) {
            largo_texto_comprimido = largo_texto = 0;
            contenido_comprimido_handler = nullptr;
            memoria.release<char>(temp_comprimido_handler, 2 * largo_texto);
            return;
        }
        largo_texto_comprimido = _largo_texto_comprimido;
        contenido_comprimido_handler = &memoria.acquire<char>(largo_texto_comprimido);
        std::memcpy(
            contenido_comprimido_handler->get_elem<char>(),
            temp_comprimido_handler.get_elem<char>(),
            largo_texto_comprimido
        );
    }
    valido = true;
}

void Texto::update(const Texto* texto) {
    update(
        texto->nonce, texto->creador, texto->destinatario,
        texto->saltos, texto->largo_texto_comprimido,
        *texto->contenido_comprimido_handler, true
    );
    Serial.println("Flag Texto::update 1");
}