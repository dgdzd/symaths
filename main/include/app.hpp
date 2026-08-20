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

#ifndef SYM_APP_HPP
#define SYM_APP_HPP

#include <cstdint>
#include <ftxui/ftxui.hpp>
#include <symaths/symaths.hpp>

namespace symtool {
	/**
	 * @brief An RAII-based class representing the application.
	 * There are multiple ways of using this app : \n
	 * (1) no arguments : Launch the symaths shell with default settings \n
	 * (2) "--develop" : Develop the given expression \n
	 * (3) "--reduces" : Reduce the given expression \n
	 * (4) "--simplify" : Simplify (develop + reduce) the given expression
	 **/
	class app {
	public:
		enum class mode : uint8_t {
			NONE,
			shell,
			fast,
		};

		struct command_log {
			std::string input;
			std::string output;
		};

	private:
		sym::library lib;
		mode m_mode;
		std::string m_input;
		std::vector<command_log> m_history_items;

	public:
		app();
		app(int argc, char* argv[]);
		~app();

		operator bool() const;

		void run();
	};
}

#endif
