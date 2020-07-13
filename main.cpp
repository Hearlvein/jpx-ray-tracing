#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>

#define T std::cerr << __LINE__ << std::endl;

const int width = 100;
const int height = 100;
const float PI = 3.1415f;
float px_sz = 5.f;
sf::Color pixels[width][height];
sf::VertexArray vertices(sf::Quads, width * height * 4);
sf::RenderWindow window(sf::VideoMode(width * px_sz, height * px_sz), "jpx");

float norm(const sf::Vector2i& vec)
{
	return std::sqrt((vec.x * vec.x) + (vec.y * vec.y));
}

float norm(const sf::Vector2f& vec)
{
	return std::sqrt((vec.x * vec.x) + (vec.y * vec.y));
}

float dist(const sf::Vector2f& a, const sf::Vector2f& b)
{
	return norm(b - a);
}

const sf::Color& px_clr(int i, int j)
{
	return pixels[i][j];
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

void px_clr(int i, int j, const sf::Color& newColor)
{
	pixels[i][j] = newColor;
	for (int k = 0; k < 4; ++k)
		vertices[(j * width + i) * 4 + k].color = newColor;
}

void px_cast_ray(int x, int y, float angle)
{
	angle = radius(angle);
	sf::Vector2f src(x, y);
	sf::Vector2f pt(src);
	sf::Vector2f step = sf::Vector2f(std::cos(angle), std::sin(angle));

	while (true)
	{
	    pt += step;

	    if (pt.x < 0 || pt.y < 0 || pt.x >= width || pt.y >= height)
            break;

        float d = dist(pt, src);
        if (d > 200.f)
        	break;

	    float a = d * 0.0005f;

	    sf::Color clr = px_clr(pt.x, pt.y);
	    float c = 1 / a;
	    if (c > 255) c = 255;
		clr = sf::Color(c, c, c);

		px_clr(pt.x, pt.y, clr);
	}
}

void px_uniform()
{
    for (int i = 0; i < width; ++i)
    {
        for (int j = 0; j < height; ++j)
        {
            if (px_clr(i, j) == sf::Color::Black)
            {
                std::vector<sf::Vector2i> neighbors;
                if (i > 0) 			neighbors.push_back({i-1, j});
                if (j > 0) 			neighbors.push_back({i, j-1});
                if (i+1 < width)	neighbors.push_back({i+1, j});
                if (j+1 < height)	neighbors.push_back({i, j+1});

                sf::Uint8 r = 0;
                sf::Uint8 g = 0;
                sf::Uint8 b = 0;
                for (const sf::Vector2i& v : neighbors)
                {
                	const sf::Color& neighClr = px_clr(v.x, v.y);
                	r += neighClr.r;
                	g += neighClr.g;
                	b += neighClr.b;
                }

                r /= neighbors.size();
                g /= neighbors.size();
                b /= neighbors.size();

                if (r > 255) r = 255;
                if (g > 255) g = 255;
                if (b > 255) b = 255;

                px_clr(i, j, { r, g, b });
            }
        }
    }
}

void px_cast_light(int x, int y)
{
    px_clr(x, y, sf::Color::White);

	// px_cast_ray(x, y, 0.f);
	for (int k = 0; k < 360; ++k)
		px_cast_ray(x, y, k);

    px_uniform();
}

int main()
{
	for (int i = 0; i < width; ++i)
	{
		for (int j = 0; j < height; ++j)
		{
		    px_pos(i, j, i, j);
			px_clr(i, j, sf::Color::Black);
		}
	}

	sf::RectangleShape ex({px_sz, px_sz});
	ex.setFillColor(sf::Color::Yellow);

	sf::Vector2i lightSrc(width / 2, height / 2);

	while (window.isOpen())
	{
		sf::Event e;
		while (window.pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
				window.close();

            else if (e.type == sf::Event::KeyPressed)
            {
                if 		(e.key.code == sf::Keyboard::Left)	lightSrc.x--;
                else if (e.key.code == sf::Keyboard::Up)	lightSrc.y--;
                else if (e.key.code == sf::Keyboard::Right) lightSrc.x++;
                else if (e.key.code == sf::Keyboard::Down)	lightSrc.y++;
            }
		}

		px_cast_light(lightSrc.x, lightSrc.y);

		window.clear();
		window.draw(vertices);
		//window.draw(ex);
		window.display();
	}

	return 0;
}
