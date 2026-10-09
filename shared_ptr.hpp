#include<iostream>

template<typename T>
class shared_ptr {
private:
    struct control_block{
        T* ptr{};
        int count{};
        control_block(T* p):ptr(p),count(1){}
    };
    control_block* cb{};
public:
    //constructors
    explicit shared_ptr(T* p) {
        //dyanamically allocate control block beacuse its lifetime can extend 
        //beyond the lifetime of the shared_ptr object that originally created it
        cb=new control_block(p);
    }
    shared_ptr()=default;

    //copy constructor
    shared_ptr(const shared_ptr& p){
        //increase the reference count
        if(p.cb){
            p.cb->count++;
        }
        cb=p.cb;
    }

    //copy assignment operator
    shared_ptr& operator=(const shared_ptr& p){
        if(this==&p){
            return *this;//self assignment prevention
        }
        //decrease the reference count of the current object
        if(cb){
            cb->count--;
            if(cb->count==0){
                delete cb->ptr;
                delete cb;
            }
        }
        //increase the reference count of the new object
        if(p.cb){
            p.cb->count++;
        }
        cb=p.cb;
        return *this;
    }

    //move constructor
    shared_ptr(shared_ptr&& p) noexcept {
        cb=p.cb;
        p.cb=nullptr;
    }

    //move assignment operator
    shared_ptr& operator=(shared_ptr&& p) noexcept {
        if(this==&p){
            return *this;//self assignment prevention
        }
        //delete the current resource
        if(cb){
            cb->count--;
            if(cb->count==0){
                delete cb->ptr;
                delete cb;
            }
        }
        cb=p.cb;
        p.cb=nullptr;
        return *this;
    }

    //overloaded operators
    T& operator*() const noexcept{
        return *cb->ptr;
    }
    T* operator->() const noexcept{
        return cb->ptr;
    }
    explicit operator bool() const noexcept{
        return cb!=nullptr && cb->ptr!=nullptr;
    }

    //destructor
    ~shared_ptr() noexcept{
        if(cb){
            cb->count--;
            if(cb->count==0){
                delete cb->ptr;
                delete cb;
            }
        }
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