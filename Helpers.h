#include<vector>
#include<SFML/Graphics.hpp>
#include "Bezier.h"
#include "Ship.h"
using namespace std;


inline int NumDigits(int number){
	if(number < 0)
		number = 0-number;
	int count = 0;
	while(number > 0){
		count++;
		number /= 10;
	}
	return count;
}

typedef struct ProgressInfo{ //object to be passed into the gradient creation thread
	sf::Image* gradientStorage;
	sf::Sprite* gradientSprites;
	int index;
	sf::RenderWindow* application;
	kd::BezierSpline* spline;
	int cSize;
	bool constant;
	ProgressInfo(int size, sf::RenderWindow* app, sf::Image* storage, sf::Sprite* sprites, kd::BezierSpline* tSpline):
	cSize(size), index(0), application(app), gradientStorage(storage), gradientSprites(sprites), spline(tSpline), constant(false){}
	void MakeConstant(){
		constant = true;
	}
} ProgressInfo;

double DistanceFromCurve(std::vector<kd::Vector2f>& pixels, sf::Vector2f currentPos);
void PointsToPixels(vector<kd::Vector2f>& points, vector<kd::Vector2f>& pixels, sf::RenderWindow& application);
void ParseToGradient(int level, kd::BezierCurve* curve, sf::RenderWindow& application, sf::Image* destination);
void ProgressGradients(void* UserData);
void AIUpdatePlayer(Ship& player, kd::BezierCurve* curve, float elapsed, sf::RenderWindow* App);

class GameState{ //contains the global state of the game in progress
private:
	bool _paused;
	bool _ready;
	sf::RenderWindow* _app;
public:
	GameState(sf::RenderWindow* App) : _paused(false), _ready(false), _app(App){};
	
	GameState& Restart(){
		_ready = true;
		_paused = true;
		return *this;
	}
	GameState& Start(){
		_ready = false;
		_paused = false;
		return *this;
	}
	bool IsReady(){
		return _ready;
	}
	bool IsRunning(){
		return !_ready;
	}
	
	bool IsPaused(){
		return _paused;
	}
	GameState& Pause(){
		_paused = true;
		return *this;
	}
	GameState& Unpause(){
		_paused = false;
		return *this;
	}
	GameState& TogglePause(){
		_paused = !_paused;
		return *this;
	}
};