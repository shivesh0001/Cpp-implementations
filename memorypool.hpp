#pragma once

#include <iostream>
#include <algorithm>  
#include <new>        // ::operator new, ::operator delete, std::align_val_t, std::bad_alloc

template <typename T>
class memorypool {
private:
    // Points to the beginning of the entire memory block.  
    // We use char* because pointer arithmetic on char*
    // moves one byte at a time.
    char* buffer;        

    // Head of our free list.
    // It points to the first AVAILABLE slot in the linked
    // list, not necessarily the lowest-addressed free slot.
    // nullptr means that no free slots remain.  
    void* front;           

    int slot_size;         
    int slot_alignment;   

public:
    explicit memorypool(int size) {

        // A slot must satisfy the alignment requirements of both T and void*.
        // We choose the larger alignment so either can be stored in a slot.
        slot_alignment = std::max(alignof(T), alignof(void*));

        int minimum_size = std::max(sizeof(T), sizeof(void*));

        // Every slot must start at a correctly aligned address. If the slot
        // size is a multiple of the alignment, and the buffer is aligned,
        // every subsequent slot will also be aligned.
        slot_size =((minimum_size + slot_alignment - 1) / slot_alignment) * slot_alignment;

        // Allocate ONE contiguous block of raw memory for all slots.
        // ::operator new is the global allocation function.
        // The leading :: explicitly selects the global namespace.
        // size * slot_size is the total number of bytes requested.
        // std::align_val_t(slot_alignment) selects the aligned allocation
        // overload, requesting memory aligned to slot_alignment.
        // IMPORTANT: this allocates raw memory. It does NOT construct any T
        // objects, unlike an expression such as new T().
        // The returned pointer is void*, which static_cast converts to char*
        // so we can navigate through the block byte by byte.
        buffer = static_cast<char*>(::operator new(size * slot_size,std::align_val_t(slot_alignment)));

        for (int i = 0; i < size; ++i) {

            void* slot = buffer + i * slot_size;
            void* next_slot =(i < size - 1)? (buffer + (i + 1) * slot_size) : nullptr;
            *static_cast<void**>(slot) = next_slot;
        }
        front = buffer;
    }

    T* allocate() {
        if (front == nullptr) {
            throw std::bad_alloc();
        }

        void* slot = front;
        front = *static_cast<void**>(slot);

        // IMPORTANT: static_cast<T*>(slot) does NOT construct a T object.
        // It only converts the pointer type. The caller must construct T
        // in this raw storage, for example with placement new:
        //   T* obj = new (pool.allocate()) T(arguments);
        // Placement new constructs T in the supplied memory instead of
        // allocating a separate block. It does not allocate another slot.
        return static_cast<T*>(slot);
    }

    void deallocate(T* ptr) {
        void* slot = static_cast<void*>(ptr);
        *static_cast<void**>(slot) = front;
        front = slot;

        // IMPORTANT:
        // deallocate() only returns the slot to the free list.
        // It does not call T's destructor.
        // If an object was constructed in this slot, destroy it BEFORE
        // returning the slot:
        //   ptr->~T();
        //   pool.deallocate(ptr);
    }

    ~memorypool() {
        // This releases the pool's raw memory, but does NOT automatically
        // call the destructors of individual T objects constructed in it.
        ::operator delete(buffer,std::align_val_t(slot_alignment));
    }
};

int main() {
    return 0;
}

/*
===============================================================================
WHAT THIS MEMORY POOL DOES
===============================================================================

1. Allocates one large contiguous block of memory.
2. Divides it into fixed-size, correctly aligned slots.
3. Stores a next-slot pointer inside every free slot, forming a linked list.
4. allocate() removes the head and returns its storage in O(1) time.
5. deallocate() inserts a returned slot at the head in O(1) time.
6. The destructor releases the entire backing memory block.

WHY USE A MEMORY POOL?
===============================================================================

- Reuse: returned slots can be reused instead of requesting fresh memory.
- Speed: allocation and deallocation are O(1) for this free-list design.
- Reduced allocator overhead: many individual objects can share one
  backing allocation rather than each requiring a general-purpose allocation.
- Predictable slot sizes: every slot can hold the same type T.
- Locality: slots occupy one contiguous block, which can help cache behavior,
  depending on how the objects are accessed.
*/