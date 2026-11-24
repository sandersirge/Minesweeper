#include "gui/game_screen.hpp"

#include "gui/pixel_font.hpp"
#include "gui/theme.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>

using namespace std;

namespace {

constexpr float CELL = static_cast<float>(theme::CELL);
constexpr float WAVE_STEP = 0.035f;  // seconds per cell of distance
constexpr float WAVE_JITTER = 0.05f; // random extra delay so the wave looks lively
constexpr float WAVE_CELL = 0.28f;   // how long the effect lasts on one cell
constexpr float BANNER_HEIGHT = 56.f;

} // namespace

void GameScreen::start(const Settings& newSettings) {
	settings = newSettings;
	minesweeper.setMessages(false);
	minesweeper.newGameWithMines(settings.rows, settings.columns, static_cast<size_t>(settings.mines));
	const sf::Vector2u size = logicalSize();
	boardX = floor((size.x - settings.columns * CELL) / 2.f);
	boardY = static_cast<float>(theme::BAR);
	hoveredCell.reset();
	pressedCell.reset();
	leftDown = facePressed = faceHovered = false;
	backToMenu = false;
	elapsed = sinceEnd = 0.f;
	waveActive = false;
	waveDelays.clear();
}

sf::Vector2u GameScreen::logicalSize() const {
	const unsigned width = max<unsigned>(settings.columns * theme::CELL, theme::MIN_WIDTH);
	return {width, static_cast<unsigned>(theme::BAR + settings.rows * theme::CELL)};
}

optional<GameScreen::CellPosition> GameScreen::cellAt(sf::Vector2f mouse) const {
	const float x = mouse.x - boardX;
	const float y = mouse.y - boardY;
	if (x < 0 || y < 0) {
		return nullopt;
	}
	const int column = static_cast<int>(x / CELL);
	const int row = static_cast<int>(y / CELL);
	if (column >= settings.columns || row >= settings.rows) {
		return nullopt;
	}
	return CellPosition{row, column};
}

sf::FloatRect GameScreen::faceArea() const {
	const float centerX = floor(logicalSize().x / 2.f);
	return sf::FloatRect({centerX - 10.f, 2.f}, {20.f, 20.f});
}

void GameScreen::handleEvent(const sf::Event& event, sf::Vector2f mouse) {
	if (event.is<sf::Event::MouseMoved>()) {
		hoveredCell = cellAt(mouse);
		faceHovered = faceArea().contains(mouse);
	} else if (event.is<sf::Event::MouseLeft>()) {
		hoveredCell.reset();
		faceHovered = false;
	} else if (const auto* press = event.getIf<sf::Event::MouseButtonPressed>()) {
		const auto cell = cellAt(mouse);
		switch (press->button) {
		case sf::Mouse::Button::Left:
			if (faceArea().contains(mouse)) {
				facePressed = true;
			} else if (cell && !minesweeper.isOver()) {
				leftDown = true;
				pressedCell = cell;
			}
			break;
		case sf::Mouse::Button::Right:
			if (cell) {
				play(*cell, true, false);
			}
			break;
		case sf::Mouse::Button::Middle:
			if (cell) {
				play(*cell, false, true);
			}
			break;
		default:
			break;
		}
	} else if (const auto* release = event.getIf<sf::Event::MouseButtonReleased>()) {
		if (release->button != sf::Mouse::Button::Left) {
			return;
		}
		if (facePressed && faceArea().contains(mouse)) {
			start(settings);
		} else if (leftDown) {
			// A cell opens on release, and only if the mouse is still on the same cell
			const auto cell = cellAt(mouse);
			if (cell && cell == pressedCell) {
				play(*cell, false, false);
			}
		}
		facePressed = false;
		leftDown = false;
		pressedCell.reset();
	} else if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
		if (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::F2) {
			start(settings);
		} else if (key->code == sf::Keyboard::Key::Escape) {
			backToMenu = true;
		}
	}
}

void GameScreen::play(const CellPosition& cell, bool flag, bool chord) {
	if (minesweeper.isOver()) {
		return;
	}
	if (flag) {
		minesweeper.makeMove(cell.row, cell.column, true);
		return;
	}
	const char current = minesweeper.cell(cell.row, cell.column);
	if (chord || (current >= '1' && current <= '8')) {
		minesweeper.chord(cell.row, cell.column);
	} else {
		minesweeper.makeMove(cell.row, cell.column, false);
	}

	if (minesweeper.isOver()) {
		startWave(cell);
	}
}

// When the game ends a wave spreads from the clicked cell (red on a loss, white on a win)
void GameScreen::startWave(const CellPosition& origin) {
	uniform_real_distribution<float> jitter(0.f, WAVE_JITTER);
	waveDelays.assign(static_cast<size_t>(settings.rows) * settings.columns, 0.f);
	waveLength = 0.f;
	for (int r = 0; r < settings.rows; r++) {
		for (int c = 0; c < settings.columns; c++) {
			const float distance =
			    hypot(static_cast<float>(r - origin.row), static_cast<float>(c - origin.column));
			const float delay = distance * WAVE_STEP + jitter(rng);
			waveDelays[static_cast<size_t>(r) * settings.columns + c] = delay;
			waveLength = max(waveLength, delay + WAVE_CELL);
		}
	}
	sinceEnd = 0.f;
	waveActive = true;
}

void GameScreen::update(float seconds) {
	if (minesweeper.hasStarted() && !minesweeper.isOver()) {
		elapsed += seconds;
	}
	if (minesweeper.isOver()) {
		sinceEnd += seconds;
	}
}

icons::Face GameScreen::faceState() const {
	if (minesweeper.isOver()) {
		return minesweeper.isLost() ? icons::Face::Lost : icons::Face::Won;
	}
	return leftDown ? icons::Face::Pressing : icons::Face::Normal;
}

void GameScreen::draw(Canvas& canvas) const {
	const sf::Vector2u size = logicalSize();
	canvas.rect(0, 0, static_cast<float>(size.x), static_cast<float>(size.y), theme::BACKGROUND);
	drawBar(canvas);
	for (int r = 0; r < settings.rows; r++) {
		for (int c = 0; c < settings.columns; c++) {
			drawCell(canvas, r, c);
		}
	}
	if (waveActive) {
		drawWave(canvas);
	}
	if (minesweeper.isOver() && sinceEnd >= waveLength) {
		drawBanner(canvas);
	}
}

void GameScreen::drawBar(Canvas& canvas) const {
	const float width = static_cast<float>(logicalSize().x);
	canvas.rect(0, 0, width, theme::BAR, theme::BAR_BACKGROUND);
	canvas.rect(0, theme::BAR - 1, width, 1, theme::BACKGROUND);

	// Left: mines not yet flagged (can go below zero)
	const long minesLeft =
	    static_cast<long>(minesweeper.mineCount()) - static_cast<long>(minesweeper.flagCount());
	icons::mine(canvas, 4, 4);
	pixelfont::drawShadowed(canvas, to_string(minesLeft), 22, 9, theme::TEXT, theme::SHADOW);

	// Middle: the face, which is also the "new game" button
	const sf::FloatRect area = faceArea();
	const Button faceButton{area, ""};
	const bool pressed = facePressed && faceHovered;
	drawButton(canvas, faceButton, faceHovered, pressed);
	icons::face(canvas, faceState(), area.position.x + 2, area.position.y + 2 + (pressed ? 1.f : 0.f));

	// Right: time in seconds (up to 999)
	const string time = to_string(min(999, static_cast<int>(elapsed)));
	pixelfont::drawShadowed(canvas, time, width - 6 - pixelfont::width(time), 9, theme::TEXT, theme::SHADOW);
}

void GameScreen::drawCell(Canvas& canvas, int row, int column) const {
	const float x = boardX + column * CELL;
	const float y = boardY + row * CELL;
	const float side = CELL - 1.f; // a 1-pixel gap between cells shows the background colour
	const char cell = minesweeper.cell(row, column);
	const bool closed = cell == Minesweeper::HIDDEN || cell == Minesweeper::FLAG;

	if (closed) {
		const bool hovered = hoveredCell && hoveredCell->row == row && hoveredCell->column == column;
		const bool pressed = hovered && leftDown && cell == Minesweeper::HIDDEN;
		sf::Color fill = theme::CLOSED;
		if (!minesweeper.isOver()) {
			if (pressed) {
				fill = theme::PRESSED;
			} else if (hovered) {
				fill = theme::HOVER;
			}
		}
		canvas.rect(x, y, side, side, fill);
		if (!pressed) {
			// A light bevel: bright top and left edge, dark bottom and right edge
			canvas.rect(x, y, side, 1, theme::CLOSED_LIGHT);
			canvas.rect(x, y, 1, side, theme::CLOSED_LIGHT);
			canvas.rect(x, y + side - 1, side, 1, theme::CLOSED_DARK);
			canvas.rect(x + side - 1, y, 1, side, theme::CLOSED_DARK);
		}
		if (cell == Minesweeper::FLAG) {
			icons::flag(canvas, x, y);
		}
		return;
	}

	canvas.rect(x, y, side, side, cell == Minesweeper::EXPLODED ? theme::EXPLOSION : theme::OPENED);
	if (cell >= '1' && cell <= '8') {
		// The number at double size (10x14) in the middle of the cell
		pixelfont::draw(canvas, string(1, cell), x + 3, y + 1, theme::NUMBERS[cell - '0'], 2.f);
	} else if (cell == Minesweeper::MINE || cell == Minesweeper::EXPLODED) {
		icons::mine(canvas, x, y);
	} else if (cell == Minesweeper::WRONG_FLAG) {
		icons::mine(canvas, x, y);
		icons::cross(canvas, x, y);
	}
}

void GameScreen::drawWave(Canvas& canvas) const {
	sf::Color color = minesweeper.isLost() ? theme::WAVE_LOSS : theme::WAVE_WIN;
	for (int r = 0; r < settings.rows; r++) {
		for (int c = 0; c < settings.columns; c++) {
			const float t = sinceEnd - waveDelays[static_cast<size_t>(r) * settings.columns + c];
			if (t < 0.f || t >= WAVE_CELL) {
				continue;
			}
			const float progress = t / WAVE_CELL;
			// A square that grows from the middle of the cell while fading out
			const float size = 2.f * round(progress * (CELL - 1.f) / 2.f);
			color.a = static_cast<uint8_t>(230.f * (1.f - progress));
			const float x = boardX + c * CELL + floor((CELL - 1.f - size) / 2.f);
			const float y = boardY + r * CELL + floor((CELL - 1.f - size) / 2.f);
			canvas.rect(x, y, size, size, color);
		}
	}
}

void GameScreen::drawBanner(Canvas& canvas) const {
	const sf::Vector2u size = logicalSize();
	const float centerX = floor(size.x / 2.f);
	const float y = floor(boardY + (settings.rows * CELL - BANNER_HEIGHT) / 2.f);
	canvas.rect(0, y, static_cast<float>(size.x), BANNER_HEIGHT, theme::BANNER);
	const bool lost = minesweeper.isLost();
	pixelfont::drawCentered(canvas, lost ? "MÄNG LÄBI" : "VÕIT!", centerX, y + 8,
	                        lost ? theme::WAVE_LOSS : theme::TEXT, 2.f);
	pixelfont::drawCentered(canvas, "ENTER - UUESTI", centerX, y + 31, theme::TEXT);
	pixelfont::drawCentered(canvas, "ESC - MENÜÜ", centerX, y + 43, theme::TEXT_DIM);
}
