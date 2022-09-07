#pragma once

#include <algorithm>
#include <assert.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>

namespace winrt::blurt::audio::implementation {

// A class for buffering audio samples as they're received, then consuming
// them (first in, first out). For latency reasons, once created an
// AudioBuffer instance does no heap allocation. The caller is responsible
// for locking access from different threads.
//
// Throughout, this class deals in total samples, not samples per channel.
template <typename SampleT = float, typename SizeT = std::int32_t>
class AudioBuffer {
   public:
    using size_type = typename SizeT;

    // Creates a buffer with a capacity to hold a given number of samples
    AudioBuffer(size_type samples_capacity)
        : read_idx_{0},
          write_idx_{0},
          buffer_{new SampleT[samples_capacity]},
          size_{samples_capacity} {
        assert(samples_capacity > 0);
    }

    // Get the remaining capacity (in samples) the buffer can hold. If this
    // method returns N, then a subsequent call to GetWriteDest() with an
    // argument greater than N has undefined behavior.
    size_type WriteCapacity() const { return size_ - write_idx_ + read_idx_; }

    // Get a non-owned pointer where the caller can write N samples, using
    // up N samples of capacity in the buffer. It's the caller's
    // responsibility to call WriteCapacity() first to ensure enough
    // capacity; if N is above capacity, behavior is undefined.
    SampleT* GetWriteDest(size_type num_samples) {
        assert(write_idx_ >= read_idx_);
        assert(WriteCapacity() >= num_samples);
        if (write_idx_ + num_samples > size_) {
            // If write_idx_ + num_samples would overflow, we make room
            assert(read_idx_ > 0);
            if (read_idx_ < size_) {
                // Not all the buffer has been consumed; move what remains
                // read_idx_ samples to the left
                std::memmove(&buffer_[0], &buffer_[read_idx_],
                             (write_idx_ - read_idx_) * sizeof(SampleT));
                write_idx_ -= read_idx_;
            } else {
                // Otherwise, the buffer has been completely read and both
                // index are at the right-hand end; shortcut both indexes
                // back to zero
                assert(read_idx_ == write_idx_ && write_idx_ == size_);
                write_idx_ = 0;
            }
            read_idx_ = 0;
        }
        SampleT* result = &buffer_[write_idx_];
        write_idx_ += num_samples;
        return result;
    }

    // Copy samples into the buffer from the given pointer. If the number of
    // samples to be copied is above capacity, behavior is undefined.
    void WriteSamplesFrom(const SampleT* src, size_type num_samples) {
        SampleT* dest = GetWriteDest(num_samples);
        std::memcpy(dest, src, num_samples * sizeof(SampleT));
    }

    // Move the write pointer back by N samples. This is useful during
    // encoding when the number of units that will be written isn't known ex
    // ante. For example, you might make sure the buffer has at least 2000
    // bytes of room for encoding but only 300 end up being used; then you
    // would call DecrementWritePointer(1700) after the encoding step.
    void DecrementWritePointer(size_type num_samples) {
        assert(write_idx_ >= num_samples);
        write_idx_ -= num_samples;
    }

    // Get the remaining capacity (in samples) available to read.
    size_type ReadCapacity() const {
        assert(write_idx_ >= read_idx_);
        return write_idx_ - read_idx_;
    }

    // Read samples from the buffer into the given pointer, up to the given
    // number. Returns the (possibly zero) number of samples actually read.
    size_type ReadSamplesTo(SampleT* dest, size_type num_samples) {
        assert(write_idx_ >= read_idx_);
        auto samples_to_copy = std::min(write_idx_ - read_idx_, num_samples);
        if (samples_to_copy > 0) {
            std::memcpy(dest, &buffer_[read_idx_], samples_to_copy * sizeof(SampleT));
            read_idx_ += samples_to_copy;
        }
        return samples_to_copy;
    }

    // Get a non-owned pointer from which the caller can consume N samples,
    // using up N samples of capacity in the buffer. After this method
    // returns, the buffer increments its read pointer so those same samples
    // can't be consumed again. It's the caller's responsibility to call
    // ReadCapacity() first to ensure enough availability; if N is too large,
    // behavior is undefined. It's also undefined behavior if another write to
    // the buffer takes place before the caller finishes reading the samples.
    const SampleT* GetReadSourceFor(size_type num_samples) {
        assert(write_idx_ >= read_idx_);
        assert(ReadCapacity() >= num_samples);
        const SampleT* result = &buffer_[read_idx_];
        read_idx_ += num_samples;
        return result;
    }

   private:
    std::unique_ptr<SampleT[]> buffer_;
    size_type size_;
    size_type read_idx_, write_idx_;
};

}  // namespace winrt::blurt::audio::implementation
