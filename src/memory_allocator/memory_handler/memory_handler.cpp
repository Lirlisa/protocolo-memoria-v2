#include <memory_allocator/memory_handler/memory_handler.hpp>
#include <Arduino.h>
#include <cstdint>


Memory_handler::Memory_handler(Block_pointer* _child) {
    valido = true;
    child = _child;
}

bool Memory_handler::es_valido() {
    return valido;
}

void Memory_handler::hacer_valido(Block_pointer* _child) {
    valido = true;
    child = _child;
}

void Memory_handler::hacer_invalido() {
    valido = false;
    child = nullptr;
}

void Memory_handler::set_child(Block_pointer* _child) {
    child = _child;
}

Block_pointer& Memory_handler::get_child() {
    return *child;
}

void Memory_handler::congelar_bloque() {
    child->congelar();
}

void Memory_handler::descongelar_bloque() {
    child->descongelar();
}

void Memory_handler::set_vacio(bool _vacio) {
    vacio = _vacio;
}

bool Memory_handler::esta_vacio() const {
    return vacio;
}

void Memory_handler::print() const {
    Serial.print("Dirección propia: ");
    Serial.println((uintptr_t)this, HEX);
    Serial.print("Vacío?: ");
    Serial.println(vacio ? "Sí" : "No");
    Serial.print("Válido?: ");
    Serial.println(valido ? "Sí" : "No");
    Serial.print("Bloque hijo: ");
    Serial.println((uintptr_t)child, HEX);
}

/*--------------------------*/


Block_pointer::Block_pointer(
    Memory_handler* _parent, void* _block_start,
    void* _data_start, std::size_t _block_size,
    std::size_t _data_size, std::size_t _alineamiento,
    std::size_t _data_type_size
) {
    parent = _parent;
    block_start = _block_start;
    data_start = _data_start;
    block_size = _block_size;
    data_size = _data_size;
    alineamiento = _alineamiento;
    data_type_size = _data_type_size;
}

void* Block_pointer::get_block_start() const {
    return block_start;
}

void* Block_pointer::get_data() const {
    return data_start;
}

std::size_t Block_pointer::get_block_size() const {
    return block_size;
}

std::size_t Block_pointer::get_data_size() const {
    return data_size;
}

Memory_handler* Block_pointer::get_parent() const {
    return parent;
}

std::size_t Block_pointer::get_alineamiento() const {
    return alineamiento;
}

std::size_t Block_pointer::get_data_type_size() const {
    return data_type_size;
}

bool Block_pointer::esta_congelado() const {
    return congelado;
}

void Block_pointer::set_parent(Memory_handler* _parent) {
    parent = _parent;
}

void Block_pointer::set_block_start(void* ptr) {
    block_start = ptr;
}

void Block_pointer::set_data(void* _data) {
    data_start = _data;
}

void Block_pointer::set_block_size(std::size_t _block_size) {
    block_size = _block_size;
}

void Block_pointer::set_data_size(std::size_t _data_size) {
    data_size = _data_size;
}

void Block_pointer::anular() {
    parent = nullptr;
    congelado = false;
    // data_start = nullptr;
    // block_start = nullptr;
    // data_size = 0;
    // block_size = 0;
}

void Block_pointer::set_alineamiento(std::size_t _alineamiento) {
    alineamiento = _alineamiento;
}

void Block_pointer::set_data_type_size(std::size_t _data_type_size) {
    data_type_size = _data_type_size;
}

void Block_pointer::congelar() {
    congelado = true;
}

void Block_pointer::descongelar() {
    congelado = false;
}

void Block_pointer::print() const {
    Serial.print("Dirección propia: ");
    Serial.println((uintptr_t)this, HEX);
    Serial.print("Handler padre: ");
    Serial.println((uintptr_t)parent, HEX);
    Serial.print("Comienzo bloque: ");
    Serial.println((uintptr_t)block_start, HEX);
    Serial.print("Comienzo data: ");
    Serial.println((uintptr_t)data_start, HEX);
    Serial.print("Fin bloque: ");
    Serial.println((uintptr_t)(static_cast<uint8_t*>(block_start) + block_size - 1), HEX);
    Serial.print("Tamaño bloque: ");
    Serial.println(block_size);
    Serial.print("Tamaño data: ");
    Serial.println(data_size);
    Serial.print("Alineaminto: ");
    Serial.println(alineamiento);
    Serial.print("Tamaño tipo de dato: ");
    Serial.println(data_type_size);
    Serial.print("Congelado?: ");
    Serial.println(congelado ? "Sí" : "No");
}