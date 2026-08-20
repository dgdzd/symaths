/*
 *	                            _   _
 *	  ___ _   _ _ __ ___   __ _| |_| |__  ___
 *	 / __| | | | '_ ` _ \ / _` | __| '_ \/ __|   Symbolic maths for C++
 *	 \__ \ |_| | | | | | | (_| | |_| | | \__ \   Version : 0.0.1
 *	 |___/\__, |_| |_| |_|\__,_|\__|_| |_|___/   https://github.com/dgdzd/symaths
 *		  |___/
 *
 * All source code is distributed under the GNU General Public License v2.0.
 *
 */

#ifndef SYM_HELPERS_HPP
#define SYM_HELPERS_HPP

#include <variant>

// Helper class for std::visit
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template<typename VariantType, typename T, size_t index = 0>
constexpr size_t variant_index() {
	static_assert(std::variant_size_v<VariantType> > index, "Type not found in variant");
	if constexpr (index == std::variant_size_v<VariantType>) {
		return index;
	} else if constexpr (std::is_same_v<std::variant_alternative_t<index, VariantType>, T>) {
		return index;
	} else {
		return variant_index<VariantType, T, index + 1>();
	}
}

#endif
