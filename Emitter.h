#pragma once
#include "Particle.h"
#include<vector>

using namespace std;
using namespace kd;
namespace kd{
	
	class Emitter : public sf::Drawable{
	private:
		vector<Particle> _particles; //particles emitted that are alive and being controlled
		float _lifetime, _elapsedLastBirth;
		bool _active; //true when particles are being emitted and affected
		bool _continuous; //when true, the emitter will create more particles every .2 seconds, otherwise it dies after the initial burst
		int _minAngle, _maxAngle; //bounding angles for the particle directions, in degrees
		int _radius; //maximum distance a particle can travel per second
		sf::Color _color;
	public:
		Emitter(bool continuous=true, int minAngle=0, int maxAngle=360, int radius=5, float lifetime=1, sf::Vector2f position=sf::Vector2f(0, 0), sf::Color color=sf::Color(255, 255, 255));
		
		Emitter& Begin();
		Emitter& Stop();
		Emitter& Resume();
		Emitter& Reset();
		
		Emitter& Update(float elapsed);
		bool IsActive();
		
		virtual void Render(sf::RenderTarget& target) const;
	};
}