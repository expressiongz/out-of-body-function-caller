
#include <iostream>
#include <Windows.h>
#include <thread>
#include <vector>
#include <tuple>

typedef void(__stdcall* cextf_proto)(const std::uint32_t, const std::uint32_t);

enum calling_conventions : std::uint8_t
{
    stdcall_,
    cdecl_,
    fastcall_
};


template<typename func_ret_t, calling_conventions ccv, typename... function_args>
void __stdcall call_ext_func(const std::uint32_t function_address, const std::uint32_t tuple_addr)
{
    const auto function_args_tuples = *reinterpret_cast<std::tuple<function_args...>*>(tuple_addr);

    switch (ccv)
    {
		    case calling_conventions::stdcall_:
		    {
			      typedef func_ret_t(__stdcall* stdcall_proc_func_proto)(function_args...);
			      const auto stdcall_proc_func = reinterpret_cast<stdcall_proc_func_proto>(function_address);
			      std::apply(stdcall_proc_func, function_args_tuples);
			       break;
		    }

		    case calling_conventions::cdecl_:
		    {
			      typedef func_ret_t(__cdecl* cdecl_proc_func_proto)(function_args...);
			      const auto cdecl_proc_func = reinterpret_cast<cdecl_proc_func_proto>(function_address);
			      std::apply(cdecl_proc_func, function_args_tuples);
			      break;
         }
		     case calling_conventions::fastcall_:
		     {
			      typedef func_ret_t(__fastcall* fastcall_proc_func_proto)(function_args...);
			      const auto fastcall_proc_func = reinterpret_cast<fastcall_proc_func_proto>(function_address);
			      std::apply(fastcall_proc_func, function_args_tuples);
			      break;
		      }
    }
}

std::vector<std::uint8_t> read_func(void* function_address)
{
    std::vector<std::uint8_t> bytes_read{};
    auto const* curr_byte = static_cast<std::uint8_t*>(function_address);
    do
    {
        bytes_read.push_back(*curr_byte);
        curr_byte += 1;

    } while (*(curr_byte + 1) != 0xCC || *(curr_byte + 2) != 0xCC || *(curr_byte + 3) != 0xCC);
    return bytes_read;
}


void* alloc_func(void* func_address)
{
    const auto func_bytes = read_func(func_address);
    const auto funct_base_addr = VirtualAlloc(nullptr, func_bytes.size(), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    std::memcpy(funct_base_addr, func_bytes.data(), func_bytes.size());
    return funct_base_addr;
}


template<typename... tuple_el>
void* alloc_tuple(std::tuple<tuple_el...> tuple)
{   
    const auto tuple_addr = VirtualAlloc(nullptr, sizeof(tuple), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    std::memcpy(tuple_addr, &tuple, sizeof(tuple));
    return tuple_addr;
}


void main_thread()
{
    // imagine we are calling a print function that takes 1 string argument.
    constexpr std::tuple tpl{ "\n\nexprssnware is me\n"};
    const auto tuple_addr = alloc_tuple(tpl);
    const auto func_addr = alloc_func(reinterpret_cast<void*>(call_ext_func<void, calling_conventions::cdecl_, const char*>));

    const auto call_game_func = static_cast<cextf_proto>(func_addr);
    call_game_func(0x003B1000, reinterpret_cast<std::uint32_t>(tuple_addr));
}

bool __stdcall DllMain(HMODULE dll_module, const std::uint32_t reason_for_call, void* reserved)
{
    if (reason_for_call == DLL_PROCESS_ATTACH)
    {
        std::thread{ &main_thread }.detach();
    }
    return true;
}
