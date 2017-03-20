template<typename in_param>
struct callable_rewritable{
   static in_param translate(void* in){
     return ((in_param)(in));
   }
};

template<typename in_param>
decltype(callable_rewritable<in_param>::translate(0x0)) translate(in_param in, void* translate_this){
   return callable_rewritable<in_param>::translate(translate_this); 
}

int main(){}
