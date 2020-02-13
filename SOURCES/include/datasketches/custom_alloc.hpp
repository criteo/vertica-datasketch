#ifndef CUSTOM_ALLOC_H
#define CUSTOM_ALLOC_H

#include <iostream>
#include <atomic>
#include <limits>
#include <cstdint>

class custom_alloc_state {
public:
    static std::atomic<int> size_used;
    static const int64_t size_max;
};

struct bad_alloc_custom : public std::bad_alloc {
    const char *what() const throw() {
        return "Failed to allocate with custom allocator (Max threshold)";
    }
};

template<typename T>
class custom_alloc {
public:
    typedef T value_type;
    typedef T *pointer;
    typedef const T *const_pointer;
    typedef T &reference;
    typedef const T &const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    template<typename U>
    struct rebind {
        typedef custom_alloc<U>
                other;
    };

    custom_alloc() = default;

    custom_alloc(const custom_alloc &) {}

    template<typename U>
    custom_alloc(const custom_alloc<U> &other) {}

    custom_alloc &operator=(const custom_alloc &) = delete;

    ~custom_alloc() = default;

    static pointer address(reference r) {
        return &r;
    }

    static const_pointer address(const_reference cr) {
        return &cr;
    }

    static size_type max_size() {
        return std::numeric_limits<size_type>::max();
    }

    bool operator==(const custom_alloc &) const {
        return true;
    }

    bool operator!=(const custom_alloc &) const {
        return false;
    }

    static pointer allocate(size_type n) {
        if (custom_alloc_state::size_used > custom_alloc_state::size_max) {
            throw new bad_alloc_custom();
        }
        custom_alloc_state::size_used += sizeof(T) * n;
        return static_cast<pointer>(operator new(sizeof(T) * n));
    }

    static pointer allocate(size_type n, pointer ptr) {
        return allocate(n);
    }

    static void deallocate(pointer ptr, size_type n) {
        if (ptr != 0) {
            custom_alloc_state::size_used -= sizeof(T) * n;
            operator delete(ptr);
        }
    }

    static void construct(pointer ptr, const value_type &t) {
        new(ptr) value_type(t);
    }

    static void destroy(pointer ptr) {
        ptr->~value_type();
    }
};

#endif  // CUSTOM_ALLOC_H
