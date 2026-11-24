#include "gui/app.hpp"

#include "gui/icons.hpp"
#include "gui/theme.hpp"

#include <algorithm>
#include <optional>

using namespace std;

namespace {

// The largest whole-number scale at which the screen fits on the desktop (1 to 4)
unsigned scaleFor(sf::Vector2u logical) {
	const sf::Vector2u desktop = sf::VideoMode::getDesktopMode().size;
	const unsigned fitX = static_cast<unsigned>(desktop.x * 0.92f) / max(1u, logical.x);
	const unsigned fitY = static_cast<unsigned>(desktop.y * 0.85f) / max(1u, logical.y);
	return clamp(min(fitX, fitY), 1u, static_cast<unsigned>(theme::MAX_SCALE));
}

} // namespace

int App::run() {
	const sf::Vector2u startSize = menu.logicalSize();
	const unsigned scale = scaleFor(startSize);
	window.create(sf::VideoMode({startSize.x * scale, startSize.y * scale}), "Minesweeper",
	              sf::Style::Titlebar | sf::Style::Close);
	window.setFramerateLimit(60);
	window.setKeyRepeatEnabled(false);
	const sf::Image icon = icons::faceImage(icons::Face::Normal, 2);
	window.setIcon(icon);
	show(menu);

	sf::Clock clock;
	while (window.isOpen()) {
		while (const optional event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
				break;
			}
			// Mouse position in logical pixels (each event carries its own position)
			sf::Vector2f mouse{-1.f, -1.f};
			if (const auto* moved = event->getIf<sf::Event::MouseMoved>()) {
				mouse = toLogical(moved->position);
			} else if (const auto* pressed = event->getIf<sf::Event::MouseButtonPressed>()) {
				mouse = toLogical(pressed->position);
			} else if (const auto* released = event->getIf<sf::Event::MouseButtonReleased>()) {
				mouse = toLogical(released->position);
			}
			current->handleEvent(*event, mouse);
		}
		if (!window.isOpen()) {
			break;
		}

		// Switching screens
		if (current == &menu) {
			if (menu.wantsQuit()) {
				window.close();
				break;
			}
			if (const auto settings = menu.takeStartRequest()) {
				game.start(*settings);
				show(game);
			}
		} else if (game.wantsMenu()) {
			show(menu);
		}

		current->update(clock.restart().asSeconds());

		window.clear(theme::BACKGROUND);
		canvas.clear();
		current->draw(canvas);
		canvas.draw(window);
		window.display();
	}
	return 0;
}

// Resizes the window for the screen and centres it on the desktop
void App::show(Screen& screen) {
	current = &screen;
	const sf::Vector2u logical = screen.logicalSize();
	const unsigned scale = scaleFor(logical);
	const sf::Vector2u size{logical.x * scale, logical.y * scale};
	window.setSize(size);
	window.setView(
	    sf::View(sf::FloatRect({0.f, 0.f}, {static_cast<float>(logical.x), static_cast<float>(logical.y)})));
	const sf::Vector2u desktop = sf::VideoMode::getDesktopMode().size;
	window.setPosition({(static_cast<int>(desktop.x) - static_cast<int>(size.x)) / 2,
	                    (static_cast<int>(desktop.y) - static_cast<int>(size.y)) / 2});
	// The previous screen's mouse position no longer applies: tell the new screen the mouse left
	screen.handleEvent(sf::Event::MouseLeft{}, {-1.f, -1.f});
}

sf::Vector2f App::toLogical(sf::Vector2i pixel) const {
	return window.mapPixelToCoords(pixel);
}
