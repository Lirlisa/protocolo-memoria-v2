#include <mensaje/mensaje/mensaje.hpp>
#include <mensaje/mensaje_ack_comunicacion/mensaje_ack_comunicacion.hpp>
#include <cstdint>
#include <cstring>
#include <Arduino.h>

Mensaje_ack_comunicacion::Mensaje_ack_comunicacion() : Mensaje() {
    tipo_payload = Mensaje::PAYLOAD_ACK_COMUNICACION;
}

Mensaje_ack_comunicacion::Mensaje_ack_comunicacion(
    uint32_t _ttr, uint16_t _emisor, uint16_t _receptor, uint16_t _nonce, uint16_t _nonce_msj_original
) : Mensaje(
    _ttr, _emisor, _receptor, _nonce, Mensaje::PAYLOAD_ACK_COMUNICACION
) {
    nonce_mensaje_original = _nonce_msj_original;
    payload_size = payload_max_size;
    transmission_size = message_without_payload_size + payload_size;
    uint8_t* payload = payload_handler.get_elem<uint8_t>();
    std::memcpy(payload, &nonce_mensaje_original, 2);
    std::memcpy(payload + 2, &receptor, 2); //emisor mensaje original
    std::memcpy(payload + 4, &emisor, 2); //receptor mensaje original
}


Mensaje_ack_comunicacion::Mensaje_ack_comunicacion(
    from_msg_ack_com, Memory_handler& handler_msg_ack_com
) : Mensaje_ack_comunicacion(
    handler_msg_ack_com.get_elem<Mensaje_ack_comunicacion>()->getTTR(),
    handler_msg_ack_com.get_elem<Mensaje_ack_comunicacion>()->getEmisor(),
    handler_msg_ack_com.get_elem<Mensaje_ack_comunicacion>()->getReceptor(),
    handler_msg_ack_com.get_elem<Mensaje_ack_comunicacion>()->getNonce(),
    handler_msg_ack_com.get_elem<Mensaje_ack_comunicacion>()->get_nonce_original()
) { }

/*
@brief Crea un mensaje ack comunicación a partir de un mensaje base. Es importante asegurarse que el payload sea el correcto
*/
Mensaje_ack_comunicacion::Mensaje_ack_comunicacion(from_msg, Memory_handler& handler_msg_origen) : Mensaje(handler_msg_origen) {
    std::memcpy(&nonce_mensaje_original, payload_handler.get_elem<uint8_t>(), 2);
    tipo_payload = Mensaje::PAYLOAD_ACK_COMUNICACION;
}

Mensaje_ack_comunicacion::~Mensaje_ack_comunicacion() { }

bool Mensaje_ack_comunicacion::confirmar_ack(uint16_t nonce_original, uint16_t emisor_original, uint16_t receptor_original) const {
    uint16_t aux_emisor, aux_receptor;
    memcpy(&aux_emisor, payload_handler.get_elem<uint8_t>() + sizeof(uint16_t), sizeof(uint16_t));
    memcpy(&aux_receptor, payload_handler.get_elem<uint8_t>() + 2 * sizeof(uint16_t), sizeof(uint16_t));
    // Serial.println("----- Datos en confirmar_ack -----");
    // Serial.print("Emisor en payload: ");
    // Serial.println(aux_emisor);
    // Serial.print("Emisor entregado: ");
    // Serial.println(emisor_original);
    // Serial.print("Receptor en payload: ");
    // Serial.println(aux_receptor);
    // Serial.print("Receptor entregado: ");
    // Serial.println(receptor_original);
    // Serial.print("Nonce en payload: ");
    // Serial.println(nonce_mensaje_original);
    // Serial.print("Nonce entregado: ");
    // Serial.println(nonce_original);
    // Serial.print("Resultado: ");
    // Serial.println((nonce_mensaje_original == nonce_original &&
    //     aux_emisor == emisor_original &&
    //     (aux_receptor == receptor_original || receptor_original == BROADCAST_CHANNEL_ID || aux_receptor == BROADCAST_CHANNEL_ID)) ? "True" : "False");
    // Serial.println("----- Fin Datos en confirmar_ack -----");
    return nonce_mensaje_original == nonce_original &&
        aux_emisor == emisor_original &&
        (aux_receptor == receptor_original || receptor_original == BROADCAST_CHANNEL_ID || aux_receptor == BROADCAST_CHANNEL_ID);
}

void Mensaje_ack_comunicacion::print() {
    Mensaje::print();

    Serial.print("Emisor original: ");
    Serial.println(receptor);
    Serial.print("Nonce original: ");
    Serial.println(nonce_mensaje_original);
}

void Mensaje_ack_comunicacion::update(
    uint32_t _ttr, uint16_t _emisor, uint16_t _receptor,
    uint16_t _nonce, uint16_t _nonce_mensaje_original
) {
    ttr = _ttr;
    emisor = _emisor;
    receptor = _receptor;
    nonce = _nonce;
    nonce_mensaje_original = _nonce_mensaje_original;
    payload_size = payload_max_size;
    transmission_size = message_without_payload_size + payload_size;

    std::memcpy(payload_handler.get_elem<uint8_t>(), &nonce_mensaje_original, sizeof(uint16_t));
    std::memcpy(payload_handler.get_elem<uint8_t>() + sizeof(uint16_t), &receptor, sizeof(uint16_t)); //emisor mensaje original
    std::memcpy(payload_handler.get_elem<uint8_t>() + 2 * sizeof(uint16_t), &emisor, sizeof(uint16_t)); //receptor mensaje original
}

void Mensaje_ack_comunicacion::update(const Mensaje& mensaje) {
    uint16_t _nonce;
    std::memcpy(&_nonce, mensaje.payload_handler.get_elem<uint8_t>(), 2);
    update(
        mensaje.getTTR(), mensaje.getEmisor(), mensaje.getReceptor(),
        mensaje.getNonce(), _nonce
    );
}

uint16_t Mensaje_ack_comunicacion::get_nonce_original() {
    return nonce_mensaje_original;
}