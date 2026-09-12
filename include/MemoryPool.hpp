#pragma once
#include <vector>
#include <cstdint>
#include <cstddef>
#include <stdexcept>

template <typename T, size_t BlockSize = 1000000>
class MemoryPool {
public:
    MemoryPool() {
        storage_.resize(BlockSize);
        free_indices_.reserve(BlockSize);
        for (size_t i = 0; i < BlockSize; ++i) {
            free_indices_.push_back(static_cast<uint32_t>(BlockSize - 1 - i));
        }
    }

    uint32_t allocate() {
        if (free_indices_.empty()) {
            throw std::bad_alloc();
        }
        uint32_t index = free_indices_.back();
        free_indices_.pop_back();
        return index;
    }

    void deallocate(uint32_t index) {
        free_indices_.push_back(index);
    }

    T& operator[](uint32_t index) {
        return storage_[index];
    }

    const T& operator[](uint32_t index) const {
        return storage_[index];
    }

    void reset() {
        free_indices_.clear();
        for (size_t i = 0; i < BlockSize; ++i) {
            free_indices_.push_back(static_cast<uint32_t>(BlockSize - 1 - i));
        }
    }

private:
    std::vector<T> storage_;
    std::vector<uint32_t> free_indices_;
};