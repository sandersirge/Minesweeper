#include "gui/game_screen.hpp"

#include "gui/pixel_font.hpp"
#include "gui/theme.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <queue>
#include <string>
#include <utility>

using namespace std;

namespace {

constexpr float CELL = static_cast<float>(theme::CELL);
// The end-of-game wave has three phases:
//   fill   - the covering spreads from the clicked cell until the whole board is covered
//   banner - the result banner appears on the fully covered board
//   decay  - after a pause the covering recedes, spreading the same way but more slowly
// Moving to a neighbouring cell takes a random time between MIN and MAX for every step.
constexpr float FILL_STEP_MIN = 0.02f;
constexpr float FILL_STEP_MAX = 0.12f;
constexpr float DECAY_STEP_MIN = 0.12f;
constexpr float DECAY_STEP_MAX = 0.50f;
constexpr float FILL_BURST = 0.25f; // a cell's square grows to full size, its hot centre cools down
constexpr float DECAY_PAUSE = 0.6f; // time between the banner appearing and the decay starting
constexpr float DECAY_CELL = 0.70f; // a cell's square shrinks and fades out
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
	fillDelays.clear();
	decayDelays.clear();
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

// When the game ends a wave covers the board (red on a loss, white on a win), the banner appears,
// then the covering recedes. Both the fill and the decay spread like fire from the clicked cell:
// every step from a cell to its neighbour (up, down, left, right) takes a random time, and a cell
// is reached when the fastest path from the clicked cell gets there (Dijkstra's shortest paths over
// those random steps). Some directions race ahead while others lag, so the front is ragged instead
// of a smooth ring.
vector<float> GameScreen::spreadTimes(const CellPosition& origin, float stepMin, float stepMax) {
	const int rows = settings.rows;
	const int columns = settings.columns;
	vector<float> times(static_cast<size_t>(rows) * columns, numeric_limits<float>::infinity());

	uniform_real_distribution<float> stepTime(stepMin, stepMax);
	using Arrival = pair<float, size_t>; // (time, cell index)
	priority_queue<Arrival, vector<Arrival>, greater<Arrival>> pending;
	const size_t start = static_cast<size_t>(origin.row) * columns + origin.column;
	times[start] = 0.f;
	pending.push({0.f, start});

	constexpr int ROW_STEP[4] = {-1, 1, 0, 0};
	constexpr int COLUMN_STEP[4] = {0, 0, -1, 1};
	while (!pending.empty()) {
		const auto [time, index] = pending.top();
		pending.pop();
		if (time > times[index]) {
			continue; // a faster path already reached this cell
		}
		const int row = static_cast<int>(index / columns);
		const int column = static_cast<int>(index % columns);
		for (int k = 0; k < 4; k++) {
			const int r = row + ROW_STEP[k];
			const int c = column + COLUMN_STEP[k];
			if (r < 0 || r >= rows || c < 0 || c >= columns) {
				continue;
			}
			const size_t next = static_cast<size_t>(r) * columns + c;
			const float arrival = time + stepTime(rng);
			if (arrival < times[next]) {
				times[next] = arrival;
				pending.push({arrival, next});
			}
		}
	}
	return times;
}

void GameScreen::startWave(const CellPosition& origin) {
	fillDelays = spreadTimes(origin, FILL_STEP_MIN, FILL_STEP_MAX);
	decayDelays = spreadTimes(origin, DECAY_STEP_MIN, DECAY_STEP_MAX);
	// The banner waits until the last cell's square has grown to full size
	bannerTime = *max_element(fillDelays.begin(), fillDelays.end()) + FILL_BURST;
	decayStart = bannerTime + DECAY_PAUSE;
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
	if (minesweeper.isOver() && sinceEnd >= bannerTime) {
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
	const bool lost = minesweeper.isLost();
	sf::Color color = lost ? theme::WAVE_LOSS : theme::WAVE_WIN;
	sf::Color core = lost ? theme::WAVE_LOSS_CORE : theme::WAVE_WIN_CORE;
	for (int r = 0; r < settings.rows; r++) {
		for (int c = 0; c < settings.columns; c++) {
			const size_t index = static_cast<size_t>(r) * settings.columns + c;
			const float sinceFill = sinceEnd - fillDelays[index];
			if (sinceFill < 0.f) {
				continue; // the fill has not reached this cell yet
			}
			// Fill: a square bursts from the middle of the cell to the full cell (covering the grid
			// lines) with a hot centre that cools down, then the cell stays covered.
			const float growth = min(1.f, sinceFill / FILL_BURST);
			float size = 2.f * round(growth * CELL / 2.f);
			float fade = 1.f;
			// Decay: once the decay reaches the cell, the square shrinks and fades out
			const float sinceDecay = sinceEnd - decayStart - decayDelays[index];
			if (sinceDecay >= DECAY_CELL) {
				continue;
			}
			if (sinceDecay >= 0.f) {
				const float left = 1.f - sinceDecay / DECAY_CELL;
				size = 2.f * round(left * CELL / 2.f);
				fade = left;
			}
			color.a = static_cast<uint8_t>(255.f * fade);
			const float x = boardX + c * CELL + floor((CELL - 1.f - size) / 2.f);
			const float y = boardY + r * CELL + floor((CELL - 1.f - size) / 2.f);
			canvas.rect(x, y, size, size, color);
			// A smaller, brighter centre makes a new square look hot (on a win both colours are white)
			core.a = static_cast<uint8_t>(255.f * fade * (1.f - growth));
			if (core.a > 0) {
				const float coreSize = 2.f * round(size / 4.f);
				const float coreOffset = (size - coreSize) / 2.f;
				canvas.rect(x + coreOffset, y + coreOffset, coreSize, coreSize, core);
			}
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
