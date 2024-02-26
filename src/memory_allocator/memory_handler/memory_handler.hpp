#ifndef MEMORY_HANDLER
#define MEMORY_HANDLER

#include <cstdint>

class Memory_handler;

class Block_pointer {
private:
    Memory_handler* parent = nullptr; //externo
    void* data = nullptr; //externo
    std::size_t block_size = 0;
public:
    Block_pointer();
    Block_pointer(Memory_handler* _parent, void* _data, std::size_t _block_size);
    ~Block_pointer();

    void* get_data() const;
    std::size_t get_block_size() const;
    Memory_handler* get_parent() const;

    void set_parent(Memory_handler* _parent);
    void set_data(void* _data);
    void set_block_size(std::size_t _block_size);
    void anular();
};

class Memory_handler {
private:
    bool valido = false;
    Block_pointer* child = nullptr; // externo
public:
    Memory_handler() = default;
    Memory_handler(Block_pointer* _child);
    ~Memory_handler() = default;

    /*
    @brief Se obtiene una referencia al elemento, sin embargo, en caso de que ocurra defragmentación, la referencia se hace inválida.
    */
    template <typename T>
    T& get_elem() {
        return *static_cast<T*>(child->get_data());
    }

    bool es_valido();
    void hacer_valido(Block_pointer* _child);
    void hacer_invalido();
    void set_child(Block_pointer* _child);
    Block_pointer& get_child();
};



#endif