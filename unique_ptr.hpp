#include<iostream>

template<typename T>
class unique_ptr {
private:
    T* ptr{};
public:
    //constructors
    explicit unique_ptr(T* p) : ptr(p){}
    unique_ptr()=default;

    //copy constructor
    unique_ptr(const unique_ptr& p)=delete;

    //copy assignment operator
    unique_ptr& operator=(const unique_ptr& p)=delete;

    //move constructor
    unique_ptr(unique_ptr&& p) noexcept {
        ptr=p.ptr;
        p.ptr=nullptr;
    }

    //move assignment operator
    unique_ptr& operator=(unique_ptr&& p) noexcept {
        if(this==&p){
            return *this;//self assignment prevention
        }
        //move the resource
        delete ptr;//delete the current resource
        ptr=p.ptr;
        p.ptr=nullptr;
        return *this;
    }

    //overloaded operators
    T& operator*() const noexcept{
        return *ptr;
    }
    T* operator->() const noexcept{
        return ptr;
    }
    explicit operator bool() const noexcept{
        return ptr!=nullptr;
    }

    //destructor
    ~unique_ptr() noexcept{
        delete ptr;
    }

};

class Resource{
public:
    Resource(){
        std::cout<<"Resource created"<<std::endl;
    }
    ~Resource(){
        std::cout<<"Resource destroyed"<<std::endl;
    }
};

int main(){

    return 0;
}