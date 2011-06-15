#pragma once
#include "Particle.h"
#include<vector>

using namespace std;
using namespace kd;
namespace kd{
	
	class Emitter : public sf::Drawable{
	private:
		vector<Particle> _particles;
		float _range;
	public:
		Emitter(float range) : _range(range){};
		Emitter& Begin();
		Emitter& Stop();
		Emitter& Resume();
		Emitter& Update(float elapsed);
		
		virtual void Render(sf::RenderTarget& target) const;
	};
}