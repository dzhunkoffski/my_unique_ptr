#include <iostream>
#include "unique.h"
#include "deleters.h"
#include "my_int.h"
#include <tuple>

template <typename T>
class DerivedDeleter : public Deleter<T> {};

template <typename T>
void DeleteFunction(T* ptr) {
    delete ptr;
}

template <typename T>
struct StatefulDeleter {
    int some_useless_field = 0;

    void operator()(T* ptr) {
        delete ptr;
        ++some_useless_field;
    }
};

struct VoidPtrDeleter {
    void operator()(void* ptr) {
        free(ptr);
    }
};

int main() {
    std::cout << "================ TEST 1 : CONSTRUCTION FROM COPYABLE DELETER ================" << '\n';
    {
        const CopyableDeleter<MyInt> cd;
        UniquePtr<MyInt, CopyableDeleter<MyInt>> s(new MyInt, cd);
    }
    std::cout << "++++++++++++++++ TEST 1 - PASSED ++++++++++++++++" << '\n';

    std::cout << "================ TEST 2 : CONSTRUCTION FROM MOVE-ONLY-DELETER ================" << '\n';
    {
        Deleter<MyInt> d;
        UniquePtr<MyInt, Deleter<MyInt>> s(new MyInt, std::move(d));
    }
    std::cout << "++++++++++++++++ TEST 2 - PASSED ++++++++++++++++" << '\n';

    std::cout << "================ TEST 3 : CONSTRUCTION FROM TEMPORARY ================" << '\n';
    {
        UniquePtr<MyInt, Deleter<MyInt>> s(new MyInt, Deleter<MyInt>{});
    }
    std::cout << "++++++++++++++++ TEST 3 - PASSED ++++++++++++++++" << '\n';
    
    std::cout << "================ TEST 4 : CONSTRUCTION WITH DELETER TYPE IS NON-CONST REFERENCE ================" << '\n';
    {
        Deleter<MyInt> d;
        UniquePtr<MyInt, Deleter<MyInt>&> s(new MyInt, d);
    }
    std::cout << "++++++++++++++++ TEST 4 - PASSED ++++++++++++++++" << '\n';

    std::cout << "================ TEST 5 : CONSTRUCTION WITH DELETER TYPE IS CONST REFERENCE ================" << '\n';
    {
        Deleter<MyInt> d;
        UniquePtr<MyInt, const Deleter<MyInt>&> s1(new MyInt, d);

        const Deleter<MyInt>& cr = d;
        UniquePtr<MyInt, const Deleter<MyInt>&> s2(new MyInt, cr);
    }
    std::cout << "++++++++++++++++ TEST 5 - PASSED ++++++++++++++++" << '\n';

    std::cout << "================ TEST 6 : MOVE WITH CUSTOM DELETER ================" << '\n';
    {
        UniquePtr<MyInt, Deleter<MyInt>> s1(new MyInt, Deleter<MyInt>(5));
        MyInt* p = s1.Get();
        UniquePtr<MyInt, Deleter<MyInt>> s2(new MyInt);

        assert(MyInt::AliveCount() == 2);
        assert(s1.GetDeleter().GetTag() == 5);
        assert(s2.GetDeleter().GetTag() == 0);

        s2 = std::move(s1);

        assert(s2.Get() == p);
        assert(s1.Get() == nullptr);
        assert(MyInt::AliveCount() == 1);
        assert(s2.GetDeleter().GetTag() == 5);
        assert(s1.GetDeleter().GetTag() == 0);
    }
    std::cout << "++++++++++++++++ TEST 6 - PASSED ++++++++++++++++" << '\n';

    std::cout << "================ TEST 7 : MOVE WITH REFERENCE DELETER TYPE ================" << '\n';
    {
        CopyableDeleter<MyInt> d1(5);
        UniquePtr<MyInt, CopyableDeleter<MyInt>&> s1(new MyInt, d1);
        MyInt* p1 = s1.Get();

        CopyableDeleter<MyInt> d2(6);
        UniquePtr<MyInt, CopyableDeleter<MyInt>&> s2(new MyInt, d2);

        assert(MyInt::AliveCount() == 2);

        s2 = std::move(s1);

        assert(s2.Get() == p1);
        assert(s1.Get() == nullptr);
        assert(MyInt::AliveCount() == 1);
        assert(d1.GetTag() == 5);
        assert(d2.GetTag() == 5);
    }
    std::cout << "++++++++++++++++ TEST 7 - PASSED ++++++++++++++++" << '\n';

    std::cout << "================ TEST 8 : GET DELETER ================" << '\n';
    {
        UniquePtr<MyInt, Deleter<MyInt>> p;

        assert(!p.GetDeleter().IsConst());
    }
    std::cout << "++++++++++++++++ TEST 8 - PASSED ++++++++++++++++" << '\n';

    std::cout << "================ TEST 9 : GET DELETER CONST ================" << '\n';
    {
        const UniquePtr<MyInt, Deleter<MyInt>> p;

        assert(p.GetDeleter().IsConst());
    }
    std::cout << "++++++++++++++++ TEST 9 - PASSED ++++++++++++++++" << '\n';

    std::cout << "================ TEST 10 : GET DELETER REFERENCE ================" << '\n';
    {
        using UDRef = UniquePtr<MyInt, Deleter<MyInt>&>;
        Deleter<MyInt> d;

        UDRef p(nullptr, d);
        const UDRef& cp = p;

        assert(!p.GetDeleter().IsConst());
        assert(!cp.GetDeleter().IsConst());
    }
    std::cout << "++++++++++++++++ TEST 10 - PASSED ++++++++++++++++" << '\n';

    std::cout << "================ TEST 11 : GET DELETER CONST REFERENCE ================" << '\n';
    {
        using UDConstRef = UniquePtr<MyInt, const Deleter<MyInt>&>;
        const Deleter<MyInt> d;

        UDConstRef p(nullptr, d);
        const UDConstRef& cp = p;

        assert(p.GetDeleter().IsConst());
        assert(cp.GetDeleter().IsConst());
    }
    std::cout << "++++++++++++++++ TEST 11 - PASSED ++++++++++++++++" << '\n';

    std::cout << "================ TEST 12 : UNIQUE_PTR<VOID, VOID_PTR_DELETER> CASE ================" << '\n';
    {
        UniquePtr<void, VoidPtrDeleter> p(malloc(100));
    }
    std::cout << "++++++++++++++++ TEST 12 - PASSED ++++++++++++++++" << '\n';

    std::cout << "================ TEST 13 : ARRAY SPECIALIZATION DELETE[] CHECK ================" << '\n';
    {
        UniquePtr<MyInt[]> u(new MyInt[100]);
        assert(MyInt::AliveCount() == 100);
        u.Reset();
        assert(MyInt::AliveCount() == 0);
    }
    std::cout << "++++++++++++++++ TEST 13 - PASSED ++++++++++++++++" << '\n';

    std::cout << "================ TEST 14 : ARRAY SPECIALIZATION ABLE TO USE CUSTOM DELETERS ================" << '\n';
    {
        UniquePtr<MyInt[], Deleter<MyInt[]>> u(new MyInt[100]);
        assert(MyInt::AliveCount() == 100);
        u.Reset();
        assert(MyInt::AliveCount() == 0);
    }
    std::cout << "++++++++++++++++ TEST 14 - PASSED ++++++++++++++++" << '\n';

    std::cout << "================ TEST 15 : ARRAY SPECIALIZATION OPERATOR[] ================" << '\n';
    {
        int* arr = new int[5];
        for (size_t i = 0; i < 5; ++i) {
            arr[i] = i;
        }

        UniquePtr<int[]> u(arr);
        for (int i = 0; i < 5; ++i) {
            assert(u[i] == i);
            u[i] = -i;
            assert(u[i] == -i);
        }
    }
    std::cout << "++++++++++++++++ TEST 15 - PASSED ++++++++++++++++" << '\n';

    std::cout << "================ TEST 16 : COMPRESSED PAIR - STATELESS STRUCT DELETER ================" << '\n';
    {
        static_assert(sizeof(UniquePtr<int>) == sizeof(void*));
        static_assert(sizeof(UniquePtr<int, std::default_delete<int>>) == sizeof(int*));
    }
    std::cout << "++++++++++++++++ TEST 16 - PASSED ++++++++++++++++" << '\n';

    std::cout << "================ TEST 17 : COMPRESSED PAIR STATEFUL STRUCT DELETER ================" << '\n';
    {
        static_assert(sizeof(UniquePtr<int, StatefulDeleter<int>>) ==
                      sizeof(std::pair<int*, StatefulDeleter<int>>));
    }
    std::cout << "++++++++++++++++ TEST 17 - PASSED ++++++++++++++++" << '\n';

    std::cout << "================ TEST 18 : COMPRESSED PAIR - STATELESS LAMBDA DELETER ================" << '\n';
    {
        auto lambda_deleter = [](int* ptr) { delete ptr; };
        static_assert(sizeof(UniquePtr<int, decltype(lambda_deleter)>) == sizeof(int*));
    }
    std::cout << "++++++++++++++++ TEST 18 - PASSED ++++++++++++++++" << '\n';

    std::cout << "================ TEST 19 : COMPRESSED PAIR - STATEFUL LAMBDA DELETER ================" << '\n';
    {
        int some_useless_counter = 0;
        auto lambda_deleter = [&some_useless_counter](int* ptr) {
            delete ptr;
            ++some_useless_counter;
        };
        static_assert(sizeof(UniquePtr<int, decltype(lambda_deleter)>) ==
                      sizeof(std::pair<int*, decltype(lambda_deleter)>));
    }
    std::cout << "++++++++++++++++ TEST 19 - PASSED ++++++++++++++++" << '\n';

    std::cout << "================ TEST 20 : FUNCTION POINTER DELETER ================" << '\n';
    {
        static_assert(sizeof(UniquePtr<int, decltype(&DeleteFunction<int>)>) ==
                      sizeof(std::pair<int*, decltype(&DeleteFunction<int>)>));
    }
    std::cout << "++++++++++++++++ TEST 20 - PASSED ++++++++++++++++" << '\n';

    std::cout << "================ TEST 21 : UPCAST IN MOVE CONSTRUCTOR ================" << '\n';
    {
        UniquePtr<MyInt, DerivedDeleter<MyInt>> s(new MyInt);
        UniquePtr<MyInt, Deleter<MyInt>> s2(std::move(s));
    }
    std::cout << "++++++++++++++++ TEST 21 - PASSED ++++++++++++++++" << '\n';

    std::cout << "================ TEST 22 : UPCAST IN MOVE ASSIGNMENT ================" << '\n';
    {
        UniquePtr<MyInt, DerivedDeleter<MyInt>> s(new MyInt);
        UniquePtr<MyInt, Deleter<MyInt>> s2(new MyInt);
        s2 = std::move(s);
    }
    std::cout << "++++++++++++++++ TEST 22 - PASSED ++++++++++++++++" << '\n';
}