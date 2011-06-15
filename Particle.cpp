#include "Particle.h"

Particle::Particle(){
	deltaColor = sf::Color(0, 0, 0);
	deltaPosition = sf::Vector2f(1, 1);
}

Particle& Particle::SetDeltaPosition(sf::Vector2f delta){
	deltaPosition = delta;
	return *this;
}
Particle& Particle::SetDeltaColor(sf::Color delta){
	deltaColor = delta;
	return *this;
}

Particle& Particle::SetLifetime(float nLife){
	lifetime = nLife;
	return *this;
}

void Particle::Render(sf::RenderTarget& target) const{
	sf::Color color = this->GetColor();
	sf::Vector2f position = this->GetPosition() - target.GetDefaultView().GetCenter();
	glBegin(GL_QUADS);
	glColor4f(color.r/255.f, color.g/255.f, color.b/255.f, color.a/255.f);
	glVertex2f(position.x-1, position.y-1);
	glVertex2f(position.x+1, position.y-1);
	glVertex2f(position.x+1, position.y+1);
	glVertex2f(position.x-1, position.y+1);
	glEnd();
}