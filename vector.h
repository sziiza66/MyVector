#pragma once

// Постарался написать exception-save вектор, могу разобраться, как сделать нормальные итераторы, если этого недостаточно.
// При условии, что ~T() исключений не кидает.

#include <cstddef>  // size_t
#include <initializer_list>
#include <new>      // operator new

template <typename T>
class Vector {
public:
    Vector();
    explicit Vector(size_t);
    explicit Vector(size_t, const T&);
    Vector(const Vector&);
    Vector(Vector&&) noexcept;
    Vector(std::initializer_list<T>);
    ~Vector();
    Vector& operator=(const Vector&);
    Vector& operator=(Vector&&);

    inline void Swap(Vector&) noexcept;

    size_t Size() const;
    size_t Capacity() const;
    bool Empty() const;

    T* begin();                 // NOLINT
    T* end();                   // NOLINT
    const T* begin() const;     // NOLINT
    const T* end() const;       // NOLINT

    T& operator[](size_t);
    const T& operator[](size_t) const;
    T& At(size_t);
    const T& At(size_t) const;
    T& Back();
    const T& Back() const;

    void PushBack(const T&);
    void PopBack();
    template <typename... Args>
    void EmplaceBack(Args&&...);

    void Clear();
    void Assign(size_t, const T&);

private:
    void Realloc();
    inline void Delete(void*, size_t);
    inline T* Index(void*, size_t) const;

private:
    void* ptr_;
    size_t capacity_;
    size_t size_;

    static constexpr size_t kTSZ = sizeof(T);
    static constexpr std::align_val_t kAl = std::align_val_t{alignof(T)};
};

template <typename T>
bool operator==(const Vector<T>&, const Vector<T>&);
template <typename T>
bool operator!=(const Vector<T>&, const Vector<T>&);

#include "vector.impl.h"
