#include <mensaje/mensaje/mensaje.hpp>
#include <memory_allocator/memory_allocator.hpp>
#include <memory_allocator/memory_handler/memory_handler.hpp>
#include <cstdint>
#include <cstring>
#include <Arduino.h>
#include <algorithm>


Mensaje::Mensaje(Memory_allocator& _memoria) : memoria(_memoria), payload_handler(memoria.acquire<uint8_t>(payload_max_size)) {
    Serial.println("Flag Mensaje constructor v2");
}

Mensaje::Mensaje(
    uint32_t _ttr, uint16_t _emisor, uint16_t _receptor,
    uint16_t _nonce, uint8_t _tipo_payload, Memory_handler& payload_externo_handler,
    unsigned _payload_size, Memory_allocator& _memoria
) : memoria(_memoria), payload_handler(memoria.acquire<uint8_t>(payload_max_size)) {
    Serial.println("Flag Mensaje Constructor 1");
    ttr = _ttr;
    Serial.println("Flag Mensaje Constructor 2");
    emisor = _emisor;
    Serial.println("Flag Mensaje Constructor 3");
    receptor = _receptor;
    Serial.println("Flag Mensaje Constructor 4");
    nonce = _nonce;
    Serial.println("Flag Mensaje Constructor 5");
    tipo_payload = _tipo_payload;
    Serial.println("Flag Mensaje Constructor 6");
    payload_size = std::min(_payload_size, payload_max_size);
    Serial.println("Flag Mensaje Constructor 7");
    transmission_size = message_without_payload_size + payload_size;
    Serial.println("Flag Mensaje Constructor 8");
    if (payload_size > 0) {
        Serial.println("Flag Mensaje Constructor 9");
        std::memcpy(payload_handler.get_elem<uint8_t>(), payload_externo_handler.get_elem<uint8_t>(), payload_size);
        Serial.println("Flag Mensaje Constructor 10");
    }
    Serial.println("Flag Mensaje Constructor 11");
}

Mensaje::Mensaje(Memory_handler& mensaje_original_handler, Memory_allocator& _memoria) :
    memoria(_memoria), payload_handler(memoria.acquire<uint8_t>(payload_max_size)) {
    Mensaje* original = mensaje_original_handler.get_elem<Mensaje>();
    ttr = original->ttr;
    emisor = original->emisor;
    receptor = original->receptor;
    nonce = original->nonce;
    tipo_payload = original->tipo_payload;
    transmission_size = std::min(original->transmission_size, raw_message_max_size);
    payload_size = std::min((unsigned)(original->payload_size), payload_max_size);
    if (payload_size > 0)
        std::memcpy(payload_handler.get_elem<uint8_t>(), original->payload_handler.get_elem<uint8_t>(), payload_size);
}

Mensaje::Mensaje(Memory_handler& data_handler, uint8_t largo_data, Memory_allocator& _memoria) :
    memoria(_memoria), payload_handler(memoria.acquire<uint8_t>(payload_max_size)) {
    Serial.println("Flag Mensaje constructor v4");
    uint32_t _ttr;
    uint16_t _emisor, _receptor, _nonce;
    uint8_t _tipo_payload, * data = data_handler.get_elem<uint8_t>();

    std::memcpy(&_emisor, data, 2); // 0, 1
    std::memcpy(&_receptor, data + 2, 2); // 2, 3
    std::memcpy(&_nonce, data + 4, 2); // 4, 5

    _ttr = (data[6] & 0xff); // 8 bits
    _ttr |= ((uint32_t)data[7] & 0xff) << 8; // 16 bits
    _ttr |= ((uint32_t)data[8] & 0xf8) << 13; // 21 bits

    _tipo_payload = data[8] & 0x7;

    ttr = _ttr;
    emisor = _emisor;
    receptor = _receptor;
    nonce = _nonce;
    tipo_payload = _tipo_payload;
    payload_size = std::min(largo_data - message_without_payload_size, payload_max_size);
    transmission_size = message_without_payload_size + payload_size;
    if (payload_size > 0)
        std::memcpy(payload_handler.get_elem<uint8_t>(), data + message_without_payload_size, payload_size);
}

Mensaje::~Mensaje() {
    memoria.release<uint8_t>(payload_handler, payload_max_size);
}

bool Mensaje::operator!=(const Mensaje& other) const {
    return nonce != other.nonce || emisor != other.emisor || receptor != other.receptor;
}

bool Mensaje::operator==(const Mensaje& other) const {
    return nonce == other.nonce && emisor == other.emisor && receptor == other.receptor;
}

Mensaje& Mensaje::operator=(const Mensaje& other) {
    if (this == &other) {
        return *this;
    }
    ttr = other.ttr;
    emisor = other.emisor;
    receptor = other.receptor;
    nonce = other.nonce;
    tipo_payload = other.tipo_payload;
    payload_size = other.payload_size;
    transmission_size = other.transmission_size;
    if (payload_size > 0)
        memcpy(payload_handler.get_elem<uint8_t>(), other.payload_handler.get_elem<uint8_t>(), payload_size);

    return *this;
}

/*
@brief Imprime los primeros bytes del payload.
*/
void Mensaje::peek(unsigned cant_bytes) const {
    if (payload_size < 1) {
        Serial.println("Sin payload");
        return;
    }
    Memory_handler& repr_handler = memoria.acquire<char>(5);
    char* repr = repr_handler.get_elem<char>();
    repr[4] = '\0';
    uint8_t* payload = payload_handler.get_elem<uint8_t>();
    for (unsigned i = 0; i < cant_bytes; i++) {
        sprintf(repr, "%02x", payload[i]);
        Serial.print(repr);
    }
    memoria.release<char>(repr_handler, 5);
}

void Mensaje::print(unsigned cant_bytes) const {
    Serial.print("Emisor: ");
    Serial.println(emisor);
    Serial.print("Receptor: ");
    Serial.println(receptor);
    Serial.print("Nonce: ");
    Serial.println(nonce);
    Serial.print("TTR: ");
    Serial.println(ttr);
    Serial.print("Tipo payload: ");
    switch (tipo_payload) {
    case PAYLOAD_ACK_COMUNICACION:
        Serial.println("ACK Comunicación");
        break;
    case PAYLOAD_ACK_MENSAJE:
        Serial.println("ACK Mensaje");
        break;
    case PAYLOAD_BEACON:
        Serial.println("Beacon");
        break;
    case PAYLOAD_TEXTO:
        Serial.println("Texto");
        break;
    case PAYLOAD_VECTOR:
        Serial.println("Vector");
        break;
    case PAYLOAD_TEXTO_VISTO:
        Serial.println("Textos Vistos");
        break;
    case PAYLOAD_BEACON_CENTRAL:
        Serial.println("Beacon Central");
        break;
    default:
        Serial.println("Tipo de payload corrupto");
    }
    Serial.print("Payload size: ");
    Serial.println(payload_size);
    Serial.print("transmission size: ");
    Serial.println(transmission_size);
    uint8_t _cant_bytes = std::min((unsigned)payload_size, cant_bytes);
    if (payload_size > 0) {
        Serial.print("Payload(");
        Serial.print(_cant_bytes);
        Serial.print("): ");
        peek(_cant_bytes);
        Serial.println("");
    }
}

/**
@brief Crea mensaje para transmitir a partir del mensaje. El destino debe tener al menos 'transmission_size' bytes disponibles.
*/
void Mensaje::parse_to_transmission(Memory_handler& destino_handler) const {
    Serial.println("Flag Mensaje::parse_to_transmission 1");
    uint8_t* destino = destino_handler.get_elem<uint8_t>();
    Serial.println("Flag Mensaje::parse_to_transmission 2");
    std::memcpy(destino, &emisor, 2); // 0, 1
    Serial.println("Flag Mensaje::parse_to_transmission 3");
    std::memcpy(destino + 2, &receptor, 2); // 2, 3
    Serial.println("Flag Mensaje::parse_to_transmission 4");
    std::memcpy(destino + 4, &nonce, 2); // 4, 5
    Serial.println("Flag Mensaje::parse_to_transmission 5");
    Serial.print("TTR: ");
    Serial.println(ttr);
    uint8_t aux = ttr & 0xff;
    std::memcpy(destino + 6, &aux, 1);

    // destino[6] = (uint8_t)(ttr & 0xff); // 8 bits
    Serial.println("Flag Mensaje::parse_to_transmission 6");
    destino[7] = (uint8_t)((ttr >> 8) & 0xff); // 16 bits
    Serial.println("Flag Mensaje::parse_to_transmission 7");
    destino[8] = (uint8_t)(((ttr >> 16) & 0x1f) << 3); // 21 bits
    Serial.println("Flag Mensaje::parse_to_transmission 8");

    destino[8] |= tipo_payload & 0x7;
    Serial.println("Flag Mensaje::parse_to_transmission 9");
    Serial.println((uintptr_t)payload_handler.get_elem<uint8_t>(), HEX);
    std::memcpy(destino + message_without_payload_size, payload_handler.get_elem<uint8_t>(), payload_size);
    Serial.println("Flag Mensaje::parse_to_transmission 10");
}

void Mensaje::setEmisor(uint16_t _emisor) {
    emisor = _emisor;
}
void Mensaje::setReceptor(uint16_t _receptor) {
    receptor = _receptor;
}

void Mensaje::setNonce(uint16_t _nonce) {
    nonce = _nonce;
}

void Mensaje::setTTR(uint32_t _ttr) {
    ttr = _ttr;
}

uint16_t Mensaje::getNonce() const {
    return nonce;
}

uint16_t Mensaje::getEmisor() const {
    return emisor;
}
uint16_t Mensaje::getReceptor() const {
    return receptor;
}

uint8_t Mensaje::getTipoPayload() const {
    return tipo_payload;
}

uint32_t Mensaje::getTTR() const {
    return ttr;
}

unsigned Mensaje::get_transmission_size() const {
    return transmission_size;
}

uint8_t Mensaje::getPayloadSize() const {
    return payload_size;
}

void Mensaje::update(
    uint32_t _ttr, uint16_t _emisor, uint16_t _receptor,
    uint16_t _nonce, uint8_t _tipo_payload, Memory_handler& payload_externo_handler,
    unsigned _payload_size
) {
    Serial.println("Flag Mensaje::update 1");
    ttr = _ttr;
    emisor = _emisor;
    receptor = _receptor;
    nonce = _nonce;
    tipo_payload = _tipo_payload;
    payload_size = std::min(_payload_size, payload_max_size);
    transmission_size = message_without_payload_size + payload_size;
    if (payload_size > 0)
        std::memcpy(payload_handler.get_elem<uint8_t>(), payload_externo_handler.get_elem<uint8_t>(), payload_size);

}

void Mensaje::update(
    uint32_t _ttr, uint16_t _emisor, uint16_t _receptor,
    uint16_t _nonce
) {
    ttr = _ttr;
    emisor = _emisor;
    receptor = _receptor;
    nonce = _nonce;
}

void Mensaje::update(Memory_handler& data_handler, uint8_t largo_data) {
    uint32_t _ttr;
    uint16_t _emisor, _receptor, _nonce;
    uint8_t _tipo_payload, * data = data_handler.get_elem<uint8_t>();


    std::memcpy(&_emisor, data, 2); // 0, 1
    std::memcpy(&_receptor, data + 2, 2); // 2, 3
    std::memcpy(&_nonce, data + 4, 2); // 4, 5

    _ttr = (data[6] & 0xff); // 8 bits
    _ttr |= ((uint32_t)data[7] & 0xff) << 8; // 16 bits
    _ttr |= ((uint32_t)data[8] & 0xf8) << 13; // 21 bits

    _tipo_payload = data[8] & 0x7;

    ttr = _ttr;
    emisor = _emisor;
    receptor = _receptor;
    nonce = _nonce;
    tipo_payload = _tipo_payload;
    payload_size = std::min(largo_data - message_without_payload_size, payload_max_size);
    transmission_size = message_without_payload_size + payload_size;
    if (payload_size > 0)
        std::memcpy(payload_handler.get_elem<uint8_t>(), data + message_without_payload_size, payload_size);
}

void Mensaje::update(const Mensaje& mensaje) {
    Serial.println("Flag Mensaje::update(const Mensaje& mensaje) 1");

    update(
        mensaje.getTTR(), mensaje.getEmisor(), mensaje.getReceptor(),
        mensaje.getNonce(), mensaje.getTipoPayload(), mensaje.payload_handler,
        mensaje.getPayloadSize()
    );
    Serial.println("Flag Mensaje::update(const Mensaje& mensaje) 2");
}


