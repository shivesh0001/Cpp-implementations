#pragma once
#include <cassert>
#include <list>
#include <vector>
#include <functional>
#include <stdexcept>
#include <utility>

template<typename keytype, typename valuetype>
class hashmap {
private:
    // Each index of this vector represents a bucket.
    // A bucket is a linked list because multiple keys can hash to
    // the same index (collision handling using separate chaining).
    // vector provides O(1) bucket access, while list allows multiple
    // key-value pairs to be stored in each bucket.
    std::vector<std::list<std::pair<keytype, valuetype>>> table;

    // Tracks the number of DISTINCT keys stored in the HashMap.
    // Incremented only when a new key is inserted.
    int msize{};

    // Number of buckets currently available.
    int capacity{};

    // Maximum average number of entries per bucket before resizing.
    // We chose 0.75 to keep collisions reasonably controlled.
    float maxloadfactor{0.75};

    // Converts a key into a bucket index.
    // std::hash generates a hash value; modulo capacity maps it
    // into the valid index range [0, capacity - 1].
    int hashfunction(const keytype& key) const {
        return std::hash<keytype>{}(key) % table.size();
    }

    // Load factor measures how full the HashMap is:
    // number of entries / number of buckets.
    // If it exceeds our chosen threshold, we increase capacity
    // and redistribute the existing entries.
    void checkloadfactor() {
        float loadfactor = static_cast<float>(msize) / table.size();

        if (loadfactor > maxloadfactor) {
            rehash();
        }
    }

    // Doubles the number of buckets and redistributes all entries.
    // The bucket index can change because hashfunction uses the
    // number of buckets. Therefore, existing entries must be
    // inserted into their correct buckets in the new table.
    void rehash() {
        int newCapacity = capacity * 2;

        std::vector<std::list<std::pair<keytype, valuetype>>>newtable(newCapacity);

        for (const auto& bucket : table) {
            for (const auto& [k, v] : bucket) {
                int index = std::hash<keytype>{}(k) % newtable.size();
                newtable[index].emplace_back(k, v);
            }
        }

        // std::move avoids copying the entire new table here.
        table = std::move(newtable);
        capacity = newCapacity;
    }

public:
    explicit hashmap(int size = 10) : capacity(size) {
        assert(size > 0);
        table.resize(size);
    }

    void insert(const keytype& key, const valuetype& value) {
        int index = hashfunction(key);

        for (auto& [k, v] : table[index]) {
            if (k == key) {
                v = value;
                return;  // Size does not change for an update.
            }
        }

        table[index].emplace_back(key, value);
        msize++;

        // Check whether the new entry requires a larger table.
        checkloadfactor();
    }

    valuetype get(const keytype& key) const {
        int index = hashfunction(key);

        for (const auto& [k, v] : table[index]) {
            if (k == key) {
                return v;
            }
        }

        throw std::runtime_error("Key not found");
    }

    bool containskey(const keytype& key) const {
        int index = hashfunction(key);

        for (const auto& [k, v] : table[index]) {
            if (k == key) {
                return true;
            }
        }

        return false;
    }

    // Removes the entry associated with the key.
    void erase(const keytype& key) {
        int index = hashfunction(key);

        auto& bucket = table[index];

        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->first == key) {
                // list::erase automatically updates neighboring links
                // and deallocates the erased node.
                bucket.erase(it);

                msize--;
                return;  // Do not use the erased iterator again.
            }
        }

        throw std::runtime_error("Key not found");
    }

    // O(1): we maintain the count instead of traversing every bucket.
    int getsize() const {
        return msize;
    }

    // O(1): an empty map has zero entries.
    bool isempty() const {
        return msize == 0;
    }
};

int main() {
    return 0;
}