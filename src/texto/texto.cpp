#include "texto.hpp"
#include <Arduino.h>


Texto::Texto(
    uint16_t _nonce, uint16_t _creador, uint16_t _destinatario,
    uint8_t _saltos, int _largo_texto,
    Memory_handler& contenido_handler, bool comprimido
) {
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
        if (!contenido_comprimido_handler->es_valido()) {
            valido = false;
            contenido_comprimido_handler = nullptr;
            return;
        }
        std::memcpy(contenido_comprimido_handler->get_elem<char>(), contenido_handler.get_elem<char>(), largo_texto_comprimido);

        //descompresión
        Memory_handler& temp_descomprimido_handler = memoria.acquire<char>(3 * largo_texto_comprimido);
        if (!temp_descomprimido_handler.es_valido()) {
            valido = false;
            contenido_comprimido_handler = nullptr;
            return;
        }
        largo_texto = unishox2_decompress_simple(
            contenido_handler.get_elem<char>(),
            largo_texto_comprimido,
            temp_descomprimido_handler.get_elem<char>()
        );
        memoria.release<char>(temp_descomprimido_handler);
    }
    else {
        //compresión
        Serial.println("Flag Texto constructor 1");
        largo_texto = _largo_texto;
        Serial.print("1 Memoria disponible en creación de texto: ");
        Serial.println(memoria.available_memory());
        Memory_handler& temp_comprimido_handler = memoria.acquire<char>(2 * largo_texto);
        Serial.print("1 Memoria disponible en creación de texto: ");
        Serial.println(memoria.available_memory());
        Serial.println("Flag Texto constructor 2");
        if (!temp_comprimido_handler.es_valido()) {
            Serial.println("Flag Texto constructor 3");
            valido = false;
            Serial.println("Flag Texto constructor 4");
            contenido_comprimido_handler = nullptr;
            Serial.println("Flag Texto constructor 5");
            return;
        }
        Serial.println("Flag Texto constructor 6");
        int _largo_texto_comprimido = unishox2_compress_simple(
            contenido_handler.get_elem<char>(),
            largo_texto,
            temp_comprimido_handler.get_elem<char>()
        );
        Serial.println("Flag Texto constructor 7");
        if (_largo_texto_comprimido > (int)max_largo_contenido_comprimido) {
            Serial.println("Flag Texto constructor 8");
            largo_texto_comprimido = largo_texto = 0;
            Serial.println("Flag Texto constructor 9");
            contenido_comprimido_handler = nullptr;
            Serial.println("Flag Texto constructor 10");
            memoria.release<char>(temp_comprimido_handler);
            Serial.println("Flag Texto constructor 11");
            return;
        }
        Serial.println("Flag Texto constructor 12");
        largo_texto_comprimido = _largo_texto_comprimido;
        Serial.println("Flag Texto constructor 13");
        Serial.print("1 Memoria disponible en creación de texto: ");
        Serial.println(memoria.available_memory());
        contenido_comprimido_handler = &memoria.acquire<char>(largo_texto_comprimido);
        Serial.print("1 Memoria disponible en creación de texto: ");
        Serial.println(memoria.available_memory());
        Serial.println("Flag Texto constructor 14");
        if (!contenido_comprimido_handler->es_valido()) {
            Serial.println("Flag Texto constructor 15");
            valido = false;
            Serial.println("Flag Texto constructor 16");
            contenido_comprimido_handler = nullptr;
            Serial.println("Flag Texto constructor 17");
            return;
        }
        Serial.println("Flag Texto constructor 18");
        std::memcpy(
            contenido_comprimido_handler->get_elem<char>(),
            temp_comprimido_handler.get_elem<char>(),
            largo_texto_comprimido
        );
        Serial.println("Flag Texto constructor 19");
        memoria.release<char>(temp_comprimido_handler);
        Serial.println("Flag Texto constructor 20");
    }
    Serial.println("Flag Texto constructor 21");
    valido = true;
    Serial.println("Flag Texto constructor 22");
}

Texto::Texto(const Texto& other) {
    if (contenido_comprimido_handler != nullptr)
        memoria.release<char>(*contenido_comprimido_handler);
    nonce = other.nonce;
    creador = other.creador;
    destinatario = other.destinatario;
    saltos = other.saltos;
    valido = other.valido;
    largo_texto = other.largo_texto;
    largo_texto_comprimido = other.largo_texto_comprimido;

    if (valido && largo_texto_comprimido > 0) {
        contenido_comprimido_handler = &memoria.acquire<char>(largo_texto_comprimido);
        if (!contenido_comprimido_handler->es_valido()) {
            valido = false;
            contenido_comprimido_handler = nullptr;
            return;
        }
        std::memcpy(
            contenido_comprimido_handler->get_elem<char>(),
            other.contenido_comprimido_handler->get_elem<char>(),
            largo_texto_comprimido
        );
    }
}

Texto::~Texto() {
    if (contenido_comprimido_handler != nullptr)
        memoria.release<char>(*contenido_comprimido_handler);
    valido = false;
    largo_texto = 0;
    largo_texto_comprimido = 0;
}

Texto& Texto::operator=(const Texto& other) {
    if (this == &other)
        return *this;

    if (contenido_comprimido_handler != nullptr)
        memoria.release<char>(*contenido_comprimido_handler);
    nonce = other.nonce;
    creador = other.creador;
    destinatario = other.destinatario;
    saltos = other.saltos;
    valido = other.valido;
    largo_texto = other.largo_texto;
    largo_texto_comprimido = other.largo_texto_comprimido;

    if (valido && largo_texto_comprimido > 0) {
        contenido_comprimido_handler = &memoria.acquire<char>(largo_texto_comprimido);
        if (!contenido_comprimido_handler->es_valido()) {
            valido = false;
            contenido_comprimido_handler = nullptr;
            return *this;
        }
        std::memcpy(
            contenido_comprimido_handler->get_elem<char>(),
            other.contenido_comprimido_handler->get_elem<char>(),
            largo_texto_comprimido
        );
    }
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
    Serial.println("Flag Texto::print 1");
    Serial.print("Memoria disponible: ");
    Serial.println(memoria.available_memory());
    Memory_handler& contenido_handler = memoria.acquire<char>(largo_texto + 1);
    Serial.print("Handler válido? ");
    Serial.println(contenido_handler.es_valido() ? "Sí" : "No");
    if (!contenido_handler.es_valido()) {
        Serial.println("No hay memoria disponible para imprimir");
        return;
    }
    Serial.println("Flag Texto::print 2");
    char* _contenido = contenido_handler.get_elem<char>();
    Serial.println((uintptr_t)_contenido, HEX);
    Serial.println("Flag Texto::print 3");
    Serial.println((uintptr_t)this, HEX);
    Serial.println((uintptr_t)contenido_comprimido_handler, HEX);
    Serial.println(contenido_comprimido_handler->es_valido() ? "válido" : "inválido");
    Serial.println(largo_texto);
    unishox2_decompress_simple(contenido_comprimido_handler->get_elem<char>(), largo_texto_comprimido, _contenido);
    Serial.println("Flag Texto::print 4");
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
    memoria.release<char>(contenido_handler);
    Serial.println("Flag Texto::print 5");
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
    Serial.println("Flag Texto::update 1");
    nonce = _nonce;
    creador = _creador;
    destinatario = _destinatario;
    saltos = _saltos;
    Serial.println("Flag Texto::update 2");
    if (_largo_texto < 1) return;
    Serial.println("Flag Texto::update 3");
    if (comprimido) { //si el contenido viene o no comprimido
        Serial.println("Flag Texto::update 4");
        if (_largo_texto > (int)max_largo_contenido_comprimido) {
            Serial.println("Flag Texto::update 5");
            largo_texto_comprimido = largo_texto = 0;
            contenido_comprimido_handler = nullptr;
            return;
        }
        Serial.println("Flag Texto::update 6");
        largo_texto_comprimido = _largo_texto;
        contenido_comprimido_handler = &memoria.acquire<char>(largo_texto_comprimido);
        if (!contenido_comprimido_handler->es_valido()) {
            Serial.println("Flag Texto::update 7");
            valido = false;
            contenido_comprimido_handler = nullptr;
            return;
        }
        Serial.println("Flag Texto::update 8");
        std::memcpy(contenido_comprimido_handler->get_elem<char>(), contenido_handler.get_elem<char>(), largo_texto_comprimido);

        //descompresión
        Memory_handler& temp_descomprimido_handler = memoria.acquire<char>(3 * largo_texto_comprimido);
        Serial.print("Bloque válido? ");
        Serial.println(temp_descomprimido_handler.es_valido() ? "Sí" : "No");
        if (!temp_descomprimido_handler.es_valido()) {
            Serial.println("Flag Texto::update 9");
            valido = false;
            contenido_comprimido_handler = nullptr;
            return;
        }
        Serial.println("Flag Texto::update 10");
        largo_texto = unishox2_decompress_simple(
            contenido_handler.get_elem<char>(),
            largo_texto_comprimido,
            temp_descomprimido_handler.get_elem<char>()
        );
        Serial.println("Flag Texto::update 11");
        Serial.print("Memoria disponible en update de texto: ");
        Serial.println(memoria.available_memory());
        memoria.release<char>(temp_descomprimido_handler);
        Serial.println("Flag Texto::update 12");
    }
    else {
        Serial.println("Flag Texto::update 13");
        //compresión
        largo_texto = _largo_texto;
        Serial.println("Flag Texto::update 14");
        Memory_handler& temp_comprimido_handler = memoria.acquire<char>(2 * largo_texto);
        Serial.println("Flag Texto::update 15");
        if (!temp_comprimido_handler.es_valido()) {
            Serial.println("Flag Texto::update 16");
            valido = false;
            contenido_comprimido_handler = nullptr;
            return;
        }
        Serial.println("Flag Texto::update 17");
        int _largo_texto_comprimido = unishox2_compress_simple(
            contenido_handler.get_elem<char>(),
            largo_texto,
            temp_comprimido_handler.get_elem<char>()
        );
        Serial.println("Flag Texto::update 18");
        if (_largo_texto_comprimido > (int)max_largo_contenido_comprimido) {
            Serial.println("Flag Texto::update 19");
            largo_texto_comprimido = largo_texto = 0;
            contenido_comprimido_handler = nullptr;
            memoria.release<char>(temp_comprimido_handler);
            return;
        }
        Serial.println("Flag Texto::update 20");
        largo_texto_comprimido = _largo_texto_comprimido;
        contenido_comprimido_handler = &memoria.acquire<char>(largo_texto_comprimido);
        if (!contenido_comprimido_handler->es_valido()) {
            Serial.println("Flag Texto::update 21");
            valido = false;
            contenido_comprimido_handler = nullptr;
            return;
        }
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
    Serial.println("Flag Texto::update 2 1");
}