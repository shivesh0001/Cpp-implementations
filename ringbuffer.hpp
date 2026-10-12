#include <cstddef>
#include <stdexcept>
#include <utility>

template <typename T>
class ringbuffer {
private:
    T* data{};
    int capacity{};
    int head{};  // Index of the oldest element.
    int tail{};  // Index where the next element will be inserted.
    int size{};  // Number of elements currently stored.

public:
    // Allocate fixed-capacity storage.
    // The buffer starts empty; allocating storage does not construct T objects.
    explicit ringbuffer(int cap) : capacity(cap) {
        if (cap <= 0) {
            throw std::invalid_argument("Capacity must be positive");
        }

        data = static_cast<T*>(::operator new(sizeof(T) * capacity));
    }

    // Copy constructor: create independent storage and copy live elements
    // in logical FIFO order, regardless of their physical positions.
    ringbuffer(const ringbuffer& other)
        : capacity(other.capacity), size(other.size) {
        data = static_cast<T*>(::operator new(sizeof(T) * capacity));

        for (int i = 0; i < size; ++i) {
            new (data + i) T(other.data[(other.head + i) % other.capacity]);
        }

        head = 0;
        tail = size % capacity;
    }

    // Copy assignment: use copy-and-swap to replace the contents.
    ringbuffer& operator=(const ringbuffer& other) {
        if (this == &other) return *this;

        ringbuffer temp(other);
        swap(temp);
        return *this;
    }

    // Move constructor: steal the allocated storage and metadata.
    ringbuffer(ringbuffer&& other) noexcept
        : data(other.data),
          capacity(other.capacity),
          head(other.head),
          tail(other.tail),
          size(other.size) {
        other.data = nullptr;
        other.capacity = 0;
        other.head = 0;
        other.tail = 0;
        other.size = 0;
    }

    // Move assignment: release current contents, then steal the source.
    ringbuffer& operator=(ringbuffer&& other) noexcept {
        if (this == &other) return *this;

        for (int i = 0; i < size; ++i) {
            data[(head + i) % capacity].~T();
        }
        ::operator delete(data);

        data = other.data;
        capacity = other.capacity;
        head = other.head;
        tail = other.tail;
        size = other.size;

        other.data = nullptr;
        other.capacity = 0;
        other.head = 0;
        other.tail = 0;
        other.size = 0;

        return *this;
    }

    // Destroy all live elements and release the storage.
    ~ringbuffer() {
        for (int i = 0; i < size; ++i) {
            data[(head + i) % capacity].~T();
        }

        ::operator delete(data);
    }

    // Insert at tail. The modulo operation wraps the index back to zero
    // when tail reaches the end of the allocated array.
    // This implementation rejects insertion when the buffer is full.
    template <typename U>
    void push(U&& value) {
        if (size == capacity) {
            throw std::overflow_error("Ring buffer is full");
        }

        new (data + tail) T(std::forward<U>(value));

        tail = (tail + 1) % capacity;
        ++size;
    }

    // Remove the oldest element at head.
    void pop() {
        if (size == 0) {
            throw std::underflow_error("Ring buffer is empty");
        }

        data[head].~T();

        head = (head + 1) % capacity;
        --size;
    }

    // Access the oldest element without removing it.
    T& front() {
        if (size == 0) {
            throw std::underflow_error("Ring buffer is empty");
        }

        return data[head];
    }

    const T& front() const {
        if (size == 0) {
            throw std::underflow_error("Ring buffer is empty");
        }

        return data[head];
    }

    bool empty() const { return size == 0; }
    bool full() const { return size == capacity; }
    int getsize() const { return size; }
    int getcapacity() const { return capacity; }

    // Swap storage and metadata in constant time.
    void swap(ringbuffer& other) noexcept {
        std::swap(data, other.data);
        std::swap(capacity, other.capacity);
        std::swap(head, other.head);
        std::swap(tail, other.tail);
        std::swap(size, other.size);
    }
};

int main(){
    return 0;
}