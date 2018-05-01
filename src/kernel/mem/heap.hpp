#pragma once
#include <sys/types.h>
#include <stdint.h>
#include <opt/option.hpp>
#include <sys/paging/ptse_arc_base.hpp>
#include <mem/ptse_allocator.hpp>

namespace heap {

    /**
     * \brief Describes heap block status
     */
    enum HeapFlags: uint64_t {
        /// If set, block is allocated
        HF_ALLOC    = 1
    };

    /// Magic, which must be present in a valid block
    static constexpr HeapFlags headerMagic = HeapFlags(0x1A110C00);

    /**
     * \brief Heap block header
     */
    struct HeapHeader {
        /// Flags field, contains magic
        uint64_t flags;
        /// Size of bytes (actual) in the block
        size_t length;
        /// Pointer to previous header (or NULL)
        HeapHeader *prev;
        /// Pointer to next header (or NULL)
        HeapHeader *next;
    };

    /**
     * \brief Heap class, used for dynamic memory allocation/deallocation
     */
    class Heap {
    public:
        /**
         * \brief Initializes heap with provided params
         * \param base      - Beginning of the heap
         * \param size      - Size (in bytes) of the heap
         */
        Heap(uintptr_t base, size_t size);

        /**
         * \brief Creates uninitialized heap
         */
        Heap();

        /**
         * \brief Checks if objects on heap are freed
         */
        ~Heap();

        /**
         * \brief Allocates block of memory of requested size
         * \param size      - Number of bytes to allocate
         * \return some(addr) - if allocation succeeded,\n
         *         none()     - if allocation failed
         */
        option<void *> alloc(size_t size);

        /**
         * \brief Allocates aligned block of memory of requested size
         * \param size      - Number of bytes to allocate
         * \param align     - Boundary to which the block should be aligned (must be power of 2)
         * \return some(addr) - if allocation succeeded,\n
         *         none()     - if allocation failed
         */
        option<void *> allocAligned(size_t size, size_t align);

        /**
         * \brief Frees a block of memory
         * \param ptr       - Pointer to memory to be freed
         * May panic if ptr does not belong to the heap or the heap header is damaged
         */
        void free(void *ptr);

        /**
         * \brief Frees a block of memory and checks its size in the header equals the size provided
         * \param ptr       - Pointer to memory to be freed
         * \param size      - Size of the block
         * May panic if ptr does not belong to the heap or the heap header is damaged
         */
        void freeChecked(void *ptr, size_t sz);

        /**
         * \return Pointer to the first header in the heap
         */
        HeapHeader *rootHeader();

        /**
         * \brief Dumps list of free/used blocks
         */
        void dump();

        /**
         * \brief Reinitializes the heap with new params
         * \param base      - Beginning of the heap
         * \param size      - Size (in bytes) of the heap
         */
        void reset(uintptr_t base, size_t size);

        /**
         * \return true if heap is initialized, flase otherwise
         */
        bool valid() const;

    private:
        uintptr_t m_base;
        size_t m_size;
    };

    /// Is a kernel heap object
    extern Heap kernelHeap;

    /// Initializes kernel heap
    void init(pml4_arc_t<ptse_allocator_t> *arc);

}
