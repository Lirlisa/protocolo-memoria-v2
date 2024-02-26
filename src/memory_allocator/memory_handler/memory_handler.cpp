#include <memory_allocator/memory_handler/memory_handler.hpp>
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

/*--------------------------*/

Block_pointer::Block_pointer() { }

Block_pointer::Block_pointer(Memory_handler* _parent, void* _data, std::size_t _block_size) {
    parent = _parent;
    data = _data;
    block_size = _block_size;
}

Block_pointer::~Block_pointer() { }

void* Block_pointer::get_data() const {
    return data;
}

std::size_t Block_pointer::get_block_size() const {
    return block_size;
}

Memory_handler* Block_pointer::get_parent() const {
    return parent;
}

void Block_pointer::set_parent(Memory_handler* _parent) {
    parent = _parent;
}

void Block_pointer::set_data(void* _data) {
    data = _data;
}

void Block_pointer::set_block_size(std::size_t _block_size) {
    block_size = _block_size;
}

void Block_pointer::anular() {
    parent = nullptr;
    data = nullptr;
    block_size = 0;
}