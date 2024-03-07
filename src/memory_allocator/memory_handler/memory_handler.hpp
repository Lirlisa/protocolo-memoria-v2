#ifndef MEMORY_HANDLER
#define MEMORY_HANDLER

#include <cstdint>

class Memory_handler;

class Block_pointer {
private:
    Memory_handler* parent = nullptr; // Externo
    void* block_start = nullptr; // Necesario para el alineamiento. Externo
    void* data_start = nullptr; // Externo
    std::size_t block_size = 0; // El tamaño del bloque tomando en cuenta el alineamiento.
    std::size_t data_size = 0; // El tamaño sólo de los datos.
    std::size_t alineamiento = 0;
    std::size_t data_type_size = 0;
    bool congelado = false;

public:
    Block_pointer() = default;
    Block_pointer(
        Memory_handler* _parent, void* _block_start,
        void* _data_start, std::size_t _block_size,
        std::size_t _data_size, std::size_t _alineamiento,
        std::size_t _data_type_size
    );
    ~Block_pointer() = default;

    Memory_handler* get_parent() const;
    void* get_block_start() const;
    void* get_data() const;
    std::size_t get_block_size() const;
    std::size_t get_data_size() const;
    std::size_t get_alineamiento() const;
    std::size_t get_data_type_size() const;
    bool esta_congelado() const;

    void set_parent(Memory_handler* _parent);
    void set_block_start(void* ptr);
    void set_data(void* _data);
    void set_block_size(std::size_t _block_size);
    void set_data_size(std::size_t _data_size);
    void set_alineamiento(std::size_t _alineamiento);
    void set_data_type_size(std::size_t _data_type_size);
    void anular();
    void congelar();
    void descongelar();
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
    @brief Se obtiene un puntero al elemento, sin embargo, en caso de que ocurra defragmentación, el puntero se invalida.
    */
    template <typename T>
    T* get_elem() const {
        return static_cast<T*>(child->get_data());
    }

    bool es_valido();
    void hacer_valido(Block_pointer* _child);
    void hacer_invalido();
    void set_child(Block_pointer* _child);
    Block_pointer& get_child();
};



#endif