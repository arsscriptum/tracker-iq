
//==============================================================================
//
//     nowarns.h : remove warnings
//
//==============================================================================
// Copyright (C)  Guillaume Plante <codegp@icloud.com>
//==============================================================================
 
#ifndef __NO_WARNS_H__
#define __NO_WARNS_H__

#ifndef _MSC_VER
#   error This file is specific to the MSVC (Microsoft Visual C++) compiler.
#endif
 
#ifndef CPPX_ALLOW_WP64
#   // The /Wp64 option generates spurious warnings when a __w64 type argument selects
#   // a correct overload with non-__w64 formal argument type, i.e. for <<. In newer
#   // versions of MSVC this option is deprecated. It Really Annoyed a lot of people!
#   ifdef  _Wp64
#       error Do not use the /Wp64 option: use a 64-bit compiler to detect 64-bit portability issues.
#   endif
#endif
 
// The following are real warnings but are generated by almost all MS headers, including
// standard library headers, so it's impractical to leave them on.
#pragma  warning( disable: 4619 )   // there is no warning number 'XXXX'
#pragma  warning( disable: 4668 )   // XXX is not defined as a preprocessor macro
#pragma  warning( disable: 4189 )
#pragma  warning( disable: 4100 )
#pragma  warning( disable: 4245 )
// The following are pure sillywarnings:
#pragma warning( disable: 4061 )    // enum value is not *explicitly* handled in switch
#pragma warning( disable: 4099 )    // first seen using 'struct' now seen using 'class'
#pragma warning( disable: 4127 )    // conditional expression is constant
#pragma warning( disable: 4217 )    // member template isn't copy constructor
#pragma warning( disable: 4250 )    // inherits (implements) some member via dominance
#pragma warning( disable: 4251 )    // needs to have dll-interface to be used by clients
#pragma warning( disable: 4275 )    // exported class derived from non-exported class
#pragma warning( disable: 4347 )    // "behavior change", function called instead of template
#pragma warning( disable: 4355 )    // "'this': used in member initializer list
#pragma warning( disable: 4428 )    // MSVC 9: universal-character-name encountered in source
#pragma warning( disable: 4505 )    // unreferenced function has been removed
#pragma warning( disable: 4510 )    // default constructor could not be generated
#pragma warning( disable: 4511 )    // copy constructor could not be generated
#pragma warning( disable: 4512 )    // assignment operator could not be generated
#pragma warning( disable: 4513 )    // destructor could not be generated
#pragma warning( disable: 4610 )    // can never be instantiated user defined constructor required
#pragma warning( disable: 4623 )    // default constructor could not be generated
#pragma warning( disable: 4624 )    // destructor could not be generated
#pragma warning( disable: 4625 )    // copy constructor could not be generated
#pragma warning( disable: 4626 )    // assignment operator could not be generated
#pragma warning( disable: 4640 )    // a local static object is not thread-safe
#pragma warning( disable: 4661 )    // a member of the template class is not defined.
#pragma warning( disable: 4670 )    // a base class of an exception class is inaccessible for catch
#pragma warning( disable: 4672 )    // a base class of an exception class is ambiguous for catch
#pragma warning( disable: 4673 )    // a base class of an exception class is inaccessible for catch
#pragma warning( disable: 4675 )    // resolved overload was found by argument-dependent lookup
#pragma warning( disable: 4702 )    // unreachable code, e.g. in <list> header.
#pragma warning( disable: 4710 )    // call was not inlined
#pragma warning( disable: 4711 )    // call was inlined
#pragma warning( disable: 4820 )    // some padding was added
#pragma warning( disable: 4917 )    // a GUID can only be associated with a class, interface or namespace
#pragma warning( disable: 4996 )    // MSVC 9: a C std library function has been "deprecated" (says MS)

#pragma warning( disable: 6031 )
#pragma warning( disable: 6001 )
#pragma warning( disable: 6262 )
#pragma warning( disable: 6387 )

#endif //__NO_WARNS_H__