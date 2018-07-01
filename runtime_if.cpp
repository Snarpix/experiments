// Copyright 2018 Stanislav Shmarov <snarpix@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject
// to the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <cstddef>
#include <cstring>
#include <limits>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>

static const std::size_t PAGE_SIZE = getpagesize();
static const std::uintptr_t PAGE_MASK = ~(static_cast<uintptr_t>(PAGE_SIZE) - 1);

static
void* page_align_ptr(void* ptr) {
    auto page_aligned = reinterpret_cast<uintptr_t>(ptr);
    page_aligned &= PAGE_MASK;
    return reinterpret_cast<void*>(page_aligned);
}

static
void unprotect(void* ptr) {
    mprotect(ptr, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC);
}

static
void protect(void* ptr) {
    mprotect(ptr, PAGE_SIZE, PROT_READ | PROT_EXEC);
}

static constexpr std::uint8_t nops[9][9] = {
    {0x90},
    {0x66, 0x90},
    {0x0F, 0x1F, 0x00},
    {0x0F, 0x1F, 0x40, 0x00},
    {0x0F, 0x1F, 0x44, 0x00, 0x00},
    {0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00},
    {0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00},
    {0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00},
};
static constexpr std::size_t nop_max_size = sizeof(nops)/sizeof(nops[0]);

static constexpr std::uint8_t jmp_rel8 = 0xEB;
static constexpr std::uint8_t jmp_rel8_size = sizeof(jmp_rel8) + sizeof(int8_t);
static constexpr std::uint8_t jmp_rel32 = 0xE9;
static constexpr std::uint8_t jmp_rel32_size = sizeof(jmp_rel32) + sizeof(int32_t);
static constexpr std::size_t jmp_optimization_threshold = 16;

void nopify(std::uint8_t* begin, std::uint8_t* end) {
    if(end <= begin) {
        std::cout << "Invalid size of nops" << std::endl;
        abort();
    }
    std::size_t size = end - begin;
    if(size > PAGE_SIZE) {
        std::cout << "Nop region too big. Implement >2 page support?" << std::endl;
        abort();
    }

    auto begin_page = page_align_ptr(begin);
    auto end_page = page_align_ptr(end - 1);
    unprotect(begin_page);
    if(begin_page != end_page) {
        unprotect(end_page);
    }

    if(size >= jmp_optimization_threshold) {
        if(size <= std::numeric_limits<int8_t>::max()) {
            *begin++ = jmp_rel8;
            int8_t jmp_offset = size - jmp_rel8_size;
            memcpy(begin, &jmp_offset, sizeof(jmp_offset));
            size -= jmp_rel8_size;
            begin += sizeof(jmp_offset);
        } else if(size <= std::numeric_limits<int32_t>::max()) {
            *begin++ = jmp_rel32;
            int32_t jmp_offset = size - jmp_rel32_size;
            memcpy(begin, &jmp_offset, sizeof(jmp_offset));
            size -= jmp_rel32_size;
            begin += sizeof(jmp_offset);
        }
    }

    while(size) {
        std::size_t nop_size = size >= nop_max_size ? nop_max_size : size;
        memcpy(begin, nops[nop_size - 1], nop_size);
        size -= nop_size;
        begin += nop_size;
    }

    protect(begin_page);
    if(begin_page != end_page) {
        protect(end_page);
    }
}

#define RUNTIME_IF_BLOCK(func, code) \
    { \
        asm volatile ("_" #func "_rif_start:"); \
        code \
        asm volatile ("_" #func "_rif_end:"); \
    }

#define RUNTIME_IF_DISABLE(func) \
    do { \
        extern std::uint8_t func ## _rif_start; \
        extern std::uint8_t func ## _rif_end; \
        nopify(&func ## _rif_start, &func ## _rif_end); \
    } while(0)

// USAGE:

// Both should be noinline!
void debug1() {
    std::cout << "DEBUG: 1111!!!" << std::endl;
}

void debug2() {
    std::cout << "DEBUG: 2222!!!" << std::endl;
}

// This is must-have. WILL produce compile error
[[gnu::noinline]]
void foo() {
    RUNTIME_IF_BLOCK(debug1_rif, \
        debug1(); \
    )
    RUNTIME_IF_BLOCK(debug2_rif, \
        debug2(); \
        debug2(); \
    )
    RUNTIME_IF_BLOCK(debug3_rif, \
        debug1(); \
        debug2(); \
        debug1(); \
        debug2(); \
    )
}

int main() {
    std::cout << "Test Debug!" << std::endl;
    foo();
    std::cout << "Disable 1!" << std::endl;
    RUNTIME_IF_DISABLE(debug1_rif);
    std::cout << "Test Debug!" << std::endl;
    foo();
    std::cout << "Disable 2!" << std::endl;
    RUNTIME_IF_DISABLE(debug2_rif);
    std::cout << "Test Debug!" << std::endl;
    foo();
    std::cout << "Disable 3!" << std::endl;
    RUNTIME_IF_DISABLE(debug3_rif);
    std::cout << "Test Debug!" << std::endl;
    foo();
    return 0;
}
