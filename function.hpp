#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include "log.h"

namespace TTF {

template< class T >
struct remove_reference      {
    typedef T type;
};
template< class T >
struct remove_reference<T&>  {
    typedef T type;
};
template< class T >
struct remove_reference<T&&> {
    typedef T type;
};


template<typename T>
constexpr T&& forward (typename remove_reference<T>::type& t) noexcept {
    return static_cast<T&&>(t);
}

template<typename T>
constexpr T&& forward (typename remove_reference<T>::type&& t) noexcept {
    return static_cast<T&&>(t);
}


template < typename T >
class Function;

template < typename Ret, typename... Args >
class Function< Ret(Args...) > {
private:
    class FunctorBase {
    public:
        virtual Ret operator()(Args &&... a) = 0;
        virtual FunctorBase *copy() const = 0;
        virtual ~FunctorBase() {};
    };
    FunctorBase *base;

    template < typename T >
    class Functor : public FunctorBase {
    public:
        T f;
        Functor(T functor) : f(functor) {}

        Ret operator()(Args &&... a) {
            return f(forward< Args >(a)...);
        }

        FunctorBase *copy() const {
            return new Functor<T>(f);
        }
    };

public:
    template < typename T >
    Function(T functor) : base(new Functor< T >(functor)) {
        // CGRA_LOGD();
    }

    Function() : base(nullptr) {
        // CGRA_LOGD();
    }

    Ret operator()() {
        return (*base)();
    }
    Function(const Function &f) {
        base = f.base->copy();
    }
    Function &operator=(const Function &f) {
        delete base;
        base = f.base->copy();
        return *this;
    }

    ~Function() {
        // CGRA_LOGD();
        delete base;
    }
};

} // namespace TTF

#endif // FUNCTION_HPP