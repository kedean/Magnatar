#include "Particle.h"

Particle::Particle(){
	deltaColor = sf::Color(0, 0, 0);
	deltaPosition = sf::Vector2f(1, 1);
	age = 0;
	lifetime = 1; //default lifetime of one second
	_useFinal = false;
}

Particle& Particle::SetPositionDelta(sf::Vector2f delta){
	deltaPosition = delta;
	return *this;
}
sf::Vector2f Particle::GetPositionDelta(){
	return deltaPosition;
}
Particle& Particle::SetFinalColor(sf::Color final){
	finalColor = final;
	_useFinal = true;
}
Particle& Particle::SetColorDelta(sf::Color delta){
	deltaColor = delta;
	return *this;
}
sf::Color Particle::GetColorDelta(){
	return deltaColor;
}

Particle& Particle::SetLifetime(float nLife){
	lifetime = nLife;
	return *this;
}
float Particle::GetLifetime(){
	return lifetime;
}
float Particle::GetAge(){
	return age;
}

void Particle::Update(float elapsed){
	this->SetPosition(this->GetPosition() + sf::Vector2f(elapsed * deltaPosition.x, elapsed * deltaPosition.y));
	sf::Color color = this->GetColor();
	if(!_useFinal)
		this->SetColor(color + sf::Color(elapsed * deltaColor.r, elapsed*deltaColor.g, elapsed*deltaColor.b, elapsed*deltaColor.a));
	else{
		this->SetColor(sf::Color(color.r + (elapsed*(finalColor.r - color.r) / lifetime),
							   color.b + (elapsed*(finalColor.g - color.g) / lifetime),
							   color.g + (elapsed*(finalColor.b - color.b) / lifetime), 
							   color.a + (elapsed*(finalColor.a - color.a) / lifetime)));
	}

	age += elapsed;
}

void Particle::Render(sf::RenderTarget& target) const{
	sf::Color color = this->GetColor();
	sf::Vector2f position = TransformToLocal(this->GetPosition());
	glBegin(GL_QUADS);
	glColor4f(color.r/255.f, color.g/255.f, color.b/255.f, color.a/255.f);
	glVertex2f(position.x-1, position.y-1);
	glVertex2f(position.x+1, position.y-1);
	glVertex2f(position.x+1, position.y+1);
	glVertex2f(position.x-1, position.y+1);
	glEnd();
}