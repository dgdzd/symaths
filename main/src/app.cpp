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
	auto add_to_history = [&](const std::string& cmd, const std::string& output) {
		if (cmd.empty()) return;

		m_history_items.push_back({cmd, output});

		cmds_history->DetachAllChildren();
		for (const auto& item : m_history_items) {
			ButtonOption option_in = ButtonOption::Simple();
			ButtonOption option_out = ButtonOption::Simple();

			option_in.transform = [item](const EntryState& state) {
				auto content = hbox({
					text(" ") ,
					paragraph(std::format("> {}", item.input)) | color(Color::White)
				});

				if (state.focused) {
					content = content | inverted;
				}

				return content | size(HEIGHT, EQUAL, 1);
			};

			option_out.transform = [item](const EntryState& state) {
				auto content = hbox({
					text(" ") ,
					paragraph(item.output),
				});

				if (state.focused) {
					content = content | inverted;
				}

				return content | size(HEIGHT, EQUAL, 1) | color(Color::Grey30);
			};

			auto btn_in = Button(item.input, [&, item] {
				m_input = item.input;
			}, option_in);
			auto btn_out = Button(item.output, [&, item] {
				m_input = item.output;
			}, option_out);

			cmds_history->Add(btn_in);
			if (!item.output.empty()) cmds_history->Add(btn_out);
		}
	};

	int cursor_pos = 0;
	InputOption input_option;
	input_option.cursor_position = &cursor_pos;
	input_option.multiline = true;

	auto input_component = Input(&m_input, "Type a command...", input_option);
	input_component |= CatchEvent([&](Event e) {
		if (e == Event::CtrlN) {
			m_input.insert(cursor_pos, "\n");
			cursor_pos += 1;
			return true;
		}

		if (e == Event::Return) {
			m_input = utils::trim(m_input);
			if (!m_input.empty()) {
				std::ostringstream oss;
				sym::object out = sym::evaluate(m_input, nullptr, oss);
				add_to_history(m_input, oss.str());
				m_input.clear();
				cursor_pos = 0;
			}
			return true;
		}
		return false;
	});

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
