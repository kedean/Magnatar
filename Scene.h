#pragma once

#include<SFML/Graphics.hpp>
#include<json_spirit.h>
#include<fstream>
#include<string>
#include<vector>
#include "Helpers.h"
#include "HUD.h"

class Scene{ //Interface for a scene. All scene types must inherit the Scene class
protected:
	json_spirit::mObject& _settings;
	sf::RenderWindow& _application;
	string _id;
	
	vector<sf::Font> _fonts;
public:
	Scene(string id, sf::RenderWindow& application, json_spirit::mObject& settings) : _id(id), _settings(settings), _application(application){}
	virtual ~Scene(){}
	virtual void Loop()=0;
	virtual Scene* HandleEvent(sf::Event)=0;
	string GetID(){
		return _id;
	}
};

class Menu : public Scene{
private:
	sf::Shape _background;
	vector<sf::String> _textToRender;
	int _highlightIndex;
	float _highlightAlpha;
	int _highlightDir;
	sf::Clock _tapTime;
	kd::HeadsUpDisplay _HUD;
public:
	Menu(string id, sf::RenderWindow& application, json_spirit::mObject& settings);
	~Menu(){
		
	}
	void Loop();
	Scene* HandleEvent(sf::Event event);
};

class Settings: public Scene{
private:
	sf::Shape _background;
public:
	Settings(string id, sf::RenderWindow& application, json_spirit::mObject& settings);
	~Settings(){
		
	}
	void Loop();
	Scene* HandleEvent(sf::Event event);
};

class Game : public Scene{
private:
	int _rubber_banding;
	int _playerVal;
	int _playerPlace;
	GameState _game;
	kd::BezierSpline _spline;
	map<string, sf::Image> _playerImages;
	vector<Ship> _playerList;
	Ship* _player; //pointer to the player character
	
	//variables for the thread that generates gradient backgrounds
	sf::Image* _gradientStorage;
	sf::Sprite* _gradientSprites;
	ProgressInfo* _gradientInfo;
	sf::Thread* _createGradients;
	int _mFrames;
	
	//hud variables
	kd::HeadsUpDisplay _HUD;
	float _fadeCountdown;
	enum FadeState{
		FADING_IN,
		FADING_OUT,
		IDLE
	} _fade;
public:
	Game(string id, sf::RenderWindow& application, json_spirit::mObject& settings);
	~Game(){
		_createGradients->Terminate(); //We don't need the gradients anymore, stop generating them if its not done yet
		
		delete[] _gradientStorage;
		delete[] _gradientSprites;
		delete _gradientInfo;
		delete _createGradients;
	}
	void Loop();
	Scene* HandleEvent(sf::Event event);
};