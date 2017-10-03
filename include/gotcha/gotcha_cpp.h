#ifndef GOTCHA_CPP_H
#define GOTCHA_CPP_H

#include <cstring>
#include <iostream>
#include <functional>
#include <tuple>
#include <type_traits>

#include<gotcha/gotcha.h>

//via: https://functionalcpp.wordpress.com/2013/08/05/function-traits/
template<class F>
struct gotcha_function_info;
 
template<class R, class... Args>
struct gotcha_function_info<R(*)(Args...)> : public gotcha_function_info<R(Args...)>
{};
 
template<class R, class... Args>
struct gotcha_function_info<R(Args...)>
{
    using return_type = R;
    using function_pointer_type = R(*)(Args...);
    using std_function_type = std::function<R(Args...)>;
    using function_wrapper_type = std::function<R(function_pointer_type,Args...)>;
    static constexpr std::size_t arity = sizeof...(Args);
 
    template <std::size_t N>
    struct argument
    {
        static_assert(N < arity, "error: invalid parameter index.");
        using type = typename std::tuple_element<N,std::tuple<Args...>>::type;
    };
};  

// member function pointer
template<class C, class R, class... Args>
struct gotcha_function_info<R(C::*)(Args...)> : public gotcha_function_info<R(C&,Args...)>
{};
 
// const member function pointer
template<class C, class R, class... Args>
struct gotcha_function_info<R(C::*)(Args...) const> : public gotcha_function_info<R(C&,Args...)>
{};
 
// member object pointer
template<class C, class R>
struct gotcha_function_info<R(C::*)> : public gotcha_function_info<R(C&)>
{};

// functor
template<class F>
struct gotcha_function_info
{
    private:
        using call_type = gotcha_function_info<decltype(&F::operator())>;
    public:
        using return_type = typename call_type::return_type;
        using function_pointer_type = typename call_type::function_pointer_type;
        using std_function_type = typename call_type::std_function_type;
 
        static constexpr std::size_t arity = call_type::arity - 1;
 
        template <std::size_t N>
        struct argument
        {
            static_assert(N < arity, "error: invalid parameter index.");
            using type = typename call_type::template argument<N+1>::type;
        };
};
 
template<class F>
struct gotcha_function_info<F&> : public gotcha_function_info<F>
{};
 
template<class F>
struct gotcha_function_info<F&&> : public gotcha_function_info<F>
{};

template<int N, class Wrap, class F>
struct runtime_wrapper;
 
template<int N, class Wrapper, class R, class... Args>
struct runtime_wrapper<N,Wrapper,R(*)(Args...)> : public runtime_wrapper<N,Wrapper, R(Args...)>
{
   using Underlying = R(*)(Args...);
   using call_traits = gotcha_function_info<Underlying>;
   using wrapper_store_type = typename call_traits::function_wrapper_type;
   static Underlying* original_call(){
      static Underlying original_call_data;
      return &original_call_data;
   } 
   static gotcha_binding_t*& bindings(){
     static gotcha_binding_t* bindings_data = new gotcha_binding_t;
     return bindings_data;
   }
   //static struct gotcha_binding_t bindings[1];
   static wrapper_store_type* instrumented_version(){
     static wrapper_store_type instrumented_version_data;
     return &instrumented_version_data;
   }
   //runtime_wrapper(const char* name, Wrapper wrapper, R(*original)(Args...)){
   runtime_wrapper(const char* name, Wrapper wrapper, R(*original)(Args...)){
     *(original_call()) = original; 
     bindings()[0].name = name;
     bindings()[0].wrapper_pointer = (void*)redirect<R>;
     bindings()[0].function_address_pointer = original_call();
     *(instrumented_version()) = wrapper;
     gotcha_wrap(bindings(),1,"nothing_good");
     //*(instrumented_version()) = ArgWrappedFunction<R(*)(Args...)>(cali::wrap_function_and_args(name, *(original_call())));
   }
   template<typename shadow_R = R>
   static auto redirect(Args... args) -> typename std::enable_if<
        !std::is_same<shadow_R, void>::value,
        shadow_R>::type {
       R ret_val = (*(instrumented_version()))(*original_call(),args...);
       return ret_val;
   }
   template<typename shadow_R = R>
   static auto redirect(Args... args) -> typename std::enable_if<
        std::is_same<shadow_R, void>::value,
        shadow_R>::type {
       (*(instrumented_version()))(*original_call(),args...);
   }
};

//via: https://functionalcpp.wordpress.com/2013/08/05/function-traits/
template<int N,class Wrapper, class R, class... Args>
struct runtime_wrapper<N,Wrapper,R(Args...)>
{};  

template<int Index, class Wrapper, class R, class... Args>
struct runtime_wrap_helper{
  using wrapper_type = runtime_wrapper<Index,Wrapper,R(*)(Args...)>;
};

template<int N, typename Callable, class R, class... Args>
auto gotcha_instrument_function(const char* name, Callable wrapper, R(*wrap_me)(Args...)) -> runtime_wrapper<N,Callable,R(*)(Args...)>*  {
  return new runtime_wrapper<N,Callable,R(*)(Args...)>(name,wrapper, wrap_me);
}
#ifndef GOTCHA_QUOTE
#define GOTCHA_QUOTE(name) #name
#define GOTCHA_STR(macro) GOTCHA_QUOTE(macro)
#endif
#define gotcha_quick_wrap(name,wrapper) \
  using original_##name = gotcha_function_info<decltype(name)>::function_pointer_type; \
  gotcha_instrument_function<__COUNTER__>(GOTCHA_STR(name),wrapper,name)
#endif
