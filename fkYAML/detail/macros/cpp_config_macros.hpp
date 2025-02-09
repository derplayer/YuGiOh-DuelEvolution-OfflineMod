///  _______   __ __   __  _____   __  __  __
/// |   __| |_/  |  \_/  |/  _  \ /  \/  \|  |     fkYAML: A C++ header-only YAML library
/// |   __|  _  < \_   _/|  ___  |    _   |  |___  version 0.3.1
/// |__|  |_| \__|  |_|  |_|   |_|___||___|______| https://github.com/fktn-k/fkYAML
///
/// SPDX-FileCopyrightText: 2023 Kensuke Fukutani <fktn.dev@gmail.com>
/// SPDX-License-Identifier: MIT
///
/// @file

#ifndef FK_YAML_DETAIL_MACROS_CPP_CONFIG_MACROS_HPP_
#define FK_YAML_DETAIL_MACROS_CPP_CONFIG_MACROS_HPP_

// This file is assumed to be included only by version_macros.hpp file.
// To avoid redundant inclusion, do not include version_macros.hpp file as the other files do.

// C++ language standard detection (__cplusplus is not yet defined for C++23)
// Skip detection if the definitions listed below already exist.
#if !defined(FK_YAML_HAS_CXX_20) && !defined(FK_YAML_HAS_CXX_17) && !defined(FK_YAML_HAS_CXX_14) &&                    \
    !defined(FK_YAML_CXX_11)
    #if (defined(__cplusplus) && __cplusplus >= 202002L) || (defined(_MSVC_LANG) && MSVC_LANG >= 202002L)
        #define FK_YAML_HAS_CXX_20
        #define FK_YAML_HAS_CXX_17
        #define FK_YAML_HAS_CXX_14
    #elif (defined(__cplusplus) && __cplusplus >= 201703L) || (defined(_HAS_CXX17) && _HAS_CXX17 == 1)
        #define FK_YAML_HAS_CXX_17
        #define FK_YAML_HAS_CXX_14
    #elif (defined(__cplusplus) && __cplusplus >= 201402L) || (defined(_HAS_CXX14) && _HAS_CXX14 == 1)
        #define FK_YAML_HAS_CXX_14
    #endif

    // C++11 is the minimum required version of the fkYAML library.
    #define FK_YAML_HAS_CXX_11
#endif

// switch usage of inline variables. Inline variables have been introduced since C++17.
#if defined(FK_YAML_HAS_CXX_17)
    #define FK_YAML_INLINE_VAR inline
#else
    #define FK_YAML_INLINE_VAR
#endif

#endif /* FK_YAML_DETAIL_MACROS_CPP_CONFIG_MACROS_HPP_ */