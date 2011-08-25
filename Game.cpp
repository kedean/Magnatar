#include "Scene.h"
#include "Ship.h"
#include "Bezier.h"
#include "Emitter.h"
#include "HUD.h"

Game::Game(string id, sf::RenderWindow& application, json_spirit::mObject& settings) : Scene(id, application, settings){
	
	json_spirit::mValue rubber_band_effect = settings["rubber_banding"];
	if(rubber_band_effect.type() == json_spirit::int_type){
		_rubber_banding = rubber_band_effect.get_int();
	}
	else{
		_rubber_banding = 1;
	}
	
	srand(time(NULL));
	
	int gSize;
	json_spirit::mValue track_length = settings["track_length"];
	if(track_length.type() == json_spirit::int_type)
		gSize = track_length.get_int(); //number of image blocks used to hold the gradient, number of component curves to the spline (one per gradient)
	else
		gSize = 1000;
	
	json_spirit::mValue track_color = settings["track_color"];
	if(track_color.type() == json_spirit::array_type){
		json_spirit::mArray track_color_array = track_color.get_array();
		while(track_color_array.size() < 3) //ensure 3 values, an r g and b
			track_color_array.push_back(0);
		_spline.SetColor(kd::Color3f(track_color_array[0].get_int(), track_color_array[1].get_int(), track_color_array[2].get_int()));
	}
	else{
		_spline.SetColor(kd::Color3f(255, 0, 0));
	}
	
	
	_spline.SetVertexRadius(0.08f)
		   .SetThrottle(0.0001f); //throttle depends on number of digits in the number. As that goes up, the speed that throttle increases with gSize goes down, to allow for a smooth curve/gradient and all levels
	
	/*Randomized curve generation loop*/
	
	_spline.AddPoint(0, -10, false);
	int direction = 1;
	for(int n = 1; n < gSize; n++){
		kd::Vector2f prevP = _spline.GetControls().back();
		
		int r = (rand() % 2);
		
		int x = prevP.x + r*direction;
		if(x > 7){ //if the new x is outside the threshold, reverse the alteration
			x = 7;
			direction = -1;
		}
		else if(x < -7){
			x = -7;
			direction = 1;
		}
		
		int y = n*5 - 10;
		if(n+1 == gSize){ //last point on the curve should be centered
			_spline.AddPoint(0, y, false);
		}
		else if(n+2 < gSize && n % 4 == 0){ //new curve
			_spline.AddPoint(x, y, false).AddCutoff().AddPoint(x, y, false);
			n++;
			_spline.AddPoint(x + (x-prevP.x), n*5 - 10, false);
		}
		else{
			_spline.AddPoint(x, y, false);
		}
	}
	
	
	//Load up the players
	
	json_spirit::mValue playerList = settings["players"];
	cout << playerList.type() << endl;
	if(playerList.type() == json_spirit::array_type){
		vector<json_spirit::mValue> playerShips = playerList.get_array();
		vector<json_spirit::mValue>::iterator ship;
		int nPlayers = playerShips.size();
		
		for(int i = 0; i < nPlayers; i++){
			if(playerShips[i].type() == json_spirit::str_type){
				string filename = playerShips[i].get_str();
				map<string, sf::Image>::iterator image = _playerImages.find(filename);
				
				if(image == _playerImages.end()){ //image not yet loaded
					_playerImages[filename] = sf::Image();
					image = _playerImages.find(filename);
					image->second.LoadFromFile(filename);
				}
				
				int xPos;
				if(i > nPlayers/2)
					xPos = -1 *((i - nPlayers/2) * 100) - 50;
				else
					xPos = (i * 100) - 50;
				
				_playerList.push_back(Ship(&(image->second), _application.GetDefaultView().GetHalfSize() + sf::Vector2f(xPos, 200), sf::Vector2f(1,1), 180.f));
			}
		}
	}
	else{
		
	}
	/*
	int nPlayers;
	json_spirit::mValue num_players = settings["num_players"];
	if(num_players.type() == json_spirit::int_type)
		nPlayers = num_players.get_int(); //number of image blocks used to hold the gradient, number of component curves to the spline (one per gradient)
	else
		nPlayers = 6;
	
	for(int i = 0; i < 6; i++){
		char filename[100];
		sprintf(filename, "ship%d.png", i); //first player is always the same sprite, the others rotate among the other 5 sprites
		_playerImages[i].LoadFromFile(filename);
	}
	
	for(int i = 0; i < nPlayers; i++){
		
		int xPos;
		if(i > nPlayers/2)
			xPos = -1 *((i - nPlayers/2) * 100) - 50;
		else
			xPos = (i * 100) - 50;
		int n = (i == 0) ? 0 : ((i % 5) + 1);
		_playerList.push_back(Ship(&(_playerImages[n]), _application.GetDefaultView().GetHalfSize() + sf::Vector2f(xPos, 200), sf::Vector2f(1,1), 180.f));
	}*/
	
	_player = &_playerList[0]; //instance that is being controlled
	
	//Load up the level
	
	_mFrames = _spline.GetCurves().size();
	
	_gradientStorage = new sf::Image[_mFrames];
	_gradientSprites = new sf::Sprite[_mFrames];
	
	_gradientInfo = new ProgressInfo(_mFrames, &_application, _gradientStorage, _gradientSprites, &_spline);
	
	ProgressGradients((void*)_gradientInfo); //preload the first few frames to prevent scratch at the start
	ProgressGradients((void*)_gradientInfo);
	ProgressGradients((void*)_gradientInfo);
	
	_gradientInfo->constant = true;
	
	_createGradients = new sf::Thread(&ProgressGradients, (void*)_gradientInfo); //load the other frames via a thread
	_createGradients->Launch();
	
	/*HUD setup*/
	
	int width = _application.GetWidth();
	
	_HUD.AddWidget("rank", (sf::Drawable*) (new sf::String("", sf::Font::GetDefaultFont(), 20)), sf::Vector2f(width-50, 0));
	
/*	sf::Shape* raceBar = static_cast<sf::Shape*>(_HUD.AddWidget("raceBar", (sf::Drawable*) (new sf::Shape)));
	*raceBar = sf::Shape::Rectangle(0, 0, 6, 400, sf::Color(255, 255, 255, 150));
	raceBar->SetPosition(sf::Vector2f(5, 50));
	
	sf::Shape* raceProgressIndicator = static_cast<sf::Shape*>(_HUD.AddWidget("racePlayer", (sf::Drawable*) (new sf::Shape)));
	*raceProgressIndicator = sf::Shape::Circle(0, 0, 4, sf::Color(255, 255, 255));
	raceProgressIndicator->SetPosition(8, 446);
*/	
	sf::Shape* powerBorder = static_cast<sf::Shape*>(_HUD.AddWidget("powerBorder", (sf::Drawable*) (new sf::Shape)));
	*powerBorder = sf::Shape::Rectangle(0, 0, 402, 20, sf::Color(255, 255, 255, 200));
	powerBorder->SetPosition(5, 5);
	
	sf::Shape* powerBar = static_cast<sf::Shape*>(_HUD.AddWidget("powerBar", (sf::Drawable*) (new Shape()), sf::Vector2f(6, 6)));
	powerBar->AddPoint(0, 0, sf::Color(255, 255, 0, 200));
	powerBar->AddPoint(400, 0, sf::Color(255, 0, 0, 200));
	powerBar->AddPoint(400, 18, sf::Color(255, 0, 0, 200));
	powerBar->AddPoint(0, 18, sf::Color(255, 255, 0, 200));

	//set up pause screen
	_game.Restart(); //game starts paused with a countdown timer
	sf::Shape* backdrop = static_cast<sf::Shape*>(_HUD.AddWidget("backdrop", (sf::Drawable*) (new Shape)));
	*backdrop = sf::Shape::Rectangle(0, 0, _application.GetWidth(), _application.GetHeight(), sf::Color(0, 0, 0, 100));
	
	_HUD.AddWidget("countdown", (sf::Drawable*) (new sf::String("", sf::Font::GetDefaultFont(), 30)), sf::Vector2f(_application.GetDefaultView().GetCenter() - sf::Vector2f(15, 15)));
	
	_fadeCountdown = 0;
	_fade = IDLE;
	
	//set up the text that declare it is a pause screen
	sf::String* pauseText = static_cast<sf::String*>(_HUD.AddWidget("pauseText", (sf::Drawable*) (new sf::String("Magnet Frozen", sf::Font::GetDefaultFont(), 30))));
	pauseText->SetPosition((_application.GetWidth() - pauseText->GetCharacterPos(13).x)/2, (_application.GetHeight() - pauseText->GetSize())/2);
	pauseText->SetColor(sf::Color(0,0,0,0));
}

void Game::Loop(){
	float height = _application.GetHeight();
	float width = _application.GetWidth();
	sf::View& view = _application.GetDefaultView();
	sf::Vector2f oldCenter = view.GetCenter();

	int mBound = abs((view.GetCenter().y - height) - height/2) / height;
	float elapsed = _application.GetFrameTime();

	_application.Draw(_gradientSprites[mBound - 1]);
	_application.Draw(_gradientSprites[mBound]);
	if(mBound+1 >= _mFrames){
		_fade = FADING_IN;
		_fadeCountdown = 2;
		_game.Pause();
	}
	else
	_application.Draw(_gradientSprites[mBound + 1]);

	static int playerVal = 0; //static so they will stay during pause screens (for display)
	static int playerPlace = 1;
	vector<Ship>::iterator playerIt;

	if(_game.IsPaused() == false && _fade == IDLE){ //paused games should not update the players
		
		//player update loop			
		for(playerIt = _playerList.begin(); playerIt < _playerList.end(); playerIt++){
			int cIndex = abs(playerIt->GetPosition().y - height) / height;
			vector<kd::Vector2f> lPoints = _spline.GetCurve(cIndex)->GetPoints();
			vector<kd::Vector2f> lPixels;
			PointsToPixels(lPoints, lPixels, _application);
			
			int forwardVal = DistanceFromCurve(lPixels, playerIt->GetPosition() + sf::Vector2f(15, 15)); //distance between the curve and this player, on their y position. Used to determine speed.
			if(forwardVal > width) //infinitely far from curve, thats not allowed. No motion in that case.
				forwardVal = 50;
			else
				forwardVal = width - forwardVal; //invert, further away from the curve should be smaller
			forwardVal = (forwardVal*forwardVal)/1000; //square and scale down
			
			int pointOfRef = playerIt->GetPosition().y;
			int place = 0; //initial place is zero. The iterator will pass over the player as well, so the offset is made there
			vector<Ship>::iterator placeIt;
			for(placeIt = _playerList.begin(); placeIt < _playerList.end(); placeIt++){
				if(placeIt->GetPosition().y < pointOfRef)
					place++;
			}
			
			sf::Vector2f moved = playerIt->Update(forwardVal + _rubber_banding*place, elapsed, _application, _playerList);
			
			if(&(*playerIt) == _player){ //player char. Move the camera and update his vars
				view.SetCenter(view.GetCenter().x, _player->GetPosition().y - 200);
				playerVal = forwardVal;
				playerPlace = place;
			}
			else {
				AIUpdatePlayer(*playerIt, _spline.GetCurve(cIndex), elapsed, &_application);
			}
			
			_application.Draw(*playerIt);
		}
	}
	else{ //draw but don't update anything
		for(playerIt = _playerList.begin(); playerIt < _playerList.end(); playerIt++){
			_application.Draw(*playerIt);
		}
	}

	/*Display the HUD*/

	if(playerVal != 1){
		playerVal *= (400.f/((width*width)/1000));
		sf::Shape* powerBar = static_cast<sf::Shape*>(_HUD["powerBar"]);
		powerBar->SetPointPosition(0, 0, 0);
		powerBar->SetPointPosition(1, playerVal, 0);
		powerBar->SetPointPosition(2, playerVal, 18);
		powerBar->SetPointPosition(3, 0, 18);
	}
	
	static_cast<sf::String*>(_HUD["rank"])->SetText(IntToRank(playerPlace));

	if(_game.IsPaused() || _fade != IDLE){ //fading of pause screen
		sf::Shape* backdrop = static_cast<sf::Shape*>(_HUD["backdrop"]);
		
		if(_fade == FADING_IN){
			int c = backdrop->GetColor().a;
			int alpha = backdrop->GetColor().a + 150.f * elapsed/_fadeCountdown;
			
			backdrop->SetColor(sf::Color(0, 0, 0, alpha));
			if(alpha >= 150){
				_fadeCountdown = 0;
				_fade = IDLE;
			}
		}
		else if(_fade == FADING_OUT){
			float alpha = backdrop->GetColor().a - 150.f * elapsed/_fadeCountdown;
			backdrop->SetColor(sf::Color(0, 0, 0, alpha));
			if(alpha <= 10){
				_fadeCountdown = 0;
				_fade = IDLE;
			}
		}
	}

	if(_game.IsReady()){ //Countdown screen, only appear at start of game
		static int timer = -1; //will cause a bug with multiple instances, fix this.
		if(timer < 0)
			timer = 3;
			
			if(timer == 0){
				timer = -1;
				_fadeCountdown = .2f;
				_fade = FADING_OUT;
				_game.Start();
			}
		
		char timerStr[5];
		if(timer > 0)
			sprintf(timerStr, "%d", timer);
		else
			sprintf(timerStr, "");
			
			static_cast<sf::String*>(_HUD["countdown"])->SetText(timerStr);
			sf::Sleep(1.f);
		timer--;
	}

	_HUD.SetPosition(view.GetCenter() - view.GetHalfSize());
	_application.Draw(_HUD);
}

Scene* Game::HandleEvent(sf::Event Event){
	if(Event.Type == sf::Event::KeyPressed){
		switch(Event.Key.Code){
			case sf::Key::Return:
				if(_game.IsRunning() && !_game.IsReady()){
					sf::String* pauseText = static_cast<sf::String*>(_HUD["pauseText"]);
					if(_game.IsPaused()){
						_fade = FADING_OUT;
						pauseText->SetColor(sf::Color(0,0,0,0));
					}
					else{
						_fade = FADING_IN;
						pauseText->SetColor(sf::Color(255,255,255,255));
					}
					_fadeCountdown = .2f;
					_game.TogglePause();
				}
				break;
			case 'w':
				break;
			case 's':
				break;
			case 'd':
			case sf::Key::Right:
				_player->StartMotion(Ship::East);
				break;
			case 'a':
			case sf::Key::Left:
				_player->StartMotion(Ship::West);
				break;
		}
	}
	else if(Event.Type == sf::Event::KeyReleased){
		switch(Event.Key.Code){
			case 'd':
			case sf::Key::Right:
				_player->StopMotion(Ship::East);
				break;
			case 'a':
			case sf::Key::Left:
				_player->StopMotion(Ship::West);
				break;
		}
	}
	
	return this;
}