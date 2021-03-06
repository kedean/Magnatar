#include <SFML/Graphics.hpp>

#pragma once
#include<iostream>
#include<vector>
#include "Collision.h"
#include "Emitter.h"

using namespace std;
using namespace sf;

class Ship : public sf::Sprite{
private:
	
	unsigned int m_health, m_maxHealth;
	vector<Emitter> _effects;
	Emitter _hEffect, _vEffect;
	
	//movement vars
	int eastStep, westStep, northStep, southStep;
	
	//update vars
	sf::View* view;
public:
	Ship(sf::Image* image, const sf::Vector2f &Position=sf::Vector2f(0, 0), const sf::Vector2f &Scale=sf::Vector2f(1, 1), float Rotation=0.f, const sf::Color &Col=sf::Color(255, 255, 255, 255));
	
	unsigned int GetHealth();
	unsigned int GetMaxHealth();
	void SetMaxHealth(unsigned int amount);
	
	void Damage(unsigned int amount);
	void Heal(unsigned int amount);
	
	bool IsAlive();
	
	enum Direction{
		East,
		West,
		North,
		South
	};
	int GetSpeed(Direction dir);
	void StartMotion(Direction dir, int speed=200);
	void StopMotion(Direction);
	void Jump();
	
	sf::Vector2f Update(float value, float elapsed, sf::RenderWindow& App, vector<Ship>& neighbors);
};