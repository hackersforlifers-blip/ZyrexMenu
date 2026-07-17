#pragma once
#include <windows.h>
#include <TlHelp32.h>
#include <vector>
#include <string>
#include <memory>
#include <cstddef>
#include <atomic>

extern "C" intptr_t
Luck_ReadVirtualMemory
(
    HANDLE ProcessHandle,
    PVOID BaseAddress,
    PVOID Buffer,
    ULONG NumberOfBytesToRead,
    PULONG NumberOfBytesRead
);

extern "C" intptr_t
Luck_WriteVirtualMemory
(
    HANDLE Processhandle,
    PVOID BaseAddress,
    PVOID Buffer,
    ULONG NumberOfBytesToWrite,
    PULONG NumberOfBytesWritten
);

class memory_t final
{
public:
    memory_t() = default;
    ~memory_t() = default;

    std::uint32_t find_process_id(const std::string& process_name);
    std::uint64_t find_module_address(const std::string& module_name);

    bool attach_to_process(const std::string& process_name);
    void detach();

    std::string read_string(std::uint64_t address);
    void write_string(std::uint64_t address, const std::string& value);

    void write_buffer(std::uint64_t address, const void* buffer, std::size_t size);

    template <typename T>
    T read(std::uint64_t address, intptr_t* out_status = nullptr);

    template <typename T>
    void write(std::uint64_t address, T value);

    std::uint32_t get_process_id();
    std::uint64_t get_module_address();
    HANDLE get_process_handle();
    void reset_connection();

    std::atomic<bool> last_read_failed{ false };
    std::atomic<bool> last_write_failed{ false };

    void reset_error_flags() { last_read_failed = false; last_write_failed = false; }
    intptr_t read_raw(uint64_t address, void* buffer, ULONG size);
    intptr_t write_raw(uint64_t address, const void* buffer, ULONG size);
private:
    std::uint32_t process_id{};
    std::uint64_t base_address{};
    HANDLE process_handle{};
};

template <typename T>
T memory_t::read(uint64_t address, intptr_t* out_status)
{
    T buffer{};
    intptr_t status = read_raw(address, &buffer, static_cast<ULONG>(sizeof(T)));
    if (out_status) *out_status = status;
    if (status < 0) {
        last_read_failed = true;
        return T{};
    }
    return buffer;
}

template <typename T>
void memory_t::write(uint64_t address, T value)
{
    intptr_t status = write_raw(address, &value, static_cast<ULONG>(sizeof(T)));
    if (status < 0) last_write_failed = true;
}

inline std::unique_ptr<memory_t> memory = std::make_unique<memory_t>();