#ifndef GAME_HPP
#define GAME_HPP

#include <SDL2/SDL.h>

#include <vector>
#include <array>
#include <cmath>
#include <tuple>
#include <type_traits>

struct Line
{
	SDL_FPoint start_point;
	SDL_FPoint end_point;
};

struct PlayerBox
{
	SDL_Rect box_;
	SDL_Color color_;
	int vx_;
	int vy_;
};

class Game
{
private:
	bool initialized_;
	bool running_;
	int cell_size_;
	int cells_width_;
	int cells_height_;
	
	bool shift_pressed_;

	std::array<PlayerBox, 4> player_boxes_;
	SDL_FPoint lines_intersection_;

	SDL_Window* window_;
	SDL_Renderer* renderer_;

public:
	Game();

	~Game();

	bool Initialize();

	void Finalize();

	void Run();

	void HandleEvents();
	
	void Tick();

	void Render();

	void RenderGrid();

	const SDL_FPoint GetIntersectionPoint(const Line& line_1, const Line& line_2) const;
};

#endif