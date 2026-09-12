#pragma once
#include <vector>
#include <atomic>
#include <cstdint>
#include <cstddef>
#include <new>

#if defined(__x86_64__) || defined(_M_X64)
#include <immintrin.h>
#define HARDWARE_PAUSE() _mm_pause()
#else
#define HARDWARE_PAUSE() ((void)0)
#endif

template <typename T, size_t Capacity = 65536>
class RingBuffer {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of two");

public:
    RingBuffer() : buffer_(Capacity) {}

    bool push(const T& item) {
        const size_t current_tail = tail_.load(std::memory_order_relaxed);
        const size_t current_head = head_.load(std::memory_order_acquire);

        if ((current_tail - current_head) >= Capacity) {
            return false;
        }

        buffer_[current_tail & BufferMask] = item;
        tail_.store(current_tail + 1, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        const size_t current_head = head_.load(std::memory_order_relaxed);
        const size_t current_tail = tail_.load(std::memory_order_release);

        if (current_head == current_tail) {
            return false;
        }

        item = buffer_[current_head & BufferMask];
        head_.store(current_head + 1, std::memory_order_release);
        return true;
    }

    bool empty() const {
        return head_.load(std::memory_order_relaxed) == tail_.load(std::memory_order_relaxed);
    }

    size_t size() const {
        const size_t head = head_.load(std::memory_order_relaxed);
        const size_t tail = tail_.load(std::memory_order_relaxed);
        return (tail >= head) ? (tail - head) : 0;
    }

private:
    static constexpr size_t BufferMask = Capacity - 1;
    static constexpr size_t HardwareDestructiveInterferenceSize = 64;

    std::vector<T> buffer_;

    alignas(HardwareDestructiveInterferenceSize) std::atomic<size_t> head_{0};
    alignas(HardwareDestructiveInterferenceSize) std::atomic<size_t> tail_{0};
};