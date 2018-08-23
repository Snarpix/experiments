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

const std::size_t PAGE_SIZE = getpagesize();
const std::uintptr_t PAGE_MASK = ~(static_cast<uintptr_t>(PAGE_SIZE) - 1);

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

constexpr std::uint8_t nop5[] = {0x0F, 0x1F, 0x44, 0x00, 0x00};
constexpr std::uint8_t jmp_rel8 = 0xEB;
constexpr std::size_t jmp_rel8_size = sizeof(jmp_rel8) + sizeof(int8_t);
constexpr std::uint8_t jmp_rel32 = 0xE9;
constexpr std::size_t jmp_rel32_size = sizeof(jmp_rel32) + sizeof(int32_t);
constexpr std::uint8_t x86_trap = 0xCC; 
static_assert(sizeof(nop5) == jmp_rel32_size);

static 
std::size_t get_rif_size(std::uint8_t* begin, std::uint8_t* end) {
    if(end <= begin) {
        std::cout << "Invalid begin and end for region" << std::endl;
        abort();
    }
    std::size_t size = end - begin;
    if(size > PAGE_SIZE) {
        std::cout << "RIF block too big. Implement >2 page support?" << std::endl;
        abort();
    }
    return size;
}

static 
void generate_rif_jump(std::uint8_t (&jmp_buffer)[sizeof(nop5)], std::size_t size) {
    if(size <= std::numeric_limits<int8_t>::max()) {
        jmp_buffer[0] = jmp_rel8;
        int8_t jmp_offset = size - jmp_rel8_size;
        memcpy(&jmp_buffer[1], &jmp_offset, sizeof(jmp_offset));
        memset(jmp_buffer + jmp_rel8_size, x86_trap, sizeof(nop5) - jmp_rel8_size);
    } else if(size <= std::numeric_limits<int32_t>::max()) {
        jmp_buffer[0] = jmp_rel32;
        int32_t jmp_offset = size - jmp_rel32_size;
        memcpy(&jmp_buffer[1], &jmp_offset, sizeof(jmp_offset));
    } else {
        std::cout << "Region too big event for 32 bit branch?!" << std::endl;
        abort();
    }
}

void write_code(std::uint8_t* begin, std::uint8_t* end, const std::uint8_t (&buf)[sizeof(nop5)]) {
    const auto begin_page = page_align_ptr(begin);
    const auto end_page = page_align_ptr(end - 1);
    unprotect(begin_page);
    if(begin_page != end_page) {
        unprotect(end_page);
    }

    memcpy(begin, buf, sizeof(nop5));

    protect(begin_page);
    if(begin_page != end_page) {
        protect(end_page);
    }
}

void rif_disable(std::uint8_t* begin, std::uint8_t* end) {
    std::size_t size = get_rif_size(begin, end);

    std::uint8_t jmp_buffer[sizeof(nop5)];
    generate_rif_jump(jmp_buffer, size);

    if(memcmp(begin, nop5, sizeof(nop5)) != 0) {
        if(memcmp(begin, jmp_buffer, sizeof(jmp_buffer)) == 0) {
            std::cout << "Trying to disable already disabled RIF block" << std::endl;
        } else {
            std::cout << "Try to disable something that is not a RIF block" << std::endl;
            abort();
        }
    }

    write_code(begin, end, jmp_buffer);
}

void rif_enable(std::uint8_t* begin, std::uint8_t* end) {
    std::size_t size = get_rif_size(begin, end);

    std::uint8_t jmp_buffer[sizeof(nop5)];
    generate_rif_jump(jmp_buffer, size);

    if(memcmp(begin, jmp_buffer, sizeof(jmp_buffer)) != 0) {
        if(memcmp(begin, nop5, sizeof(nop5)) == 0) {
            std::cout << "Trying to enable already enable RIF block" << std::endl;
        } else {
            std::cout << "Try to enable something that is not a RIF block" << std::endl;
            abort();
        }
    }

    write_code(begin, end, nop5);
}

#define RUNTIME_IF_BLOCK(func, code) \
    asm volatile ( \
        "_" #func "_rif_start:\n\t" \
        ".byte 0x0F, 0x1F, 0x44, 0x00, 0x00" \
        ); \
    { \
        code \
    } \
    asm volatile ("_" #func "_rif_end:");

#define RUNTIME_IF_DISABLE(func) \
    do { \
        extern std::uint8_t func ## _rif_start; \
        extern std::uint8_t func ## _rif_end; \
        rif_disable(&func ## _rif_start, &func ## _rif_end); \
    } while(0)

#define RUNTIME_IF_ENABLE(func) \
    do { \
        extern std::uint8_t func ## _rif_start; \
        extern std::uint8_t func ## _rif_end; \
        rif_enable(&func ## _rif_start, &func ## _rif_end); \
    } while(0)

// USAGE:

void debug1() {
    std::cout << "DEBUG: 1111!!!" << std::endl;
}

void debug2() {
    std::cout << "DEBUG: 2222!!!" << std::endl;
}

// This is must-have. WILL produce compile error
[[gnu::noinline]]
void foo() noexcept {
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
    std::cout << "Enable 2!" << std::endl;
    RUNTIME_IF_ENABLE(debug2_rif);
    std::cout << "Test Debug!" << std::endl;
    foo();
    std::cout << "Enable 3!" << std::endl;
    RUNTIME_IF_ENABLE(debug3_rif);
    std::cout << "Test Debug!" << std::endl;
    foo();
    std::cout << "Enable 1!" << std::endl;
    RUNTIME_IF_ENABLE(debug1_rif);
    std::cout << "Test Debug!" << std::endl;
    foo();
    return 0;
}
