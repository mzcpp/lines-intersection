#include "Game.hpp"
#include "Constants.hpp"
#include "LineEquation.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <cstdint>
#include <iostream>
#include <cmath>
#include <tuple>
#include <type_traits>


Game::Game() : 
	initialized_(false), 
	running_(false), 
	cell_size_(32), 
	cells_width_(constants::screen_width / cell_size_), 
	cells_height_(constants::screen_height / cell_size_), 
	shift_pressed_(false)
{
	initialized_ = Initialize();

	const int box_size = 10;

	for (std::size_t i = 0; i < player_boxes_.size(); ++i)
	{
		player_boxes_[i].box_.w = box_size;
		player_boxes_[i].box_.h = box_size;
		player_boxes_[i].vx_ = 0;
		player_boxes_[i].vy_ = 0;
	}

	player_boxes_[0].box_.x = (constants::screen_width * 1 / 3) - (box_size / 2);
	player_boxes_[0].box_.y = (constants::screen_height * 1 / 3) - (box_size / 2);
	player_boxes_[0].color_ = { 0xFF, 0x00, 0x00, 0xFF };

	player_boxes_[1].box_.x = (constants::screen_width * 2 / 3) - (box_size / 2);
	player_boxes_[1].box_.y = (constants::screen_height * 1 / 3) - (box_size / 2);
	player_boxes_[1].color_ = { 0xA0, 0x00, 0x00, 0xFF };

	player_boxes_[2].box_.x = (constants::screen_width * 1 / 3) - (box_size / 2);
	player_boxes_[2].box_.y = (constants::screen_height * 2 / 3) - (box_size / 2);
	player_boxes_[2].color_ = { 0x00, 0xFF, 0x00, 0xFF };
	
	player_boxes_[3].box_.x = (constants::screen_width * 2 / 3) - (box_size / 2);
	player_boxes_[3].box_.y = (constants::screen_height * 2 / 3) - (box_size / 2);
	player_boxes_[3].color_ = { 0x00, 0xA0, 0x00, 0xFF };

	lines_intersection_ = { -1.0f, -1.0f };
}

Game::~Game()
{
	Finalize();
}

bool Game::Initialize()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not be initialized! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"))
	{
		printf("%s\n", "Warning: Texture filtering is not enabled!");
	}

	window_ = SDL_CreateWindow(constants::game_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, constants::screen_width, constants::screen_height, SDL_WINDOW_SHOWN);

	if (window_ == nullptr)
	{
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);

	if (renderer_ == nullptr)
	{
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	constexpr int img_flags = IMG_INIT_PNG;

	if (!(IMG_Init(img_flags) & img_flags))
	{
		printf("SDL_image could not be initialized! SDL_image Error: %s\n", IMG_GetError());
		return false;
	}

	return true;
}

void Game::Finalize()
{
	SDL_DestroyWindow(window_);
	window_ = nullptr;
	
	SDL_DestroyRenderer(renderer_);
	renderer_ = nullptr;

	SDL_Quit();
	IMG_Quit();
}

void Game::Run()
{
	if (!initialized_)
	{
		return;
	}

	running_ = true;

	constexpr double ms = 1.0 / 60.0;
	std::uint64_t last_time = SDL_GetPerformanceCounter();
	long double delta = 0.0;

	double timer = SDL_GetTicks();

	int frames = 0;
	int ticks = 0;

	while (running_)
	{
		const std::uint64_t now = SDL_GetPerformanceCounter();
		const long double elapsed = static_cast<long double>(now - last_time) / static_cast<long double>(SDL_GetPerformanceFrequency());

		last_time = now;
		delta += elapsed;

		HandleEvents();

		while (delta >= ms)
		{
			Tick();
			delta -= ms;
			++ticks;
		}

		//printf("%Lf\n", delta / ms);
		Render();
		++frames;

		if (SDL_GetTicks() - timer > 1000.0)
		{
			timer += 1000.0;
			//printf("Frames: %d, Ticks: %d\n", frames, ticks);
			frames = 0;
			ticks = 0;
		}
	}
}

void Game::HandleEvents()
{
	SDL_Event e;

	while (SDL_PollEvent(&e) != 0)
	{
		if (e.type == SDL_QUIT)
		{
			running_ = false;
			return;
		}
		
		int speed = 5;

		if (e.type == SDL_KEYDOWN)
		{
			if (e.key.keysym.sym == SDLK_LSHIFT)
			{
				shift_pressed_ = true;
			}

			if (e.key.keysym.sym == SDLK_w)
			{
				player_boxes_[shift_pressed_ ? 1 : 0].vy_ = -speed;
			}

			if (e.key.keysym.sym == SDLK_a)
			{
				player_boxes_[shift_pressed_ ? 1 : 0].vx_ = -speed;
			}

			if (e.key.keysym.sym == SDLK_s)
			{
				player_boxes_[shift_pressed_ ? 1 : 0].vy_ = speed;
			}

			if (e.key.keysym.sym == SDLK_d)
			{
				player_boxes_[shift_pressed_ ? 1 : 0].vx_ = speed;
			}

			if (e.key.keysym.sym == SDLK_UP)
			{
				player_boxes_[shift_pressed_ ? 3 : 2].vy_ = -speed;
			}

			if (e.key.keysym.sym == SDLK_DOWN)
			{
				player_boxes_[shift_pressed_ ? 3 : 2].vy_ = speed;
			}

			if (e.key.keysym.sym == SDLK_LEFT)
			{
				player_boxes_[shift_pressed_ ? 3 : 2].vx_ = -speed;
			}

			if (e.key.keysym.sym == SDLK_RIGHT)
			{
				player_boxes_[shift_pressed_ ? 3 : 2].vx_ = speed;
			}
		}
		else if (e.type == SDL_KEYUP)
		{
			if (e.key.keysym.sym == SDLK_LSHIFT)
			{
				shift_pressed_ = false;
			}

			if (e.key.keysym.sym == SDLK_w)
			{
				player_boxes_[0].vy_ = 0;
				player_boxes_[1].vy_ = 0;
			}

			if (e.key.keysym.sym == SDLK_a)
			{
				player_boxes_[0].vx_ = 0;
				player_boxes_[1].vx_ = 0;
			}

			if (e.key.keysym.sym == SDLK_s)
			{
				player_boxes_[0].vy_ = 0;
				player_boxes_[1].vy_ = 0;
			}

			if (e.key.keysym.sym == SDLK_d)
			{
				player_boxes_[0].vx_ = 0;
				player_boxes_[1].vx_ = 0;
			}

			if (e.key.keysym.sym == SDLK_UP)
			{
				player_boxes_[2].vy_ = 0;
				player_boxes_[3].vy_ = 0;
			}

			if (e.key.keysym.sym == SDLK_DOWN)
			{
				player_boxes_[2].vy_ = 0;
				player_boxes_[3].vy_ = 0;
			}

			if (e.key.keysym.sym == SDLK_LEFT)
			{
				player_boxes_[2].vx_ = 0;
				player_boxes_[3].vx_ = 0;
			}

			if (e.key.keysym.sym == SDLK_RIGHT)
			{
				player_boxes_[2].vx_ = 0;
				player_boxes_[3].vx_ = 0;
			}
		}
	}
}

void Game::Tick()
{
	for (std::size_t i = 0; i < player_boxes_.size(); ++i)
	{
		player_boxes_[i].box_.x += player_boxes_[i].vx_;
		player_boxes_[i].box_.y += player_boxes_[i].vy_;
	}

	Line line_1;
	line_1.start_point.x = player_boxes_[0].box_.x + (player_boxes_[0].box_.w / 2);
	line_1.start_point.y = player_boxes_[0].box_.y + (player_boxes_[0].box_.h / 2);
	line_1.end_point.x = player_boxes_[1].box_.x + (player_boxes_[1].box_.w / 2);
	line_1.end_point.y = player_boxes_[1].box_.y + (player_boxes_[1].box_.h / 2);

	Line line_2;
	line_2.start_point.x = player_boxes_[2].box_.x + (player_boxes_[2].box_.w / 2);
	line_2.start_point.y = player_boxes_[2].box_.y + (player_boxes_[2].box_.h / 2);
	line_2.end_point.x = player_boxes_[3].box_.x + (player_boxes_[3].box_.w / 2);
	line_2.end_point.y = player_boxes_[3].box_.y + (player_boxes_[3].box_.h / 2);

	lines_intersection_ = GetIntersectionPoint(line_1, line_2);

// 	if (lines_intersection_.x != std::numeric_limits<float>::max() && lines_intersection_.y != std::numeric_limits<float>::max())
// 	{
// 		std::cout << lines_intersection_.x << ' ' << lines_intersection_.y << '\n';
// 	}
}

void Game::Render()
{
	SDL_RenderSetViewport(renderer_, NULL);
	SDL_SetRenderDrawColor(renderer_, 0x00, 0x00, 0x00, 0xff);
	SDL_RenderClear(renderer_);

	RenderGrid();

	for (std::size_t i = 0; i < player_boxes_.size(); ++i)
	{
		SDL_SetRenderDrawColor(renderer_, player_boxes_[i].color_.r, player_boxes_[i].color_.g, player_boxes_[i].color_.b, player_boxes_[i].color_.a);
		SDL_RenderFillRect(renderer_, &player_boxes_[i].box_);
	}

	SDL_SetRenderDrawColor(renderer_, 0x00, 0xff, 0xff, 0xff);
	
	SDL_RenderDrawLine(renderer_, 
		player_boxes_[0].box_.x + (player_boxes_[0].box_.w / 2), 
		player_boxes_[0].box_.y + (player_boxes_[0].box_.h / 2), 
		player_boxes_[1].box_.x + (player_boxes_[1].box_.w / 2), 
		player_boxes_[1].box_.y + (player_boxes_[1].box_.h / 2));
	
	SDL_RenderDrawLine(renderer_, 
		player_boxes_[2].box_.x + (player_boxes_[2].box_.w / 2), 
		player_boxes_[2].box_.y + (player_boxes_[2].box_.h / 2), 
		player_boxes_[3].box_.x + (player_boxes_[3].box_.w / 2), 
		player_boxes_[3].box_.y + (player_boxes_[3].box_.h / 2));

	if (lines_intersection_.x != std::numeric_limits<float>::max() && lines_intersection_.y != std::numeric_limits<float>::max())
	{
		constexpr float box_size = 10.0f;
		const SDL_FRect collision_box = { lines_intersection_.x - (box_size / 2), lines_intersection_.y - (box_size / 2), box_size, box_size };
		SDL_SetRenderDrawColor(renderer_, 0xff, 0xff, 0xff, 0xff);
		SDL_RenderDrawRectF(renderer_, &collision_box);
	}

	SDL_RenderPresent(renderer_);
}

void Game::RenderGrid()
{
	SDL_SetRenderDrawColor(renderer_, 0x14, 0x14, 0x14, 0xff);

	for (int y = 1; y < cells_height_; ++y)
	{
		SDL_RenderDrawLine(renderer_, 0, y * cell_size_, constants::screen_width, y * cell_size_);
	}

	for (int x = 1; x < cells_width_; ++x)
	{
		SDL_RenderDrawLine(renderer_, x * cell_size_, 0, x * cell_size_, constants::screen_height);
	}
}

const SDL_FPoint Game::GetIntersectionPoint(const Line& line_1, const Line& line_2) const
{
	SDL_FPoint result_intersection = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
	
	const auto line_1_coefficients = LinearEquationCoefficients(line_1.start_point.x, line_1.start_point.y, line_1.end_point.x, line_1.end_point.y);
    const auto line_2_coefficients = LinearEquationCoefficients(line_2.start_point.x, line_2.start_point.y, line_2.end_point.x, line_2.end_point.y);

    const float& A1 = std::get<0>(line_1_coefficients);
    const float& B1 = std::get<1>(line_1_coefficients);
    const float& C1 = std::get<2>(line_1_coefficients);
   
    const float& A2 = std::get<0>(line_2_coefficients);
    const float& B2 = std::get<1>(line_2_coefficients);
    const float& C2 = std::get<2>(line_2_coefficients);

    const float det = A1 * B2 - A2 * B1;

    if (det == 0)
    {
        //std::cout << "Lines are parallel\n";
    }
    else
    {
        const float x = (B2 * C1 - B1 * C2) / det;
        const float y = (A1 * C2 - A2 * C1) / det;

        // const bool x_on_line_1 = FloatingPointGreaterThan(x, std::min(line_1.start_point.x, line_1.end_point.x)) && FloatingPointGreaterThan(std::max(line_1.start_point.x, line_1.end_point.x), x);
        // const bool y_on_line_1 = FloatingPointGreaterThan(y, std::min(line_1.start_point.y, line_1.end_point.y)) && FloatingPointGreaterThan(std::max(line_1.start_point.y, line_1.end_point.y), y);
        // const bool x_on_line_2 = FloatingPointGreaterThan(x, std::min(line_2.start_point.x, line_2.end_point.x)) && FloatingPointGreaterThan(std::max(line_2.start_point.x, line_2.end_point.x), x);
        // const bool y_on_line_2 = FloatingPointGreaterThan(y, std::min(line_2.start_point.y, line_2.end_point.y)) && FloatingPointGreaterThan(std::max(line_2.start_point.y, line_2.end_point.y), y);

		const bool x_on_line_1 = (FloatingPointLessThan(std::min(line_1.start_point.x, line_1.end_point.x), x) || FloatingPointSame(std::min(line_1.start_point.x, line_1.end_point.x), x)) && 
								(FloatingPointLessThan(x, std::max(line_1.start_point.x, line_1.end_point.x)) || FloatingPointSame(x, std::max(line_1.start_point.x, line_1.end_point.x)));

		const bool y_on_line_1 = (FloatingPointLessThan(std::min(line_1.start_point.y, line_1.end_point.y), y) || FloatingPointSame(std::min(line_1.start_point.y, line_1.end_point.y), y)) && 
								(FloatingPointLessThan(y, std::max(line_1.start_point.y, line_1.end_point.y)) || FloatingPointSame(y, std::max(line_1.start_point.y, line_1.end_point.y)));

		const bool x_on_line_2 = (FloatingPointLessThan(std::min(line_2.start_point.x, line_2.end_point.x), x) || FloatingPointSame(std::min(line_2.start_point.x, line_2.end_point.x), x)) && 
								(FloatingPointLessThan(x, std::max(line_2.start_point.x, line_2.end_point.x)) || FloatingPointSame(x, std::max(line_2.start_point.x, line_2.end_point.x)));

		const bool y_on_line_2 = (FloatingPointLessThan(std::min(line_2.start_point.y, line_2.end_point.y), y) || FloatingPointSame(std::min(line_2.start_point.y, line_2.end_point.y), y)) && 
								(FloatingPointLessThan(y, std::max(line_2.start_point.y, line_2.end_point.y)) || FloatingPointSame(y, std::max(line_2.start_point.y, line_2.end_point.y)));					

        // const bool x_on_line_1 = (std::min(line_1.start_point.x, line_1.end_point.x) <= x) && (x <= std::max(line_1.start_point.x, line_1.end_point.x));
        // const bool y_on_line_1 = (std::min(line_1.start_point.y, line_1.end_point.y) <= y) && (y <= std::max(line_1.start_point.y, line_1.end_point.y));

        // const bool x_on_line_2 = (std::min(line_2.start_point.x, line_2.end_point.x) <= x) && (x <= std::max(line_2.start_point.x, line_2.end_point.x));
        // const bool y_on_line_2 = (std::min(line_2.start_point.y, line_2.end_point.y) <= y) && (y <= std::max(line_2.start_point.y, line_2.end_point.y));

		// std::cout << "--------------------\n";
		// std::cout << line_1.start_point.x << ' ' << line_1.start_point.y << ' ' << line_1.end_point.x << ' ' << line_1.end_point.y << '\n';
		// std::cout << line_2.start_point.x << ' ' << line_2.start_point.y << ' ' << line_2.end_point.x << ' ' << line_2.end_point.y << '\n';
        // std::cout << "(" << x << ", " << y << ")" << '\n';
		// std::cout << "--------------------\n";

        if (x_on_line_1 && y_on_line_1 && x_on_line_2 && y_on_line_2)
        {
			result_intersection.x = x;
			result_intersection.y = y;
            //std::cout << "Point on line!\n";
        }
        else
        {
            //std::cout << "Point NOT on line!\n";
        }
    }

	return result_intersection;
}