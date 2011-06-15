#include "Emitter.h"
#include<cstdlib>
#include<cmath>

Emitter::Emitter(int minAngle, int maxAngle, int radius, float lifetime, sf::Vector2f position) : _lifetime(lifetime), _minAngle(minAngle), _maxAngle(maxAngle), _radius(radius), _active(false){
	this->SetPosition(position);
	srand(time(NULL));
}

Emitter& Emitter::Begin(){
	_active = true;
	return *this;
}
Emitter& Emitter::Stop(){
	_active = false;
	return *this;
}
Emitter& Emitter::Resume(){
	_active = true;
	return *this;
}

bool Emitter::IsActive(){
	return _active;
}

Emitter& Emitter::Update(float elapsed){
	if(_active){
		if(_particles.size() == 0){
			for(int i = 0; i < 50; i++){
				Particle p;
				p.SetPosition(0, 0);
				float angle = RADIANS * (rand() % (_maxAngle - _minAngle) + _minAngle);
				int velocity = rand() % _radius;
				p.SetPositionDelta(sf::Vector2f(velocity * cos(angle), velocity * sin(angle)));
				p.SetFinalColor(sf::Color(255, 255, 255, 0));
				p.SetLifetime(_lifetime);
				p.SetCenter(0, 0);
				_particles.push_back(p);
			  }
		}
		vector<Particle>::iterator it;
		for(it = _particles.begin(); it < _particles.end(); it++){
			if(it->GetLifetime() <= it->GetAge()){
				_particles.erase(it);
			}
			else{
				it->Update(elapsed);
			}
		}
		
		if(_particles.size() == 0)
			this->_active = false;
	}
	return *this;
}

void Emitter::Render(sf::RenderTarget& target) const{
	
	vector<Particle>::const_iterator it;
	for(it = _particles.begin(); it < _particles.end(); it++){
		target.Draw(*it);
	}
}