#ifndef REGISTER_TYPES_H
#define REGISTER_TYPES_H

#include <stdint.h>
#include <cstddef>
#include <cstdint>

union axilite_reg_status_t {
	uint32_t value;
	struct {
		uint32_t
		last_seq_nr			: 16,
		last_count			: 6,
		max_frame_size		: 6,
		invalid_frame_size	: 1,
		frame_too_short		: 1,
		frame_too_long		: 1,
		is_running			: 1;
	};
};


/*
 * Access functions for status register to handle offset in register
 * representation because of size constraints.
*/

static inline void
setStatusMaxFrameSize(volatile axilite_reg_status_t& reg, uint32_t value)
{ reg.max_frame_size = value - 1; }

static inline void
setStatusLastCount(volatile axilite_reg_status_t& reg, uint32_t value)
{ reg.last_count = value - 1; }

static inline uint32_t
getStatusLastCount(const volatile axilite_reg_status_t& reg)
{ return reg.last_count + 1; }

static inline uint32_t
getStatusMaxFrameSize(const volatile axilite_reg_status_t& reg)
{ return reg.max_frame_size + 1; }


union reg_doorbell_t {
	uint32_t value;
	struct {
		uint32_t
		seq_nr			: 16,
		count			: 6,
		is_valid		: 1;
	};

	constexpr reg_doorbell_t() : value(0) {}
};



/*
 * Access functions for doorbell register to handle offset in register
 * representation because of size constraints.
*/


static inline void
setDoorbellCount(volatile reg_doorbell_t& reg, uint32_t value)
{ reg.count = value - 1; }

static inline uint32_t
getDoorbellCount(const volatile reg_doorbell_t& reg)
{ return reg.count + 1; }



template<size_t N, typename T = uint32_t>
struct Rtds2GpuMemoryBuffer {
	// this type is only for memory interpretation, it makes no sense to create
	// an instance so it's forbidden
	Rtds2GpuMemoryBuffer() = delete;

	// T can be a more complex type that wraps multiple values
	static constexpr size_t rawValueCount = N * (sizeof(T) / 4);

	// As of C++14, offsetof() is not working for non-standard layout types (i.e.
	// composed of non-POD members). This might work in C++17 though.
	// More info: https://gist.github.com/graphitemaster/494f21190bb2c63c5516
	//static constexpr size_t doorbellOffset = offsetof(Rtds2GpuMemoryBuffer, doorbell);
	//static constexpr size_t dataOffset = offsetof(Rtds2GpuMemoryBuffer, data);

	// HACK: This might break horribly, let's just hope C++17 will be there soon
	static constexpr size_t dataOffset = 0;
	static constexpr size_t doorbellOffset = sizeof(Rtds2GpuMemoryBuffer::data);

	T data[N];
	reg_doorbell_t doorbell;
};

#endif // REGISTER_TYPES_H
