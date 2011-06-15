#pragma once
#include "Bezier.h"
#include<SFML/Graphics.hpp>

using namespace std;
using namespace kd;
namespace kd{
	class Particle : public sf::Drawable{
	public:
		sf::Color deltaColor;
		sf::Vector2f deltaPosition;
		float lifetime;
		
		Particle();
		
		Particle& SetDeltaPosition(sf::Vector2f delta);
		Particle& SetDeltaColor(sf::Color delta);
		
		Particle& SetLifetime(float nLife);
		virtual void Render(sf::RenderTarget& target) const;
	};
}