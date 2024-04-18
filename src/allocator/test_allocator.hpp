#ifndef ALLOCATOR_TEST_HPP
#define ALLOCATOR_TEST_HPP


template <class T>
class Allocator_test {
private:

public:
    using value_type = T;

    Allocator_test() = default;

    template <class U>
    Allocator_test(const Allocator_test<U>&) { }

    ~Allocator_test() = default;

    T* allocate(size_t n, const void* hint = nullptr) {
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    void deallocate(T* p, size_t n) {
        ::operator delete(p);
    }

    template<class U, class... Args>
    void construct(U* p, Args&& ...args) {
        new (p) T(std::forward<Args>(args)...);
    }

    template<class U>
    void destroy(U* p) {
        p->~U();
    }

    // Rebind helper for allocator_traits
    template<class U>
    struct rebind {
        using other = Allocator_test<U>;
    };
};

#endif