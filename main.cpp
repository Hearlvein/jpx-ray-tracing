#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>

#define T std::cerr << __LINE__ << std::endl;

struct px_t
{
	float intensity = 0.f;	// scales from 0 to 1
	bool solid = false;
	sf::Color color = sf::Color::Black;

	void reset()
	{
		intensity = 0.f;
		solid = false;
		color = sf::Color::Black;
	}
};

const int width = 100;
const int height = 100;
const float ray_length = 30.f;
const float PI = 3.14159265f;
int px_smooth_level = 2;
float px_sz = 5.f;
bool px_updated = false;
px_t pixels[width][height];
sf::VertexArray vertices(sf::Quads, width * height * 4);
sf::RenderWindow window(sf::VideoMode(width * px_sz, height * px_sz), "jpx");

float norm(const sf::Vector2f& vec)
{
	return std::sqrt((vec.x * vec.x) + (vec.y * vec.y));
}

float dist(const sf::Vector2f& a, const sf::Vector2f& b)
{
	return norm(b - a);
}

float radius(float a)
{
	return PI / 180 * a;
}

void px_pos(int i, int j, int x, int y)
{
	vertices[(j * width + i) * 4 + 0].position = sf::Vector2f(x, y) * px_sz;
	vertices[(j * width + i) * 4 + 1].position = sf::Vector2f(x+1, y) * px_sz;
	vertices[(j * width + i) * 4 + 2].position = sf::Vector2f(x+1, y+1) * px_sz;
	vertices[(j * width + i) * 4 + 3].position = sf::Vector2f(x, y+1) * px_sz;
}

void px_intensity(int x, int y, float mod)
{
	float& i = pixels[x][y].intensity;
	i = mod;

	if 		(i > 1.f)	i = 1.f;
	else if (i < 0.f)	i = 0.f;
}

void px_clr(int i, int j, const sf::Color& newColor)
{
	pixels[i][j].color = newColor;
	for (int k = 0; k < 4; ++k)
		vertices[(j * width + i) * 4 + k].color = newColor;
}

void px_colorize()
{
	for (int x = 0; x < width; ++x)
	{
		for (int y = 0; y < height; ++y)
		{
			if (!pixels[x][y].solid)
			{
				sf::Uint8 c = pixels[x][y].intensity * 255.f;
				px_clr(x, y, { c, c, c });
			}
			else
				px_clr(x, y, sf::Color::Red);
		}
	}
}

void px_cast_ray(int from_x, int from_y, float angle)
{
	angle = radius(angle);
	sf::Vector2f src((float)from_x, (float)from_y);
	sf::Vector2f pt(src);
	sf::Vector2f step = sf::Vector2f(std::cos(angle), std::sin(angle));

	while (true)
	{
		int x = std::round(pt.x);
		int y = std::round(pt.y);

		if (x < 0 || y < 0 || x >= width || y >= height || pixels[x][y].solid)
			break;

		float d = dist(pt, src);
		if (d >= ray_length)
			break;

		float mod = (ray_length - d) / ray_length;

		// px_intensity(x, y, mod);
		px_intensity(x, y, pixels[x][y].intensity + mod);

		pt += step;
	}
}

void px_smooth()
{
	for (int k = 0; k < px_smooth_level; ++k)
	{
		for (int x = 0; x < width; ++x)
		{
			for (int y = 0; y < height; ++y)
			{
				// if (pixels[x][y].intensity == 0.f)
				// {
					float s = 0.f;
					int c = 0;

					if (x > 0) 							{ s += pixels[x-1][y].intensity; ++c; }
					if (y > 0) 							{ s += pixels[x][y-1].intensity; ++c; }
					if (x+1 < width)					{ s += pixels[x+1][y].intensity; ++c; }
					if (y+1 < height)					{ s += pixels[x][y+1].intensity; ++c; }

					if (x > 0 && y > 0)					{ s += pixels[x-1][y-1].intensity; ++c; }
					if (x > 0 && y+1 < height)			{ s += pixels[x-1][y+1].intensity; ++c; }
					if (x+1 < width && y > 0)			{ s += pixels[x+1][y-1].intensity; ++c; }
					if (x+1 < width && y+1 < height)	{ s += pixels[x+1][y+1].intensity; ++c; }

					px_intensity(x, y, s / (float)c);	// the frame implies c != 0
				// }
			}
		}
	}
}

void px_cast_light(int x, int y)
{
	int nb_rays = 180;
	for (int k = 0; k < nb_rays; ++k)
		px_cast_ray(x, y, (float)k * 360.f / (float)nb_rays);
}

void px_solid(int x, int y, int w, int h)
{
	for (int i = x; i < x+w; ++i)
	{
		for (int j = y; j < y+h; ++j)
		{
			pixels[i][j].solid = true;
			px_intensity(i, j, 0.f);
		}
	}
}

void px_reset()
{
	for (int i = 0; i < width; ++i)
	{
		for (int j = 0; j < height; ++j)
			pixels[i][j].reset();
	}
}

int main()
{
	for (int i = 0; i < width; ++i)
	{
		for (int j = 0; j < height; ++j)
			px_pos(i, j, i, j);
	}

	int sx = 70;
	int sy = 70;
	int ss = 10;
	sf::Vector2i lightSrc(width / 2, height / 2);

	sf::Clock timer;

	while (window.isOpen())
	{
		sf::Event e;
		while (window.pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
				window.close();

			else if (e.type == sf::Event::KeyPressed)
			{
				bool modified = true;
				switch (e.key.code)
				{
				case sf::Keyboard::Left:	sx--; break;
				case sf::Keyboard::Up:		sy--; break;
				case sf::Keyboard::Right:	sx++; break;
				case sf::Keyboard::Down:	sy++; break;
				case sf::Keyboard::Z:		lightSrc.y--; break;
				case sf::Keyboard::Q:		lightSrc.x--; break;
				case sf::Keyboard::S:		lightSrc.y++; break;
				case sf::Keyboard::D:		lightSrc.x++; break;
				case sf::Keyboard::P:		if (px_smooth_level > 0)	px_smooth_level--;	break;
				case sf::Keyboard::M:		if (px_smooth_level < 10)	px_smooth_level++;	break;
				default:					modified = false; break;
				}

				if (modified) px_updated = false;
			}
		}

		std::cout << "frame took " << timer.restart().asMilliseconds() << '\n';


		if (!px_updated)
		{
			px_reset();
			px_solid(sx, sy, ss, ss);
			px_cast_light(lightSrc.x, lightSrc.y);
			px_smooth();
			px_colorize();
			px_updated = true;
		}

		window.clear();
		window.draw(vertices);
		window.display();
	}

	return 0;
}
