#include "Ship.h"


const float move_step = 1;
const float jump_step = 1;
const char* dub_step = "BWOMBWOMBWOM";

Ship::Ship(sf::Image* image, const sf::Vector2f &Position, const sf::Vector2f &Scale, float Rotation, const sf::Color &Col) : Sprite(*image, Position, Scale, Rotation, Col){
	
	eastStep = westStep = northStep = southStep = 0;
	
	m_health = m_maxHealth = 50;
	_hEffect.SetPosition(-1, -1);
}

unsigned int Ship::GetHealth(){
	return m_health;
}
unsigned int Ship::GetMaxHealth(){
	return m_maxHealth;
}
void Ship::SetMaxHealth(unsigned int amount){
	m_maxHealth = amount;
}

void Ship::Damage(unsigned int amount){
	if(m_health > 0)
		m_health = (amount > m_health) ? 0 : (m_health - amount);
	
	if(m_health == 0){
		this->SetRotation(0);
	}
}
void Ship::Heal(unsigned int amount){
	m_health += amount;
	if(m_health > m_maxHealth){
		m_health = m_maxHealth;
	}
}

bool Ship::IsAlive(){
	return m_health != 0;
}


int Ship::GetSpeed(Direction dir){
	switch(dir){
		case East:
			return eastStep;
			break;
		case West:
			return westStep;
			break;
		case North:
			return northStep;
			break;
		case South:
			return southStep;
			break;
	}
}
void Ship::StartMotion(Direction dir, int speed){
	switch(dir){
		case East:
			eastStep = speed;
			break;
		case West:
			westStep = 0-speed;
			break;
		case North:
			northStep = 0-speed;
			break;
		case South:
			southStep = speed;
			break;
	}
}
void Ship::StopMotion(Direction dir){
	switch(dir){
		case East:
			eastStep = 0;
			break;
		case West:
			westStep = 0;
			break;
		case North:
			northStep = 0;
			break;
		case South:
			southStep = 0;
			break;
	}
}
void Ship::Jump(){
}

sf::Vector2f Ship::Update(float value, float elapsed, sf::RenderWindow& App, vector<Ship>& neighbors){
	
	/*
	 ========
	 Check for death
	 ========
	 */
	
	if(!IsAlive())
		return sf::Vector2f(0, 0);
	
	/*
	sf::Vector2f position = this->GetPosition();
	sf::Color current = map.GetPixel(position.x, position.y);
	 */
	northStep -= value;
	
	
	
	
	/*
	 ========
	 x and y steps are combinations of directionals. If xStep is zero, then the net x axis motion is nil, if yStep is zero, then the net y axis motion is nil
	 ========
	 */
	
	int xStep = (eastStep + westStep);
	int yStep = (northStep + southStep);
	
	if(xStep > 0){
		if(_hEffect.GetPosition().x >= this->GetPosition().x || _hEffect.GetPosition().x == -1){
			_hEffect = Emitter(true, 120, 240, 25, 1).Begin();
		}
		_hEffect.SetPosition(this->GetPosition() - sf::Vector2f(this->GetSize().x-1, this->GetSize().y/2));
		_hEffect.Update(elapsed);
		App.Draw(_hEffect);
	}
	else if(xStep < 0){
		if(_hEffect.GetPosition().x <= this->GetPosition().x || _hEffect.GetPosition().x == -1){
			_hEffect = Emitter(true, -60, 60, 25, 1).Begin();
		}
		_hEffect.SetPosition(this->GetPosition() + sf::Vector2f(1, this->GetSize().y/-2));
		_hEffect.Update(elapsed);
		App.Draw(_hEffect);
	}
	else{
		_hEffect.Reset();
	}
	
	if(xStep == 0 && yStep == 0) //nothing to do, Ship is not in motion
		return sf::Vector2f(0, 0);
	
	/*
	 ========
	 Actual movement
	 ========
	 */
	if(eastStep != 0 && this->GetPosition().x + 30 >= App.GetDefaultView().GetHalfSize().x*2){
		eastStep = 0;
	}
	if(westStep != 0 && this->GetPosition().x <= 30 + this->GetSize().x){
		westStep = 0;
	}
	
	this->Move(
			   (eastStep+westStep) * elapsed,
			   (northStep+southStep) * elapsed
			   );
	
	/*
	 ========
	 Detect collisions, if needed, reverse movement.
	 ========
	 */
	
	for(vector<Ship>::iterator it=neighbors.begin(); it!=neighbors.end(); ++it){
		if(this != &(*it) && Collision::CircleTest((Sprite)(*this), (Sprite)(*it))){
			sf::Vector2f slope = this->GetPosition() - it->GetPosition();
			slope.x = slope.x * northStep*elapsed/300.f;
			slope.y = slope.y * northStep*elapsed/300.f;
			it->Move(slope);
			slope.x *= -1;
			slope.y *= -1;
			this->Move(slope);
			northStep += value;
			return slope + sf::Vector2f(0, -1*value*elapsed);
		 
			
			/*
			sf::Vector2f otherSize = it->GetSize();
			sf::Vector2f thisSize = this->GetSize();
			
			sf::Vector2f otherPos = it->GetPosition();
			sf::Vector2f thisPos = this->GetPosition();
			
			sf::Vector2f shiftEach;
			
			if(thisPos.x < otherPos.x && (thisPos.x + thisSize.x) > otherPos.x){ //other object is to the right, push it to the right until it isn't colliding
				shiftEach.x = ((thisPos.x + thisSize.x) - otherPos.x);
			}
			
			if(thisPos.y < otherPos.y && (thisPos.y + thisSize.y) > otherPos.y){ //other object is lower, push it down until it isn't colliding
				shiftEach.y = ((thisPos.y + thisSize.y) - otherPos.y);
			}
			
			//grrrrr
			
		//	it->Move(shiftEach.x, shiftEach.y);
			this->Move(-shiftEach.x, -shiftEach.y);*/
		}
	}
	
	northStep += value;
	return sf::Vector2f(0, -1*value*elapsed);
	
}