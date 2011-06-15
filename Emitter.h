#pragma once
#include "Particle.h"
#include<vector>

using namespace std;
using namespace kd;
namespace kd{
	
	class Emitter : public sf::Drawable{
	private:
		vector<Particle> _particles;
		float _lifetime;
		bool _active;
		int _minAngle, _maxAngle, _radius;
	public:
		Emitter(int minAngle=0, int maxAngle=360, int radius=5, float lifetime=1, sf::Vector2f position=sf::Vector2f(0, 0));
		Emitter& Begin();
		Emitter& Stop();
		Emitter& Resume();
		Emitter& Update(float elapsed);
		bool IsActive();
		
		virtual void Render(sf::RenderTarget& target) const;
	};
}