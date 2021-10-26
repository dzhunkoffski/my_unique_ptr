#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t
#include <memory>

template <typename T>
struct DefaultDeleter {
    void operator()(T* ptr) const noexcept {
        static_assert(sizeof(T) > 0);
        static_assert(!std::is_void<T>::value);
        delete ptr;
    }
};

template <typename T>
struct DefaultDeleter<T[]> {
    void operator()(T* ptr) const {
        static_assert(sizeof(T) > 0);
        static_assert(!std::is_void<T>::value);
        delete[] ptr;
    }
};

// Primary template
template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
private:
    CompressedPair<T*, Deleter> compressed_pair_;

    template <typename Tb, typename Del>
    friend class UniquePtr;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : compressed_pair_(ptr, Deleter()) {
    }

    template <typename D>
    UniquePtr(T* ptr, D&& deleter) : compressed_pair_(ptr, std::forward<D>(deleter)) {
    }

    template <typename Tb, typename Del>
    UniquePtr(UniquePtr<Tb, Del>&& other) noexcept
            : compressed_pair_(other.Release(), std::forward<Del>(other.GetDeleter())) {
    }

    UniquePtr(const UniquePtr&) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    //    UniquePtr& operator=(UniquePtr&& other) = default;

    template <typename Lol, typename Kek>
    UniquePtr& operator=(UniquePtr<Lol, Kek>&& other) noexcept {
        Reset(other.Release());
        GetDeleter() = std::forward<Kek>(other.GetDeleter());

        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) noexcept {
        Reset();
        return *this;
    }

    UniquePtr& operator=(const UniquePtr&) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        GetDeleter()(compressed_pair_.GetFirst());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* released = compressed_pair_.GetFirst();
        compressed_pair_.GetFirst() = nullptr;

        return released;
    }
    void Reset(T* ptr = nullptr) {
        T* old_ptr = compressed_pair_.GetFirst();
        compressed_pair_.GetFirst() = ptr;
        if (old_ptr) {
            GetDeleter()(old_ptr);
        }
    }
    void Swap(UniquePtr& other) {
        T* tmp = compressed_pair_.GetFirst();
        compressed_pair_.GetFirst() = other.compressed_pair_.GetFirst();
        other.compressed_pair_.GetFirst() = tmp;

        auto temp = compressed_pair_.GetSecond();
        compressed_pair_.GetSecond() = other.compressed_pair_.GetSecond();
        other.compressed_pair_.GetSecond() = temp;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const noexcept {
        return compressed_pair_.GetFirst();
    }
    Deleter& GetDeleter() noexcept {
        return compressed_pair_.GetSecond();
    }
    const Deleter& GetDeleter() const noexcept {
        return compressed_pair_.GetSecond();
    }
    explicit operator bool() const {
        return compressed_pair_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *compressed_pair_.GetFirst();
    }
    T* operator->() const {
        return compressed_pair_.GetFirst();
    }
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
private:
    CompressedPair<T*, Deleter> compressed_pair_;

public:
    explicit UniquePtr(T* ptr = nullptr) : compressed_pair_(ptr, Deleter()) {
    }

    void Reset(T* ptr = nullptr) {
        T* old_ptr = compressed_pair_.GetFirst();
        compressed_pair_.GetFirst() = ptr;
        if (old_ptr) {
            GetDeleter()(old_ptr);
        }
    }

    Deleter& GetDeleter() noexcept {
        return compressed_pair_.GetSecond();
    }
    const Deleter& GetDeleter() const noexcept {
        return compressed_pair_.GetSecond();
    }

    T* Get() const noexcept {
        return compressed_pair_.GetFirst();
    }

    std::add_lvalue_reference_t<T> operator[](size_t ix) const {
        return Get()[ix];
    }

    ~UniquePtr() {
        GetDeleter()(compressed_pair_.GetFirst());
    }
};
