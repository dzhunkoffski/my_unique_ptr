#pragma once
#include <type_traits>
#include <cstddef>
#include <utility>

template <class T, size_t I, bool = std::is_empty_v<T> && !std::is_final_v<T>>
struct CompressedPairElem {
    T val;

    CompressedPairElem() : val(T()) {
    }

    template <typename F>
    CompressedPairElem(F&& other) : val(std::forward<F>(other)) {
    }

    const T& Get() const {
        return val;
    }

    T& Get() {
        return val;
    }
};

template <class T, size_t I>
struct CompressedPairElem<T, I, true> : public T {

    CompressedPairElem() : T{T()} {
    }

    template <typename F>
    CompressedPairElem(F&& other) {
    }

    const T& Get() const {
        return *this;
    }

    T& Get() {
        return *this;
    }
};

template <typename F, typename S>
class CompressedPair : private CompressedPairElem<F, 0>, private CompressedPairElem<S, 1> {
public:
    CompressedPair() : First{}, Second{} {
    }

    template <typename K, typename L>
    CompressedPair(K&& first, L&& second)
            : First{std::forward<K>(first)}, Second{std::forward<L>(second)} {
    }

    F& GetFirst() {
        return First::Get();
    }
    const F& GetFirst() const {
        return First::Get();
    }

    S& GetSecond() {
        return Second::Get();
    }
    const S& GetSecond() const {
        return Second::Get();
    }

private:
    using First = CompressedPairElem<F, 0>;
    using Second = CompressedPairElem<S, 1>;
};
