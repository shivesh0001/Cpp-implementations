
#include <cstddef>
#include <new>       // ::operator new, ::operator delete, placement new
#include <utility>   // std::move

template <typename T>
class Vector {

private:
    // Points to the beginning of the allocated memory block.
    T* data{};  

    int capacity{};  
    int size{};     

    void reallocate(int new_capacity) {

        // Allocate a new contiguous block of raw storage.
        // ::operator new allocates memory but does not construct any T objects.
        // sizeof(T) * new_capacity gives the number of bytes required.
        // We use raw allocation rather than new T[new_capacity] because
        // new T[] would construct every element, including unused capacity.
        // A vector should construct elements only when they are inserted.
        T* new_data = static_cast<T*>(::operator new(new_capacity * sizeof(T)));

        // Transfer the existing live elements into the new block.
        for (int i = 0; i < size; i++) {

            // Placement new constructs a new T at new_data + i.
            // std::move(data[i]) casts the source expression to an rvalue,
            // allowing T's move constructor to be selected when available.
            // std::move itself does not move anything. The move constructor
            // determines how the resources are transferred.
            // Placement new uses existing storage; it does not allocate
            // another block of memory.
            new (new_data + i) T(std::move(data[i]));

            // Moving does NOT destroy the original object.
            // The old object is still alive, possibly in a moved-from state.
            // Its destructor must run before we release its storage.
            // For a class that owns resources, this cleans up anything
            // the moved-from object still owns. For int, it has no effect.
            data[i].~T();
        }

        // We do not use delete[] because we did not allocate with new[].
        ::operator delete(data);
        data = new_data;
        capacity = new_capacity;
    }

public:
    Vector() = default;


    explicit Vector(int cap) : capacity(cap) {

        // Allocate raw storage for cap elements.
        // size remains zero because no T objects have been constructed yet.
        data = static_cast<T*>(::operator new(capacity * sizeof(T)));
    }


    // Copy constructor: create a separate vector containing copies.
    // This is a DEEP COPY of the vector's elements and its storage.
    Vector(const Vector& other)
        : capacity(other.capacity), size(other.size) {

        data = static_cast<T*>(::operator new(capacity * sizeof(T)));

        // We use other.data[i] as an lvalue, so T's copy constructor
        // is selected rather than explicitly requesting a move.
        // Placement new is necessary because data[i] does not yet
        // refer to a constructed T object.
        for (int i = 0; i < size; i++) {
            new (data + i) T(other.data[i]);
        }
    }


    // Copy assignment operator.
    // Unlike the copy constructor, the destination already exists and may
    // already own storage and contain live elements.
    Vector& operator=(const Vector& other) {

        if (this == &other) {
            return *this;
        }

        // Destroy every currently live element.
        for (int i = 0; i < size; i++) {
            data[i].~T();
        }

        // Release the destination's old allocation.
        ::operator delete(data);

        capacity = other.capacity;
        size = other.size;

        // Allocate storage owned independently by the destination.
        data = static_cast<T*>(::operator new(capacity * sizeof(T)));

        // Copy-construct each live element into the new storage.
        for (int i = 0; i < size; i++) {
            new (data + i) T(other.data[i]);
        }
        // Return the destination so chained assignments work:
        return *this;
    }


    // Move constructor.
    // A move transfers ownership of the allocation instead of allocating
    // a new block and moving every element individually.
    // noexcept communicates that this ownership transfer does not throw.
    Vector(Vector&& other) noexcept : data(other.data),capacity(other.capacity),size(other.size) {
        other.data = nullptr;
        other.capacity = 0;
        other.size = 0;
    }


    // Move assignment operator.
    // The destination may already own a different allocation, so we must
    // release its current resources before taking ownership from the source.
    Vector& operator=(Vector&& other) noexcept {

        if (this == &other) {
            return *this;
        }
        for (int i = 0; i < size; i++) {
            data[i].~T();
        }
        ::operator delete(data);

        data = other.data;
        capacity = other.capacity;
        size = other.size;

        other.data = nullptr;
        other.capacity = 0;
        other.size = 0;

        return *this;
    }

    void push_back(const T& value) {

        if (size == capacity) {
            int new_capacity = capacity == 0 ? 1 : capacity * 2;
            reallocate(new_capacity);
        }
        new (data + size) T(value);
        size++;
    }

    void reserve(int new_capacity) {
        if (new_capacity > capacity) {
            reallocate(new_capacity);
        }
    }


    void pop_back() {
        if (size > 0) {
            size--;
            data[size].~T();
        }
    }
    T& operator[](int index) {
        return data[index];
    }
    int getsize() const {
        return size;
    }
    int getcapacity() const {
        return capacity;
    }


    // Destructor.
    // First destroy every live object, because releasing raw storage does
    // not automatically invoke the destructors of objects constructed there.
    ~Vector() {

        for (int i = 0; i < size; i++) {
            data[i].~T();
        }

        // Release the backing allocation after all live objects are destroyed.
        // Calling operator delete with nullptr is valid.
        ::operator delete(data);
    }
};

int main() {
    return 0;
}

/*
===============================================================================
DESIGN SUMMARY
===============================================================================

1. SIZE VS CAPACITY
   size is the number of live T objects.
   capacity is the number of T-sized slots in the allocation.

   Allocated storage is not the same as constructed objects.
   This is why we use raw operator new and placement new.

2. OBJECT LIFETIMES
   push_back() constructs an object.
   reallocate() move-constructs objects in new storage and destroys the old
   objects.
   pop_back() destroys the last object.
   The destructor destroys all remaining live objects.

   Calling a destructor ends an object's lifetime; it does not release the
   storage containing that object.

3. COPY VS MOVE
   Copying creates independent storage and copy-constructs the elements.
   Moving transfers ownership of the existing allocation.

   The copy operations implement deep-copy semantics.
   The move operations reset the source to an empty state to avoid double
   ownership of the same allocation.

4. WHY CONTIGUOUS STORAGE?
   - O(1) indexed element access.
   - Good cache locality during sequential traversal.
   - Low per-element storage overhead.
   - Amortized O(1) push_back() with geometric growth.

5. INVALIDATION
   Reallocation moves the elements to another memory block. Pointers,
   references, and iterators into the old block become invalid.

6. LIMITATIONS OF THIS LEARNING IMPLEMENTATION
   - No bounds checking in operator[].
   - No automatic growth policy beyond doubling capacity.
   - No shrink_to_fit(), iterators, or const operator[].
   - No emplace_back() or variadic perfect forwarding.
   - No support for over-aligned T with the current ordinary allocation.
   - Copy assignment and reallocation do not provide strong exception safety.
   - The push_back(const T&) implementation has an aliasing limitation:
     if value refers to an element in this same vector and insertion triggers
     reallocation, that reference may become invalid before the copy occurs.
   - Capacity is stored as int, so negative capacities and integer overflow
     are not handled.
   - The implementation assumes T is suitable for the operations requested:
     copying requires a copy-constructible T, and reallocation uses moves.

7. COMPLEXITY
   operator[]: O(1)
   getsize(): O(1)
   getcapacity(): O(1)
   pop_back(): O(1)
   reserve(): O(n) if reallocation is needed, otherwise O(1)
   push_back(): amortized O(1), O(n) when reallocation is needed
   Copy construction/assignment: O(n)
   Move construction/assignment: O(1), excluding destruction of the
   destination's previous elements in move assignment.

===============================================================================
*/
