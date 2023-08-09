#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <random>
#include <algorithm>
#include <vector>
#include <random>


using namespace sf;
using namespace std;

#define PI 3.14159265358979323846264338

sf::Color getColor(sf::Time elapsed);


class GravitySource
{
	sf::Vector2f pos;
	float strength;
	// you can add more variables here if you want to customize your gravity source

public:
	GravitySource(float pos_x, float pos_y, float strength)
	{
		pos.x = pos_x;
		pos.y = pos_y;
		this->strength = strength;
	}

	sf::Vector2f getPos()
	{
		return pos;
	}

	void setPos(sf::Vector2f pos)
	{
		this->pos = pos;
	}

	float getStrength()
	{
		return strength;
	}

	void setStrength(float strength)
	{
		this->strength = strength;
	}
};

// vertex array version of particle
class ParticleSystem : public sf::Drawable, public sf::Transformable
{
	sf::VertexArray		va;
	sf::VertexBuffer	vb;

public:
	ParticleSystem() : va(sf::Points, 0), m_particles(0), vb(sf::Points, sf::VertexBuffer::Stream)
	{
	}

	ParticleSystem(size_t count) : va(sf::Points, 1), m_particles(1), vb(sf::Points, sf::VertexBuffer::Dynamic)
	{
		vb.create(count);
	}

	void render(sf::RenderWindow& window)
	{
		if (vb.isAvailable())
		{
			vb.update(&va[0], va.getVertexCount(), 0);
			window.draw(vb, sf::BlendAdd);
		}

	}

	void update_physics(GravitySource& s)
	{
		for (int i = 0; i < m_particles.size(); i++)
		{
			Particle& p = m_particles[i];

			float distance_x = s.getPos().x - p.pos.x;
			float distance_y = s.getPos().y - p.pos.y;

			float distance = sqrt(distance_x * distance_x + distance_y * distance_y);

			float inverse_distance = 1.0f / distance;

			float normalized_x = distance_x * inverse_distance;
			float normalized_y = distance_y * inverse_distance;

			float inverse_square_dropoff = inverse_distance * inverse_distance;

			float acceleration_x = normalized_x * s.getStrength() * inverse_distance / 3;
			float acceleration_y = normalized_y * s.getStrength() * inverse_distance / 3;

			p.vel.x += acceleration_x;
			p.vel.y += acceleration_y;
			p.vel.x -= p.vel.x * 0.01f;
			p.vel.y -= p.vel.y * 0.01f;

			p.pos.x += p.vel.x;
			p.pos.y += p.vel.y;

			va[i].position = p.pos;
		}
	}

	// emit particles at random position and velocity within range
	void emit(sf::Vector2f pos, sf::Time elapsed, size_t limit)
	{
		if (va.getVertexCount() < limit)
		{
			for (int i = 0; i < 1000; i++)
			{
				Particle p;
				float angle = (std::rand() % 360) * 3.14f / 180.f;
				float speed = 2.0f;

				p.vel = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
				p.color = getColor(elapsed);
				
				// randomize position in circle
				p.pos = getRandomPointInsideCircle(pos, 100);
				

				sf::Vertex v;
				v.position = p.pos;
				v.color = p.color;
				va.append(v);
				m_particles.push_back(p);
			}
		}

	}

	void setColor(sf::Color color)
	{
		for(int i=0 ; i<va.getVertexCount() ; i++)
			va[i].color = color;
	}

	void setPosition(sf::Vector2f pos)
	{
		for (int i = 0; i < va.getVertexCount(); i++)
			va[i].position = pos;
	}

	size_t getParticleCount() const
	{
		return va.getVertexCount();
	}

	void clear()
	{
		va.clear();
		m_particles.clear();
	}


private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		// transform 적용
		states.transform *= getTransform();

		// our particles don't use a texture
		states.texture = NULL;

		// vertex array 그리기
		target.draw(va, states);
	}

private:
	struct Particle
	{
		sf::Vector2f pos;
		sf::Vector2f vel;
		sf::Color color;
	};

	sf::Color getColor(sf::Time elapsed)
	{
		float red = (std::sin(elapsed.asSeconds()) + 1.0f) / 2.0f;  // 0 ~ 1 사이의 값
		float green = (std::sin(elapsed.asSeconds() + 4.0f * PI / 3.0f) + 1.0f) / 2.0f;  // 0 ~ 1 사이의 값
		//float blue = (std::sin(elapsed.asSeconds() + 4.0f * PI / 3.0f) + 1.0f) / 2.0f;  // 0 ~ 1 사이의 값

		// 각 색상 값을 정수로 변환하여 0 ~ 255 사이의 값으로 조정합니다.
		int r = static_cast<int>(red * 255.0f);
		int g = static_cast<int>(green * 255.0f);
		//int b = static_cast<int>(blue * 255.0f);
		//int a = static_cast<int>(127);

		sf::Color color;

		color.a = 50;
		color.r = r;
		color.g = g;
		color.b = 255;
		

		return color;
	}

	sf::Vector2f getRandomPointInsideCircle(const sf::Vector2f& center, float radius) {
		// 랜덤한 각도를 구합니다 (0~2*pi 범위)
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dist(0, 2 * PI);
		float angle = dist(gen);

		// 랜덤한 거리를 구합니다 (0~radius 범위)
		std::uniform_real_distribution<float> dist_radius(0, radius);
		float distance = dist_radius(gen);

		// 랜덤한 위치를 계산하여 반환합니다
		sf::Vector2f random_point;
		random_point.x = center.x + distance * std::cos(angle);
		random_point.y = center.y + distance * std::sin(angle);

		return random_point;
	}
	vector<Particle> m_particles;
};


int main(void)
{

	sf::RenderWindow window(sf::VideoMode(1900, 1000), "Particles");
	//window.setFramerateLimit(60);

	// 경과 시간을 측정할 clock 생성
	sf::Clock clock;
	sf::Time elapsed;

	GravitySource source(0, 0, 0);

	// 마우스 위치
	sf::Mouse mouse;
	sf::Vector2i mousePosition;

	const size_t particleLimit = 3000000;
	ParticleSystem p(particleLimit);

	while (window.isOpen())
	{
		// 이벤트 핸들링
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
				window.close();
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
				p.clear();
		}

		window.clear();
		// mousePosition to Vector2f
		mousePosition = sf::Mouse::getPosition(window);
		source.setPos(window.mapPixelToCoords(mousePosition));

		if (mouse.isButtonPressed(sf::Mouse::Left))
		{
			p.emit(window.mapPixelToCoords(mousePosition), clock.getElapsedTime(), particleLimit);
		}
		if (mouse.isButtonPressed(sf::Mouse::Right))
		{
			source.setStrength(500);
		}
		else
		{
			source.setStrength(0);
		}

		p.update_physics(source);
		p.render(window);

		window.display();
	}
	return 0;
}
