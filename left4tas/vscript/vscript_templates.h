//========== Copyright (c) 2008, Valve Corporation, All rights reserved. ========
//
// Purpose:
//
//=============================================================================

#ifndef VSCRIPT_TEMPLATES_H
#define VSCRIPT_TEMPLATES_H

#include "tier0/basetypes.h"

#if defined( _WIN32 )
#pragma once
#endif

#define	FUNC_APPEND_PARAMS_0	
#define	FUNC_APPEND_PARAMS_1	pDesc->m_Parameters.SetGrowSize( 1 ); pDesc->m_Parameters.EnsureCapacity( 1 ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_1 ) );
#define	FUNC_APPEND_PARAMS_2	pDesc->m_Parameters.SetGrowSize( 1 ); pDesc->m_Parameters.EnsureCapacity( 2 ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_1 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_2 ) );
#define	FUNC_APPEND_PARAMS_3	pDesc->m_Parameters.SetGrowSize( 1 ); pDesc->m_Parameters.EnsureCapacity( 3 ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_1 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_2 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_3 ) );
#define	FUNC_APPEND_PARAMS_4	pDesc->m_Parameters.SetGrowSize( 1 ); pDesc->m_Parameters.EnsureCapacity( 4 ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_1 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_2 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_3 ) );	pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_4 ) );
#define	FUNC_APPEND_PARAMS_5	pDesc->m_Parameters.SetGrowSize( 1 ); pDesc->m_Parameters.EnsureCapacity( 5 ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_1 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_2 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_3 ) );	pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_4 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_5 ) );
#define	FUNC_APPEND_PARAMS_6	pDesc->m_Parameters.SetGrowSize( 1 ); pDesc->m_Parameters.EnsureCapacity( 6 ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_1 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_2 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_3 ) );	pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_4 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_5 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_6 ) );
#define	FUNC_APPEND_PARAMS_7	pDesc->m_Parameters.SetGrowSize( 1 ); pDesc->m_Parameters.EnsureCapacity( 7 ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_1 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_2 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_3 ) );	pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_4 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_5 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_6 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_7 ) );
#define	FUNC_APPEND_PARAMS_8	pDesc->m_Parameters.SetGrowSize( 1 ); pDesc->m_Parameters.EnsureCapacity( 8 ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_1 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_2 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_3 ) );	pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_4 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_5 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_6 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_7 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_8 ) );
#define	FUNC_APPEND_PARAMS_9	pDesc->m_Parameters.SetGrowSize( 1 ); pDesc->m_Parameters.EnsureCapacity( 9 ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_1 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_2 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_3 ) );	pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_4 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_5 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_6 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_7 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_8 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_9 ) );
#define	FUNC_APPEND_PARAMS_10	pDesc->m_Parameters.SetGrowSize( 1 ); pDesc->m_Parameters.EnsureCapacity( 10 ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_1 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_2 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_3 ) );	pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_4 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_5 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_6 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_7 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_8 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_9 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_10 ) );
#define	FUNC_APPEND_PARAMS_11	pDesc->m_Parameters.SetGrowSize( 1 ); pDesc->m_Parameters.EnsureCapacity( 11 ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_1 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_2 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_3 ) );	pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_4 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_5 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_6 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_7 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_8 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_9 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_10 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_11 ) );
#define	FUNC_APPEND_PARAMS_12	pDesc->m_Parameters.SetGrowSize( 1 ); pDesc->m_Parameters.EnsureCapacity( 12 ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_1 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_2 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_3 ) );	pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_4 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_5 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_6 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_7 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_8 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_9 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_10 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_11 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_12 ) );
#define	FUNC_APPEND_PARAMS_13	pDesc->m_Parameters.SetGrowSize( 1 ); pDesc->m_Parameters.EnsureCapacity( 13 ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_1 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_2 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_3 ) );	pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_4 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_5 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_6 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_7 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_8 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_9 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_10 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_11 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_12 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_13 ) );
#define	FUNC_APPEND_PARAMS_14	pDesc->m_Parameters.SetGrowSize( 1 ); pDesc->m_Parameters.EnsureCapacity( 14 ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_1 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_2 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_3 ) );	pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_4 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_5 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_6 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_7 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_8 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_9 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_10 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_11 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_12 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_13 ) ); pDesc->m_Parameters.AddToTail( ScriptDeduceType( FUNC_ARG_TYPE_14 ) );

#define DEFINE_NONMEMBER_FUNC_TYPE_DEDUCER(N) \
	template <typename FUNCTION_RETTYPE FUNC_TEMPLATE_FUNC_PARAMS_##N> \
	inline void ScriptDeduceFunctionSignature(ScriptFuncDescriptor_t *pDesc, FUNCTION_RETTYPE (*pfnProxied)( FUNC_BASE_TEMPLATE_FUNC_PARAMS_##N ) ) \
	{ \
		pDesc->m_ReturnType = ScriptDeduceType(FUNCTION_RETTYPE); \
		FUNC_APPEND_PARAMS_##N \
	}

FUNC_GENERATE_ALL( DEFINE_NONMEMBER_FUNC_TYPE_DEDUCER );

#define DEFINE_MEMBER_FUNC_TYPE_DEDUCER(N) \
	template <typename OBJECT_TYPE_PTR, typename FUNCTION_CLASS, typename FUNCTION_RETTYPE FUNC_TEMPLATE_FUNC_PARAMS_##N> \
	inline void ScriptDeduceFunctionSignature(ScriptFuncDescriptor_t *pDesc, OBJECT_TYPE_PTR pObject, FUNCTION_RETTYPE ( FUNCTION_CLASS::*pfnProxied )( FUNC_BASE_TEMPLATE_FUNC_PARAMS_##N ) ) \
	{ \
		pDesc->m_ReturnType = ScriptDeduceType(FUNCTION_RETTYPE); \
		FUNC_APPEND_PARAMS_##N \
	}

FUNC_GENERATE_ALL( DEFINE_MEMBER_FUNC_TYPE_DEDUCER );

//-------------------------------------

#define DEFINE_CONST_MEMBER_FUNC_TYPE_DEDUCER(N) \
	template <typename OBJECT_TYPE_PTR, typename FUNCTION_CLASS, typename FUNCTION_RETTYPE FUNC_TEMPLATE_FUNC_PARAMS_##N> \
	inline void ScriptDeduceFunctionSignature(ScriptFuncDescriptor_t *pDesc, OBJECT_TYPE_PTR pObject, FUNCTION_RETTYPE ( FUNCTION_CLASS::*pfnProxied )( FUNC_BASE_TEMPLATE_FUNC_PARAMS_##N ) const ) \
	{ \
		pDesc->m_ReturnType = ScriptDeduceType(FUNCTION_RETTYPE); \
		FUNC_APPEND_PARAMS_##N \
	}

FUNC_GENERATE_ALL( DEFINE_CONST_MEMBER_FUNC_TYPE_DEDUCER );

#define ScriptInitMemberFuncDescriptor_( pDesc, class, func, scriptName )	if ( 0 ) {} else { (pDesc)->m_pszScriptName = scriptName; (pDesc)->m_pszFunction = #func; ScriptDeduceFunctionSignature( pDesc, (class *)(0), &class::func ); }

#define ScriptInitFuncDescriptorNamed( pDesc, func, scriptName )						if ( 0 ) {} else { (pDesc)->m_pszScriptName = scriptName; (pDesc)->m_pszFunction = #func; ScriptDeduceFunctionSignature( pDesc, &func ); }
#define ScriptInitFuncDescriptor( pDesc, func )											ScriptInitFuncDescriptorNamed( pDesc, func, #func )
#define ScriptInitMemberFuncDescriptorNamed( pDesc, class, func, scriptName )			ScriptInitMemberFuncDescriptor_( pDesc, class, func, scriptName )
#define ScriptInitMemberFuncDescriptor( pDesc, class, func )							ScriptInitMemberFuncDescriptorNamed( pDesc, class, func, #func )

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

template <typename FUNCPTR_TYPE>
inline void *ScriptConvertFreeFuncPtrToVoid( FUNCPTR_TYPE pFunc )
{
#if defined(_PS3) || defined(POSIX)
	COMPILE_TIME_ASSERT( sizeof( FUNCPTR_TYPE ) == sizeof( void* ) * 2 || sizeof( FUNCPTR_TYPE ) == sizeof( void* ) );
	
	if ( sizeof( FUNCPTR_TYPE ) == 4 )
	{
		union FuncPtrConvertMI
		{
			FUNCPTR_TYPE pFunc;
			void *stype;
		};

		FuncPtrConvertMI convert;
		convert.pFunc = pFunc;
		return convert.stype;
	}
	else
	{
		union FuncPtrConvertMI
		{
			FUNCPTR_TYPE pFunc;
			struct
			{
				void *stype;
				intptr_t iToc;
			} fn8;
		};

		FuncPtrConvertMI convert;
		convert.fn8.iToc = 0;
		convert.pFunc = pFunc;
		if ( !convert.fn8.iToc )
			return convert.fn8.stype;
		
		Assert( 0 );
		DebuggerBreak();
		return 0;
	}
#else
	return ( void * ) pFunc;
#endif
}

template <typename FUNCPTR_TYPE>
inline FUNCPTR_TYPE ScriptConvertFreeFuncPtrFromVoid( void *p )
{
#if defined(_PS3) || defined(POSIX)
	COMPILE_TIME_ASSERT( sizeof( FUNCPTR_TYPE ) == sizeof(void*)*2 || sizeof( FUNCPTR_TYPE ) == sizeof(void*) );

	if ( sizeof( FUNCPTR_TYPE ) == 4 )
	{
		union FuncPtrConvertMI
		{
			FUNCPTR_TYPE pFunc;
			void *stype;
		};

		FuncPtrConvertMI convert;
		convert.pFunc = 0;
		convert.stype = p;
		return convert.pFunc;
	}
	else
	{
		union FuncPtrConvertMI
		{
			FUNCPTR_TYPE pFunc;
			struct
			{
				void *stype;
				intptr_t iToc;
			} fn8;
		};

		FuncPtrConvertMI convert;
		convert.pFunc = 0;
		convert.fn8.stype = p;
		convert.fn8.iToc = 0;
		return convert.pFunc;
	}


#else
	return (FUNCPTR_TYPE) p;
#endif
}

template <typename FUNCPTR_TYPE>
inline void *ScriptConvertFuncPtrToVoid( FUNCPTR_TYPE pFunc )
{
	typedef FUNCPTR_TYPE FuncPtr_t;
	size_t funcPtrSize = sizeof( FuncPtr_t ); funcPtrSize;

#if defined(_PS3) || defined(POSIX)
	return ScriptConvertFreeFuncPtrToVoid<FUNCPTR_TYPE>( pFunc );
#else

	if ( ( sizeof( FUNCPTR_TYPE ) == sizeof( void * ) ) )
	{
		// simple inheritance
		union FuncPtrConvert
		{
			void *p;
			FUNCPTR_TYPE pFunc;
		};

		FuncPtrConvert convert;
		convert.pFunc = pFunc;
		return convert.p;
	}
#if MSVC
	else if ( ( IsPlatformWindowsPC32() && ( sizeof( FUNCPTR_TYPE ) == sizeof( void * ) + sizeof( int ) ) ) ||
	          ( IsPlatformWindowsPC64() && ( sizeof( FUNCPTR_TYPE ) == sizeof( void * ) + sizeof( int ) * 2 ) ) )
	{
		// multiple and virtual inheritance
		struct MicrosoftUnknownMFP
		{
			void *p;
			int m_delta;
		};
	
		union FuncPtrConvertMI
		{
			MicrosoftUnknownMFP mfp;
			FUNCPTR_TYPE pFunc;
		};

		FuncPtrConvertMI convert;
		convert.pFunc = pFunc;
		if ( convert.mfp.m_delta == 0 )
		{
			return convert.mfp.p;
		}
		AssertMsg( 0, "Function pointer must be from primary vtable" );
	}
	else if ( ( IsPlatformWindowsPC32() && ( sizeof( FUNCPTR_TYPE ) == sizeof( void * ) + ( sizeof( int ) * 3 ) ) ) || 
	          ( IsPlatformWindowsPC64() && ( sizeof( FUNCPTR_TYPE ) == sizeof( void * ) + ( sizeof( int ) * 4 ) ) ) )
	{
		// unknown_inheritance case
		struct MicrosoftUnknownMFP
		{
			void *p;
			int m_delta;
			int m_vtordisp;
			int m_vtable_index;
		};

		union FuncPtrConvertMI
		{
			MicrosoftUnknownMFP mfp;
			FUNCPTR_TYPE pFunc;
		};

		FuncPtrConvertMI convert;
		convert.pFunc = pFunc;
		if ( convert.mfp.m_delta == 0 )
		{
			return convert.mfp.p;
		}
		AssertMsg( 0, "Function pointer must be from primary vtable" );
	}
#elif defined( GNUC )
	else if ( ( sizeof( FUNCPTR_TYPE ) == sizeof( void * ) + sizeof( int ) ) )
	{
		AssertMsg( 0, "Note: This path has not been verified yet. See comments below in #else case." );
	
		struct GnuMFP
		{
			union
			{
				void *funcadr;		// If vtable_index_2 is even, then this is the function pointer.
				int vtable_index_2;		// If vtable_index_2 is odd, then this = vindex*2+1.
			};
			int delta;
		};
	
		GnuMFP *p = (GnuMFP*)&pFunc;
		if ( p->vtable_index_2 & 1 )
		{
			char **delta = (char**)p->delta;
			char *pCur = *delta + (p->vtable_index_2+1)/2;
			return (void*)( pCur + 4 );
		}
		else
		{
			return p->funcadr;
		}
	}
#endif
	else
		AssertMsg( 0, "Member function pointer not supported. Why on earth are you using virtual inheritance!?" );
	return NULL;
#endif
}

template <typename FUNCPTR_TYPE>
inline FUNCPTR_TYPE ScriptConvertFuncPtrFromVoid( void *p )
{
#if defined(_PS3) || defined(POSIX)
	return ScriptConvertFreeFuncPtrFromVoid<FUNCPTR_TYPE>( p );
#else

	if ( ( sizeof( FUNCPTR_TYPE ) == sizeof( void * ) ) )
	{
		union FuncPtrConvert
		{
			void *p;
			FUNCPTR_TYPE pFunc;
		};

		FuncPtrConvert convert;
		convert.p = p;
		return convert.pFunc;
	}

#if MSVC
	if ( ( sizeof( FUNCPTR_TYPE ) == sizeof( void * ) + sizeof( int ) ) )
	{
		struct MicrosoftUnknownMFP
		{
			void *p;
			int m_delta;
		};

		union FuncPtrConvertMI
		{
			MicrosoftUnknownMFP mfp;
			FUNCPTR_TYPE pFunc;
		};

		FuncPtrConvertMI convert;
		convert.mfp.p = p;
		convert.mfp.m_delta = 0;
		return convert.pFunc;
	}
	if ( ( sizeof( FUNCPTR_TYPE ) == sizeof( void * ) + ( sizeof( int ) * 3 ) ) )
	{
		struct MicrosoftUnknownMFP
		{
			void *p;
			int m_delta;
			int m_vtordisp;
			int m_vtable_index;
		};

		union FuncPtrConvertMI
		{
			MicrosoftUnknownMFP mfp;
			FUNCPTR_TYPE pFunc;
		};

		FuncPtrConvertMI convert;
		convert.mfp.p = p;
		convert.mfp.m_delta = 0;
		return convert.pFunc;
	}
#elif defined( POSIX )
	AssertMsg( 0, "Note: This path has not been implemented yet." );
#else
	AssertMsg(0, "Need to implement code to crack non-offset member function pointer case");
#endif
	Assert( 0 );
	return NULL;
#endif
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

#define FUNC_BASE_TEMPLATE_FUNC_PARAMS_PASSTHRU_0
#define FUNC_BASE_TEMPLATE_FUNC_PARAMS_PASSTHRU_1 , FUNC_BASE_TEMPLATE_FUNC_PARAMS_1
#define FUNC_BASE_TEMPLATE_FUNC_PARAMS_PASSTHRU_2 , FUNC_BASE_TEMPLATE_FUNC_PARAMS_2
#define FUNC_BASE_TEMPLATE_FUNC_PARAMS_PASSTHRU_3 , FUNC_BASE_TEMPLATE_FUNC_PARAMS_3
#define FUNC_BASE_TEMPLATE_FUNC_PARAMS_PASSTHRU_4 , FUNC_BASE_TEMPLATE_FUNC_PARAMS_4
#define FUNC_BASE_TEMPLATE_FUNC_PARAMS_PASSTHRU_5 , FUNC_BASE_TEMPLATE_FUNC_PARAMS_5
#define FUNC_BASE_TEMPLATE_FUNC_PARAMS_PASSTHRU_6 , FUNC_BASE_TEMPLATE_FUNC_PARAMS_6
#define FUNC_BASE_TEMPLATE_FUNC_PARAMS_PASSTHRU_7 , FUNC_BASE_TEMPLATE_FUNC_PARAMS_7
#define FUNC_BASE_TEMPLATE_FUNC_PARAMS_PASSTHRU_8 , FUNC_BASE_TEMPLATE_FUNC_PARAMS_8
#define FUNC_BASE_TEMPLATE_FUNC_PARAMS_PASSTHRU_9 , FUNC_BASE_TEMPLATE_FUNC_PARAMS_9
#define FUNC_BASE_TEMPLATE_FUNC_PARAMS_PASSTHRU_10 , FUNC_BASE_TEMPLATE_FUNC_PARAMS_10
#define FUNC_BASE_TEMPLATE_FUNC_PARAMS_PASSTHRU_11 , FUNC_BASE_TEMPLATE_FUNC_PARAMS_11
#define FUNC_BASE_TEMPLATE_FUNC_PARAMS_PASSTHRU_12 , FUNC_BASE_TEMPLATE_FUNC_PARAMS_12
#define FUNC_BASE_TEMPLATE_FUNC_PARAMS_PASSTHRU_13 , FUNC_BASE_TEMPLATE_FUNC_PARAMS_13
#define FUNC_BASE_TEMPLATE_FUNC_PARAMS_PASSTHRU_14 , FUNC_BASE_TEMPLATE_FUNC_PARAMS_14

#define	SCRIPT_BINDING_ARGS_0
#define	SCRIPT_BINDING_ARGS_1 pArguments[0]
#define	SCRIPT_BINDING_ARGS_2 pArguments[0], pArguments[1]
#define	SCRIPT_BINDING_ARGS_3 pArguments[0], pArguments[1], pArguments[2]
#define	SCRIPT_BINDING_ARGS_4 pArguments[0], pArguments[1], pArguments[2], pArguments[3]
#define	SCRIPT_BINDING_ARGS_5 pArguments[0], pArguments[1], pArguments[2], pArguments[3], pArguments[4]
#define	SCRIPT_BINDING_ARGS_6 pArguments[0], pArguments[1], pArguments[2], pArguments[3], pArguments[4], pArguments[5]
#define	SCRIPT_BINDING_ARGS_7 pArguments[0], pArguments[1], pArguments[2], pArguments[3], pArguments[4], pArguments[5], pArguments[6]
#define	SCRIPT_BINDING_ARGS_8 pArguments[0], pArguments[1], pArguments[2], pArguments[3], pArguments[4], pArguments[5], pArguments[6], pArguments[7]
#define	SCRIPT_BINDING_ARGS_9 pArguments[0], pArguments[1], pArguments[2], pArguments[3], pArguments[4], pArguments[5], pArguments[6], pArguments[7], pArguments[8]
#define	SCRIPT_BINDING_ARGS_10 pArguments[0], pArguments[1], pArguments[2], pArguments[3], pArguments[4], pArguments[5], pArguments[6], pArguments[7], pArguments[8], pArguments[9]
#define	SCRIPT_BINDING_ARGS_11 pArguments[0], pArguments[1], pArguments[2], pArguments[3], pArguments[4], pArguments[5], pArguments[6], pArguments[7], pArguments[8], pArguments[9], pArguments[10]
#define	SCRIPT_BINDING_ARGS_12 pArguments[0], pArguments[1], pArguments[2], pArguments[3], pArguments[4], pArguments[5], pArguments[6], pArguments[7], pArguments[8], pArguments[9], pArguments[10], pArguments[11]
#define	SCRIPT_BINDING_ARGS_13 pArguments[0], pArguments[1], pArguments[2], pArguments[3], pArguments[4], pArguments[5], pArguments[6], pArguments[7], pArguments[8], pArguments[9], pArguments[10], pArguments[11], pArguments[12]
#define	SCRIPT_BINDING_ARGS_14 pArguments[0], pArguments[1], pArguments[2], pArguments[3], pArguments[4], pArguments[5], pArguments[6], pArguments[7], pArguments[8], pArguments[9], pArguments[10], pArguments[11], pArguments[12], pArguments[13]


#define DEFINE_SCRIPT_BINDINGS(N) \
	template <typename FUNC_TYPE, typename FUNCTION_RETTYPE FUNC_TEMPLATE_FUNC_PARAMS_##N> \
	class CNonMemberScriptBinding##N \
	{ \
	public: \
 		static bool Call( void *pFunction, void *pContext, ScriptVariant_t *pArguments, int nArguments, ScriptVariant_t *pReturn ) \
 		{ \
			Assert( nArguments == N ); \
			Assert( pReturn ); \
			Assert( !pContext ); \
			\
			if ( nArguments != N || !pReturn || pContext ) \
			{ \
				return false; \
			} \
			*pReturn = (ScriptConvertFreeFuncPtrFromVoid<FUNC_TYPE>(pFunction))( SCRIPT_BINDING_ARGS_##N ); \
			if ( pReturn->m_type == FIELD_VECTOR ) \
				pReturn->m_pVector = new Vector(*pReturn->m_pVector); \
 			return true; \
 		} \
	}; \
	\
	template <typename FUNC_TYPE FUNC_TEMPLATE_FUNC_PARAMS_##N> \
	class CNonMemberScriptBinding##N<FUNC_TYPE, void FUNC_BASE_TEMPLATE_FUNC_PARAMS_PASSTHRU_##N> \
	{ \
	public: \
		static bool Call( void *pFunction, void *pContext, ScriptVariant_t *pArguments, int nArguments, ScriptVariant_t *pReturn ) \
		{ \
			Assert( nArguments == N ); \
			Assert( !pReturn ); \
			Assert( !pContext ); \
			\
			if ( nArguments != N || pReturn || pContext ) \
			{ \
				return false; \
			} \
			(ScriptConvertFreeFuncPtrFromVoid<FUNC_TYPE>(pFunction))( SCRIPT_BINDING_ARGS_##N ); \
			return true; \
		} \
	}; \
	\
	template <class OBJECT_TYPE_PTR, typename FUNC_TYPE, typename FUNCTION_RETTYPE FUNC_TEMPLATE_FUNC_PARAMS_##N> \
	class CMemberScriptBinding##N \
	{ \
	public: \
 		static bool Call( void *pFunction, void *pContext, ScriptVariant_t *pArguments, int nArguments, ScriptVariant_t *pReturn ) \
 		{ \
			Assert( nArguments == N ); \
			Assert( pReturn ); \
			Assert( pContext ); \
			\
			if ( nArguments != N || !pReturn || !pContext ) \
			{ \
				return false; \
			} \
			*pReturn = (((OBJECT_TYPE_PTR)(pContext))->*ScriptConvertFuncPtrFromVoid<FUNC_TYPE>(pFunction))( SCRIPT_BINDING_ARGS_##N ); \
			if ( pReturn->m_type == FIELD_VECTOR ) \
				pReturn->m_pVector = new Vector(*pReturn->m_pVector); \
 			return true; \
 		} \
	}; \
	\
	template <class OBJECT_TYPE_PTR, typename FUNC_TYPE FUNC_TEMPLATE_FUNC_PARAMS_##N> \
	class CMemberScriptBinding##N<OBJECT_TYPE_PTR, FUNC_TYPE, void FUNC_BASE_TEMPLATE_FUNC_PARAMS_PASSTHRU_##N> \
	{ \
	public: \
		static bool Call( void *pFunction, void *pContext, ScriptVariant_t *pArguments, int nArguments, ScriptVariant_t *pReturn ) \
		{ \
			Assert( nArguments == N ); \
			Assert( !pReturn ); \
			Assert( pContext ); \
			\
			if ( nArguments != N || pReturn || !pContext ) \
			{ \
				return false; \
			} \
			(((OBJECT_TYPE_PTR)(pContext))->*ScriptConvertFuncPtrFromVoid<FUNC_TYPE>(pFunction))( SCRIPT_BINDING_ARGS_##N ); \
			return true; \
		} \
	}; \
	\
	template <typename FUNCTION_RETTYPE FUNC_TEMPLATE_FUNC_PARAMS_##N> \
	inline ScriptBindingFunc_t ScriptCreateBinding(FUNCTION_RETTYPE (*pfnProxied)( FUNC_BASE_TEMPLATE_FUNC_PARAMS_##N ) ) \
	{ \
		typedef FUNCTION_RETTYPE (*Func_t)(FUNC_BASE_TEMPLATE_FUNC_PARAMS_##N); \
		return &CNonMemberScriptBinding##N<Func_t, FUNCTION_RETTYPE FUNC_BASE_TEMPLATE_FUNC_PARAMS_PASSTHRU_##N>::Call; \
	} \
	\
	template <typename OBJECT_TYPE_PTR, typename FUNCTION_CLASS, typename FUNCTION_RETTYPE FUNC_TEMPLATE_FUNC_PARAMS_##N> \
		inline ScriptBindingFunc_t ScriptCreateBinding(OBJECT_TYPE_PTR pObject, FUNCTION_RETTYPE (FUNCTION_CLASS::*pfnProxied)( FUNC_BASE_TEMPLATE_FUNC_PARAMS_##N ) ) \
	{ \
		typedef FUNCTION_RETTYPE (FUNCTION_CLASS::*Func_t)(FUNC_BASE_TEMPLATE_FUNC_PARAMS_##N); \
		return &CMemberScriptBinding##N<OBJECT_TYPE_PTR, Func_t, FUNCTION_RETTYPE FUNC_BASE_TEMPLATE_FUNC_PARAMS_PASSTHRU_##N>::Call; \
	} \
	\
	template <typename OBJECT_TYPE_PTR, typename FUNCTION_CLASS, typename FUNCTION_RETTYPE FUNC_TEMPLATE_FUNC_PARAMS_##N> \
		inline ScriptBindingFunc_t ScriptCreateBinding(OBJECT_TYPE_PTR pObject, FUNCTION_RETTYPE (FUNCTION_CLASS::*pfnProxied)( FUNC_BASE_TEMPLATE_FUNC_PARAMS_##N ) const ) \
	{ \
		typedef FUNCTION_RETTYPE (FUNCTION_CLASS::*Func_t)(FUNC_BASE_TEMPLATE_FUNC_PARAMS_##N); \
		return &CMemberScriptBinding##N<OBJECT_TYPE_PTR, Func_t, FUNCTION_RETTYPE FUNC_BASE_TEMPLATE_FUNC_PARAMS_PASSTHRU_##N>::Call; \
	}

FUNC_GENERATE_ALL( DEFINE_SCRIPT_BINDINGS );

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

#endif // VSCRIPT_TEMPLATES_H
