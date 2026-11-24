#include "MenuScreen.hpp"

#include "Icons.hpp"
#include "PixelFont.hpp"
#include "Theme.hpp"

#include <algorithm>
#include <string>

using namespace std;

namespace {

struct Difficulty {
	const char* name;
	Settings settings;
};

// The classic levels: beginner 9x9/10, intermediate 16x16/40, expert 16x30/99
constexpr Difficulty DIFFICULTIES[3] = {
    {"ALGAJA", {9, 9, 10}},
    {"KESKMINE", {16, 16, 40}},
    {"EKSPERT", {16, 30, 99}},
};

string describe(const Settings& s) {
	return to_string(s.rows) + "x" + to_string(s.columns) + " · " + to_string(s.mines);
}

bool sameSettings(const Settings& a, const Settings& b) {
	return a.rows == b.rows && a.columns == b.columns && a.mines == b.mines;
}

constexpr float STEPPER_Y[3] = {166.f, 182.f, 198.f}; // rows, columns and mines lines
constexpr float REPEAT_DELAY = 0.4f;                  // hold this long before repeating
constexpr float REPEAT_INTERVAL = 0.05f;

} // namespace

MenuScreen::MenuScreen() {
	for (size_t i = 0; i < 3; i++) {
		buttons[BEGINNER + i] = {sf::FloatRect({12.f, 88.f + 20.f * i}, {152.f, 16.f}), DIFFICULTIES[i].name,
		                         describe(DIFFICULTIES[i].settings)};
	}
	for (size_t line = 0; line < 3; line++) {
		buttons[ROWS_MINUS + 2 * line] = {sf::FloatRect({100.f, STEPPER_Y[line]}, {14.f, 12.f}), "-"};
		buttons[ROWS_PLUS + 2 * line] = {sf::FloatRect({150.f, STEPPER_Y[line]}, {14.f, 12.f}), "+"};
	}
	buttons[START] = {sf::FloatRect({12.f, 214.f}, {104.f, 16.f}), "ALUSTA"};
	buttons[QUIT] = {sf::FloatRect({120.f, 214.f}, {44.f, 16.f}), "VÄLJU"};
	current = DIFFICULTIES[0].settings;
	updateSelection();
}

optional<Settings> MenuScreen::takeStartRequest() {
	if (!startRequested) {
		return nullopt;
	}
	startRequested = false;
	return current;
}

optional<size_t> MenuScreen::buttonAt(sf::Vector2f mouse) const {
	for (size_t i = 0; i < BUTTON_COUNT; i++) {
		if (buttons[i].contains(mouse)) {
			return i;
		}
	}
	return nullopt;
}

void MenuScreen::handleEvent(const sf::Event& event, sf::Vector2f mouse) {
	if (event.is<sf::Event::MouseMoved>()) {
		hoveredButton = buttonAt(mouse);
	} else if (event.is<sf::Event::MouseLeft>()) {
		hoveredButton.reset();
	} else if (const auto* press = event.getIf<sf::Event::MouseButtonPressed>()) {
		if (press->button != sf::Mouse::Button::Left) {
			return;
		}
		pressedButton = buttonAt(mouse);
		holdTime = 0.f;
		repeatTime = 0.f;
		// +/- change right away (and repeatedly while held), other buttons act on release
		if (pressedButton && isStepper(*pressedButton)) {
			step(*pressedButton);
		}
	} else if (const auto* release = event.getIf<sf::Event::MouseButtonReleased>()) {
		if (release->button != sf::Mouse::Button::Left) {
			return;
		}
		if (pressedButton && buttonAt(mouse) == pressedButton) {
			activate(*pressedButton);
		}
		pressedButton.reset();
	} else if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
		switch (key->code) {
		case sf::Keyboard::Key::Enter:
			startRequested = true;
			break;
		case sf::Keyboard::Key::Escape:
			quit = true;
			break;
		case sf::Keyboard::Key::Num1:
			selectDifficulty(BEGINNER);
			break;
		case sf::Keyboard::Key::Num2:
			selectDifficulty(INTERMEDIATE);
			break;
		case sf::Keyboard::Key::Num3:
			selectDifficulty(EXPERT);
			break;
		default:
			break;
		}
	}
}

void MenuScreen::update(float seconds) {
	if (!pressedButton || !isStepper(*pressedButton) || hoveredButton != pressedButton) {
		return;
	}
	holdTime += seconds;
	if (holdTime < REPEAT_DELAY) {
		return;
	}
	repeatTime += seconds;
	while (repeatTime >= REPEAT_INTERVAL) {
		repeatTime -= REPEAT_INTERVAL;
		step(*pressedButton);
	}
}

void MenuScreen::activate(size_t id) {
	if (id <= EXPERT) {
		selectDifficulty(id);
	} else if (id == START) {
		startRequested = true;
	} else if (id == QUIT) {
		quit = true;
	}
}

void MenuScreen::step(size_t id) {
	switch (id) {
	case ROWS_MINUS:
		current.rows--;
		break;
	case ROWS_PLUS:
		current.rows++;
		break;
	case COLUMNS_MINUS:
		current.columns--;
		break;
	case COLUMNS_PLUS:
		current.columns++;
		break;
	case MINES_MINUS:
		current.mines--;
		break;
	case MINES_PLUS:
		current.mines++;
		break;
	default:
		return;
	}
	current.rows = clamp(current.rows, MIN_SIDE, MAX_ROWS);
	current.columns = clamp(current.columns, MIN_SIDE, MAX_COLUMNS);
	clampMines();
	updateSelection();
}

void MenuScreen::selectDifficulty(size_t id) {
	current = DIFFICULTIES[id - BEGINNER].settings;
	updateSelection();
}

// At least one mine and at least one free cell
void MenuScreen::clampMines() {
	current.mines = clamp(current.mines, 1, current.rows * current.columns - 1);
}

void MenuScreen::updateSelection() {
	for (size_t i = 0; i < 3; i++) {
		buttons[BEGINNER + i].selected = sameSettings(current, DIFFICULTIES[i].settings);
	}
}

void MenuScreen::draw(Canvas& canvas) const {
	const float centerX = WIDTH / 2.f;
	canvas.rect(0, 0, WIDTH, HEIGHT, theme::BACKGROUND);

	pixelfont::drawShadowed(canvas, "MINESWEEPER", 23, 12, theme::TEXT, theme::SHADOW, 2.f);
	const icons::Face face = pressedButton ? icons::Face::Pressing : icons::Face::Normal;
	icons::face(canvas, face, centerX - 16, 36, 2.f);

	pixelfont::drawCentered(canvas, "RASKUSASTE", centerX, 77, theme::TEXT_DIM);
	pixelfont::drawCentered(canvas, "KOHANDATUD", centerX, 153, theme::TEXT_DIM);

	const char* labels[3] = {"KÕRGUS", "LAIUS", "POMME"};
	const int values[3] = {current.rows, current.columns, current.mines};
	for (int line = 0; line < 3; line++) {
		pixelfont::draw(canvas, labels[line], 14, STEPPER_Y[line] + 2, theme::TEXT);
		pixelfont::drawCentered(canvas, to_string(values[line]), 132, STEPPER_Y[line] + 2, theme::TEXT);
	}

	for (size_t i = 0; i < BUTTON_COUNT; i++) {
		const bool hovered = hoveredButton == i;
		drawButton(canvas, buttons[i], hovered, hovered && pressedButton == i);
	}
}
