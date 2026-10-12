#include <unordered_map>
#include <list>
#include <utility>
#include <stdexcept>

template <typename K, typename V>
class lru_cache {
private:
    int capacity{};

    // Front contains the most recently used entry.
    // Back contains the least recently used entry.
    std::list<std::pair<K, V>> cache;

    // Maps each key to its corresponding node in the list.
    // This allows us to access and move entries in O(1) time.
    std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> mp;

public:
    explicit lru_cache(int cap) : capacity(cap) {
        if (cap <= 0) {
            throw std::invalid_argument("Capacity must be positive");
        }
    }

    // Retrieve a value and mark its entry as most recently used.
    V& get(const K& key) {
        auto it = mp.find(key);

        if (it == mp.end()) {
            throw std::out_of_range("Key not found");
        }

        // Move the existing node to the front without recreating it.
        cache.splice(cache.begin(), cache, it->second);

        return it->second->second;
    }

    // Insert a new entry or update an existing one.
    void put(const K& key, const V& value) {
        auto it = mp.find(key);

        if (it != mp.end()) {
            // Update the value and mark the entry as most recently used.
            it->second->second = value;
            cache.splice(cache.begin(), cache, it->second);
            return;
        }

        // Evict the least recently used entry when the cache is full.
        if (cache.size() == capacity) {
            auto last = cache.back();
            mp.erase(last.first);
            cache.pop_back();
        }

        // Insert the new entry at the front.
        cache.emplace_front(key, value);
        mp[key] = cache.begin();
    }

    bool contains(const K& key) const {
        return mp.find(key) != mp.end();
    }

    int getsize() const {
        return cache.size();
    }

    int getcapacity() const {
        return capacity;
    }

    bool empty() const {
        return cache.empty();
    }
};

int main(){
    return 0;
}