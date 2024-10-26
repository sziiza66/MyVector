#pragma once

#include <cstddef>
#include <stdexcept>    // out_of_range
#include <type_traits>  // is_nothrow_move_constructible_v
#include <utility>      // forward
#include "vector.h"

template <typename T>
Vector<T>::Vector() : ptr_(operator new(kTSZ, kAl)), capacity_(1), size_(0) {
}

template <typename T>
Vector<T>::Vector(size_t size)
    : ptr_(operator new(kTSZ * size, kAl)), capacity_(size), size_(size) {
    size_t i = 0;
    try {
        for (; i != size_; ++i) {
            new (Index(ptr_, i)) T();
        }
    } catch (...) {
        Delete(ptr_, i);
        throw;
    }
}

template <typename T>
Vector<T>::Vector(size_t size, const T& obj)
    : ptr_(operator new(kTSZ * size, kAl)), capacity_(size), size_(size) {
    size_t i = 0;
    try {
        for (; i != size_; ++i) {
            new (Index(ptr_, i)) T(obj);
        }
    } catch (...) {
        Delete(ptr_, i);
        throw;
    }
}

template <typename T>
Vector<T>::Vector(const Vector& other)
    : ptr_(operator new(other.size_ * kTSZ, kAl)), capacity_(other.size_), size_(other.size_) {
    size_t i = 0;
    try {
        for (; i != size_; ++i) {
            new (Index(ptr_, i)) T(other[i]);
        }
    } catch (...) {
        Delete(ptr_, i);
        throw;
    }
}

template <typename T>
Vector<T>::Vector(Vector&& other) noexcept : ptr_(nullptr), capacity_(0), size_(0) {
    Swap(other);
}

template <typename T>
Vector<T>::Vector(std::initializer_list<T> lst)
    : ptr_(operator new(lst.size() * kTSZ, kAl)), capacity_(lst.size()), size_(lst.size()) {
    auto it = lst.begin();
    size_t i = 0;
    try {
        for (; i != size_; ++i) {
            if constexpr (std::is_move_constructible_v<T>) {
                new (Index(ptr_, i)) T(std::move(*it));
            } else {
                new (Index(ptr_, i)) T(*it);
            }
            ++it;
        }
    } catch (...) {
        Delete(ptr_, i);
        throw;
    }
}

template <typename T>
Vector<T>::~Vector() {
    if (ptr_) {
        Delete(ptr_, size_);
    }
}

template <typename T>
Vector<T>& Vector<T>::operator=(const Vector& other) {
    if (this == &other) {
        return *this;
    }

    void* nptr = operator new(other.size_ * kTSZ, kAl);
    size_t i = 0;
    try {
        for (; i != other.size_; ++i) {
            new (Index(nptr, i)) T(other[i]);
        }
    } catch (...) {
        Delete(nptr, i);
        throw;
    }
    Delete(ptr_, size_);
    ptr_ = nptr;
    size_ = other.size_;
    capacity_ = size_;
    return *this;
}

template <typename T>
Vector<T>& Vector<T>::operator=(Vector&& other) {
    if (this == &other) {
        return *this;
    }
    Delete(ptr_, size_);
    ptr_ = nullptr;
    capacity_ = 0;
    size_ = 0;
    Swap(other);
    return *this;
}

template <typename T>
inline void Vector<T>::Swap(Vector& other) noexcept {
    std::swap(ptr_, other.ptr_);
    std::swap(capacity_, other.capacity_);
    std::swap(size_, other.size_);
}

template <typename T>
size_t Vector<T>::Size() const {
    return size_;
}

template <typename T>
size_t Vector<T>::Capacity() const {
    return capacity_;
}

template <typename T>
bool Vector<T>::Empty() const {
    return size_ == 0;
}

template <typename T>
T* Vector<T>::begin() {
    return Index(ptr_, 0);
}

template <typename T>
T* Vector<T>::end() {
    return Index(ptr_, size_);
}

template <typename T>
const T* Vector<T>::begin() const {
    return Index(ptr_, 0);
}

template <typename T>
const T* Vector<T>::end() const {
    return Index(ptr_, size_);
}

template <typename T>
T& Vector<T>::operator[](size_t ind) {
    return *Index(ptr_, ind);
}

template <typename T>
const T& Vector<T>::operator[](size_t ind) const {
    return *Index(ptr_, ind);
}

template <typename T>
T& Vector<T>::At(size_t ind) {
    if (ind >= size_) {
        throw std::out_of_range("Index out of range.");
    }
    return *Index(ptr_, ind);
}

template <typename T>
const T& Vector<T>::At(size_t ind) const {
    if (ind >= size_) {
        throw std::out_of_range("Index out of range.");
    }
    return *Index(ptr_, ind);
}

template <typename T>
T& Vector<T>::Back() {
    return *Index(ptr_, size_ - 1);
}

template <typename T>
const T& Vector<T>::Back() const {
    return *Index(ptr_, size_ - 1);
}

template <typename T>
void Vector<T>::PushBack(const T& obj) {
    if (capacity_ == size_) {
        Realloc();
    }
    new (Index(ptr_, size_)) T(obj);
    ++size_;
}

template <typename T>
void Vector<T>::PopBack() {
    Index(ptr_, size_--)->~T();
}

template <typename T>
template <typename... Args>
void Vector<T>::EmplaceBack(Args&&... args) {
    if (capacity_ == size_) {
        Realloc();
    }
    new (Index(ptr_, size_)) T(std::forward<Args>(args)...);
    ++size_;
}

template <typename T>
void Vector<T>::Clear() {
    Delete(ptr_, size_);
    ptr_ = operator new(kTSZ, kAl);
    capacity_ = 1;
    size_ = 0;
}

template <typename T>
void Vector<T>::Assign(size_t size, const T& obj) {
    void* nptr = operator new(size * kTSZ, kAl);
    size_t i = 0;
    try {
        for (; i != size; ++i) {
            new (Index(nptr, i)) T(obj);
        }
    } catch (...) {
        Delete(nptr, i);
        throw;
    }
    Delete(ptr_, size_);
    ptr_ = nptr;
    size_ = size;
    capacity_ = size;
}

template <typename T>
void Vector<T>::Realloc() {
    void* nptr = operator new(capacity_ * kTSZ * 2, kAl);
    if constexpr (std::is_nothrow_move_constructible_v<T>) {
        for (size_t i = 0; i != size_; ++i) {
            new (Index(nptr, i)) T(std::move(*Index(ptr_, i)));
        }
        capacity_ *= 2;
        Delete(ptr_, size_);
        ptr_ = nptr;
    } else {
        size_t i = 0;
        try {
            for (; i != size_; ++i) {
                new (Index(nptr, i)) T(*Index(ptr_, i));
            }
        } catch (...) {
            Delete(nptr, i);
            throw;
        }
        capacity_ *= 2;
        Delete(ptr_, size_);
        ptr_ = nptr;
    }
}

template <typename T>
inline void Vector<T>::Delete(void* ptr, size_t size) {
    T* p = static_cast<T*>(ptr);
    for (size_t i = 0; i != size; ++i, ++p) {
        p->~T();
    }
    operator delete(ptr, kAl);
}

template <typename T>
bool operator==(const Vector<T>& lhs, const Vector<T>& rhs) {
    if (lhs.Size() != rhs.Size()) {
        return false;
    }
    bool retval = true;
    for (size_t i = 0; i < lhs.Size() && retval; ++i) {
        retval = retval && (lhs[i] == rhs[i]);
    }
    return retval;
}

template <typename T>
inline T* Vector<T>::Index(void* ptr, size_t ind) const {
    return static_cast<T*>(ptr) + ind;
}

template <typename T>
bool operator!=(const Vector<T>& lhs, const Vector<T>& rhs) {
    return !(lhs == rhs);
}
