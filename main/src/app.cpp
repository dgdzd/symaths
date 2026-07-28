#include "app.hpp"

#include "utils/strings.hpp"

using namespace symtool;
using namespace ftxui;

app::app() {
	m_mode = mode::shell;
}

app::app(int argc, char* argv[]) {
	if (argc == 1) {
		m_mode = mode::shell;
		return;
	}

}

app::~app() {
}

app::operator bool() const {
	return m_mode != mode::NONE;
}

void app::run() {
	auto screen = App::Fullscreen();

	// HOME TAB

	auto first_log = Button("Welcome to symaths ! Instructions and their output will be shown here.", []{}, ButtonOption::Ascii());
	auto cmds_history = Container::Vertical({first_log});

	// Clear and rebuild list
	auto add_to_history = [&](const std::string& cmd) {
		if (cmd.empty()) return;

		m_history_items.push_back({cmd, "HO LEE SHEET"});

		cmds_history->DetachAllChildren();
		for (const auto& item : m_history_items) {
			ButtonOption option = ButtonOption::Simple();

			option.transform = [item](const EntryState& state) {
				auto content = hbox({
					text(" ") ,
					paragraph(item.input),
					filler(),
					paragraph(item.output) | dim | color(Color::GrayDark),
					text(" ") ,
				});

				if (state.focused) {
					content = content | inverted;
				}

				return content | size(HEIGHT, EQUAL, 1);
			};

			auto btn = Button(item.input, [&, item] {
				m_input = item.input;
			}, option);

			cmds_history->Add(btn);
		}
	};

	InputOption input_option;
	input_option.on_enter = [&] {
		m_input.pop_back(); // Remove '\n' char
		m_input = utils::trim(m_input);
		if (!m_input.empty()) {
			add_to_history(m_input);
			m_input.clear();
		}
	};

	auto input_component = Input(&m_input, "Type a command...", input_option);

	auto home_component = Container::Vertical({
		cmds_history,
		input_component,
	});

	auto home_renderer = Renderer(home_component, [&] {
		return vbox({
			cmds_history->Render() | vscroll_indicator | frame | flex,
			input_component->Render() | border,
		});
	});

	int tab_index = 0;
	std::vector<std::string> tab_entries = {"Home"};

	auto tab_selection = Menu(&tab_entries, &tab_index, MenuOption::HorizontalAnimated());
	auto tab_content = Container::Tab({
		home_renderer,
	}, &tab_index);

	auto main_container = Container::Vertical({
		Container::Horizontal({
			tab_selection,
		}),
		tab_content
	});

	auto main_renderer = Renderer(main_container, [&] {
		return vbox({
			text("symaths — command line tool") | bold | hcenter,
			hbox({
				tab_selection->Render() | flex,
			}),
			tab_content->Render() | flex,
		});
	  });

	screen.Loop(main_renderer);
}
