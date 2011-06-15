#pragma once
#include "Bezier.h"
#include<SFML/Graphics.hpp>

using namespace std;
using namespace kd;
namespace kd{
	class Particle : public sf::Drawable{
	private:
		sf::Color deltaColor, finalColor;
		sf::Vector2f deltaPosition;
		float lifetime, age;
		bool _useFinal;
	public:
		Particle();
		
		Particle& SetLifetime(float nLife);
		float GetLifetime();
		
		float GetAge();
		
		Particle& SetPositionDelta(sf::Vector2f delta);
		sf::Vector2f GetPositionDelta();
		Particle& SetColorDelta(sf::Color delta);
		Particle& SetFinalColor(sf::Color final);
		sf::Color GetColorDelta();
		
		void Update(float elapsed);
		virtual void Render(sf::RenderTarget& target) const;
	};
}