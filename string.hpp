#include <cstddef>
#include <cstring>
#include <utility>
#include <algorithm>

class String {
private:
    char* data{};
    int size{};
    int capacity{};

    // Reallocates storage while preserving the existing string.
    // capacity represents usable character positions, excluding '\0'.
    // Therefore, we allocate capacity + 1 bytes.
    void reallocate(std::size_t new_capacity) {
        char* new_data = static_cast<char*>(::operator new(new_capacity + 1));

        if (data != nullptr) {
            // Copy the existing characters and the null terminator.
            std::memcpy(new_data, data, size + 1);
        } else {
            // A default-constructed or moved-from String has no storage.
            new_data[0] = '\0';
        }

        // Release the old allocation after copying its contents.
        ::operator delete(data);

        data = new_data;
        capacity = new_capacity;
    }

public:
    // Default construction: no allocation is needed for an empty string.
    String() = default;

    // Construct from a null-terminated C-style string.
    explicit String(const char* str) {
        if (str == nullptr) return;

        size = std::strlen(str);
        capacity = size;

        // Allocate one extra byte for the null terminator.
        data = static_cast<char*>(::operator new(capacity + 1));

        // Copy the characters, including '\0'.
        std::memcpy(data, str, size + 1);
    }

    // Copy constructor: create an independent copy of the string.
    String(const String& other) : size(other.size), capacity(other.capacity) {
        if (other.data != nullptr) {
            data = static_cast<char*>(::operator new(capacity + 1));
            std::memcpy(data, other.data, size + 1);
        }
    }

    // Copy assignment: copy the contents into independently owned storage.
    String& operator=(const String& other) {
        if (this == &other) return *this;

        char* new_data = nullptr;

        if (other.data != nullptr) {
            new_data = static_cast<char*>(::operator new(other.capacity + 1));
            std::memcpy(new_data, other.data, other.size + 1);
        }

        ::operator delete(data);

        data = new_data;
        size = other.size;
        capacity = other.capacity;

        return *this;
    }

    // Move constructor: transfer ownership instead of copying characters.
    String(String&& other) noexcept : data(other.data), size(other.size), capacity(other.capacity) {
        other.data = nullptr;
        other.size = 0;
        other.capacity = 0;
    }

    // Move assignment: release the current allocation and steal the source's.
    String& operator=(String&& other) noexcept {
        if (this == &other) return *this;

        ::operator delete(data);

        data = other.data;
        size = other.size;
        capacity = other.capacity;

        other.data = nullptr;
        other.size = 0;
        other.capacity = 0;

        return *this;
    }

    // Release the owned character buffer.
    ~String() {
        ::operator delete(data);
    }

    // Accessors. 
    int getsize() const { return size; }
    int getcapacity() const { return capacity; }
    bool empty() const { return size == 0; }

    // Always provide a valid C string, even when data is nullptr.
    const char* c_str() const {
        return data != nullptr ? data : "";
    }

    // Unchecked character access, like operator[] in std::string.
    char& operator[](std::size_t index) { return data[index]; }
    const char& operator[](std::size_t index) const { return data[index]; }

    // Increase available storage without changing the string's size.
    void reserve(std::size_t new_capacity) {
        if (new_capacity > capacity) reallocate(new_capacity);
    }

    // Append one character, growing capacity geometrically when necessary.
    void push_back(char ch) {
        if (size == capacity) {
            int new_capacity = capacity == 0 ? 1 : capacity * 2;
            reserve(new_capacity);
        }

        data[size] = ch;
        ++size;
        data[size] = '\0';
    }

    // Append another String.
    String& append(const String& other) {
        // Handle self-append because the destination and source would overlap.
        String temp;
        if (this == &other) {
            temp = other;
            return append(temp);
        }

        if (other.size == 0) return *this;

        int old_size = size;
        int new_size = size + other.size;

        if (new_size > capacity) {
            int new_capacity = capacity == 0 ? 1 : capacity;

            // Double capacity until the appended contents fit.
            while (new_capacity < new_size) new_capacity *= 2;

            reserve(new_capacity);
        }

        // Copy only the appended characters, then restore the terminator.
        std::memcpy(data + old_size, other.data, other.size);
        size = new_size;
        data[size] = '\0';

        return *this;
    }

    // Append a C-style string. A temporary protects against pointers into data
    // becoming invalid if the destination needs to reallocate.
    String& append(const char* str) {
        String temp(str);
        return append(temp);
    }

    String& operator+=(const String& other) { return append(other); }
    String& operator+=(const char* str) { return append(str); }
    String& operator+=(char ch) { push_back(ch); return *this; }

    // Clear the contents while retaining allocated capacity.
    void clear() {
        size = 0;
        if (data != nullptr) data[0] = '\0';
    }

    // Exchange ownership and metadata without copying characters.
    void swap(String& other) noexcept {
        std::swap(data, other.data);
        std::swap(size, other.size);
        std::swap(capacity, other.capacity);
    }

    // Concatenate two strings into a new String.
    friend String operator+(const String& lhs, const String& rhs) {
        String result(lhs);
        result.append(rhs);
        return result;
    }

    // Compare sizes and character contents, excluding the terminator.
    friend bool operator==(const String& lhs, const String& rhs) {
        return lhs.size == rhs.size && (lhs.size == 0 || std::memcmp(lhs.c_str(), rhs.c_str(), lhs.size) == 0);
    }

    friend bool operator!=(const String& lhs, const String& rhs) {
        return !(lhs == rhs);
    }
};

int main(){
    return 0;
}