#include <iostream>
#include <Windows.h>
#include <thread>
#include <vector>
#include <tuple>

typedef void( __stdcall* cextf_proto )( const std::uint32_t, const std::uint32_t );

enum calling_conventions : std::uint8_t
{
    stdcall_,
    cdecl_,
    fastcall_
};


template< typename func_ret_t, calling_conventions ccv, typename... function_args >
void __stdcall call_ext_func( std::uint32_t function_address, std::uint32_t tuple_addr )
{
    const auto function_args_tuples = *reinterpret_cast< std::tuple< function_args... > * >( tuple_addr );

    switch ( ccv )
    {
		    case calling_conventions::stdcall_:
		    {
			      typedef func_ret_t( __stdcall* stdcall_proc_func_proto )( function_args... );
			      const auto stdcall_proc_func = reinterpret_cast< stdcall_proc_func_proto >( function_address );
			      std::apply( stdcall_proc_func, function_args_tuples );
			      break;
		    }

		    case calling_conventions::cdecl_:
		    {
			      typedef func_ret_t( __cdecl* cdecl_proc_func_proto )( function_args... );
			      const auto cdecl_proc_func = reinterpret_cast< cdecl_proc_func_proto >( function_address );
			      std::apply( cdecl_proc_func, function_args_tuples );
			      break;
			}
		    case calling_conventions::fastcall_:
		    {
			      typedef func_ret_t( __fastcall* fastcall_proc_func_proto )( function_args... );
			      const auto fastcall_proc_func = reinterpret_cast< fastcall_proc_func_proto >( function_address );
			      std::apply( fastcall_proc_func, function_args_tuples );
			      break;
		    }
    }
}

std::vector< std::uint8_t > ret_function_instrs( void* function_address )
{
	std::vector< std::uint8_t > function_bytes{ };
	auto byte = static_cast< std::uint8_t* >( function_address );
	do 
	{
		function_bytes.push_back( *byte );
		++byte;
	} while ( *reinterpret_cast< std::uint32_t* >( byte ) != 0xCCCCCCCC );
	return function_bytes;
}


void* alloc_func(void* func_address)
{
    const auto function_instructions = ret_function_instrs( func_address );
    const auto allocated_function = VirtualAlloc( nullptr, function_instructions.size( ), MEM_COMMIT, PAGE_EXECUTE_READWRITE );

	if( !allocated_function )
		throw std::runtime_error( "failed to allocate memory for function." );

    std::memcpy( allocated_function, function_instructions.data( ), function_instructions.size( ) );
    return allocated_function;
}


template< typename... tuple_el >
void* alloc_tuple( std::tuple< tuple_el... > tuple )
{   
    const auto allocated_tuple = VirtualAlloc( nullptr, sizeof( tuple ), MEM_COMMIT, PAGE_EXECUTE_READWRITE );
	if( !allocated_tuple )
		throw std::runtime_error( "failed to allocate memory for tuple" );

    std::memcpy( allocated_tuple, &tuple, sizeof( tuple ) );
    return allocated_tuple;
}


void main_thread( HMODULE dll_module )
{
	const auto base = reinterpret_cast< std::uint32_t >( GetModuleHandleA( nullptr ) );
	constexpr auto function_rva = 0x1000;

    constexpr auto arg_tuple = std::make_tuple< const char* >( { "exprssn" } );
	const auto tuple = alloc_tuple( arg_tuple );

    const auto caller_func_address = alloc_func( reinterpret_cast< void* >( call_ext_func< void, calling_conventions::cdecl_, std::string_view > ) );
	const auto caller = static_cast< cextf_proto >( caller_func_address );

    caller( base + function_rva, reinterpret_cast< std::uint32_t >( tuple ) );
	FreeLibrary( dll_module );
}

bool __stdcall DllMain( HMODULE dll_module, const std::uint32_t reason_for_call, void*  )
{
    if ( reason_for_call == DLL_PROCESS_ATTACH )
        std::thread{ &main_thread, dll_module }.detach( );
    return true;
}
