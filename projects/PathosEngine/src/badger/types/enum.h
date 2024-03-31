#pragma once

#include <type_traits>

#define ENUM_CLASS_FLAGS(EnumType) \
	inline EnumType operator|  (EnumType x, EnumType y)                         { return (EnumType)((__underlying_type(EnumType))x | (__underlying_type(EnumType))y); } \
	inline EnumType operator&  (EnumType x, EnumType y)                         { return (EnumType)((__underlying_type(EnumType))x & (__underlying_type(EnumType))y); } \
	inline bool     operator== (EnumType x, std::underlying_type_t<EnumType> y) { return           (__underlying_type(EnumType))x == y;                               } \
	inline bool     operator!= (EnumType x, std::underlying_type_t<EnumType> y) { return           (__underlying_type(EnumType))x != y;                               } \
	inline bool     operator== (std::underlying_type_t<EnumType> y, EnumType x) { return           (__underlying_type(EnumType))x == y;                               } \
	inline bool     operator!= (std::underlying_type_t<EnumType> y, EnumType x) { return           (__underlying_type(EnumType))x != y;                               }

#define ENUM_HAS_FLAG(EnumValue, Flag) (0 != (EnumValue & Flag))
