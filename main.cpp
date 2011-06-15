#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "Ship.h"
#include "Bezier.h"
#include "Helpers.h"
#include "Emitter.h"


int main(){
	sf::RenderWindow App(sf::VideoMode(900, 900*sf::VideoMode::GetDesktopMode().Height/sf::VideoMode::GetDesktopMode().Width), "Magnatar", sf::Style::Close);
	//sf::Style::Fullscreen
	App.SetFramerateLimit(120);
	App.PreserveOpenGLStates(true);
	
	sf::View& view = App.GetDefaultView();
	
	GameState game(&App);
	
	srand(time(NULL));
	
	const int gSize = 500; //number of image blocks used to hold the gradient, number of component curves to the spline (one per gradient)
	
	kd::BezierSpline spline;
	spline.SetColor(kd::Color3f(255, 0, 0))
		  .SetVertexRadius(0.08f)
		  .SetThrottle(0.0001f); //throttle depends on number of digits in the number. As that goes up, the speed that throttle increases with gSize goes down, to allow for a smooth curve/gradient and all levels
	
	/*Randomized curve generation loop*/
	
	spline.AddPoint(0, -10, false);
	for(int n = 1; n < gSize; n++){
		kd::Vector2f prevP = spline.GetControls().back();
		
		int r = (rand() % 6 - 3);
		
		int x = prevP.x + r;
		if(x < -7 || x > 7) //if the new x is outside the threshold, reverse the alteration
			x = prevP.x - r;
		
		int y = n*5 - 10;
		if(n+1 == gSize){ //last point on the curve should be centered, as should x one
			spline.AddPoint(0, y, false);
		}
		else if(n+2 < gSize && n % 4 == 0){ //new curve
			spline.AddPoint(x, y, false).AddCutoff().AddPoint(x, y, false);
			n++;
			spline.AddPoint(x + (x-prevP.x), n*5 - 10, false);
		}
		else{
			spline.AddPoint(x, y, false);
		}
	}
	
	
	//Load up the players
	
	sf::Image playerImages[6];
	vector<Ship> playerList;
	for(int i = 0; i < 6; i++){
		char filename[100];
		sprintf(filename, "ship%d.png", i);
		playerImages[i].LoadFromFile(filename);
		playerList.push_back(Ship(&(playerImages[i]), App.GetDefaultView().GetHalfSize() + sf::Vector2f((i-3)*100, 200), sf::Vector2f(1,1), 180.f));
	}
	Ship* player = &playerList[0]; //instance that is being controlled
	
	
	//Load up the level
	
	int cSize = spline.GetCurves().size();
	
	sf::Image gradientStorage[cSize];
	sf::Sprite gradientSprites[cSize];
	
	ProgressInfo gradientInfo(cSize, &App, gradientStorage, gradientSprites, &spline);
	
	ProgressGradients((void*)&gradientInfo); //preload the first few frames to prevent scratch at the start
	
	gradientInfo.constant = true;
	
	sf::Thread createGradients(&ProgressGradients, (void*)&gradientInfo); //load the other frames via a thread
	createGradients.Launch();
	
	/*HUD setup*/
	
	int width = App.GetWidth();
	sf::Shape powerBorder = sf::Shape::Rectangle(0, 0, 402, 20, sf::Color(255, 255, 255, 200));
	sf::Shape powerBar;
	powerBar.AddPoint(0, 0, sf::Color(255, 255, 0, 200));
	powerBar.AddPoint(400, 0, sf::Color(255, 0, 0, 200));
	powerBar.AddPoint(400, 18, sf::Color(255, 0, 0, 200));
	powerBar.AddPoint(0, 18, sf::Color(255, 255, 0, 200));
	
	sf::String placingText("1st", sf::Font::GetDefaultFont(), 20);
	
	//set up pause screen
	
	game.Start(); //game starts paused
	sf::Shape backdrop = sf::Shape::Rectangle(0, 0, App.GetWidth(), App.GetHeight(), sf::Color(0, 0, 0, 100));
	
	float fadeCountdown = 0;
	enum FadeState{
		FADING_IN,
		FADING_OUT,
		IDLE
	} fade = IDLE;
	
	vector<Emitter> effects;
	
    // Start game loop
    while (App.IsOpened())
    {
		// Process events
        sf::Event Event;
        while (App.GetEvent(Event))
        {
            if(Event.Type == sf::Event::Closed)
                App.Close();
			else if(Event.Type == sf::Event::KeyPressed){
				switch(Event.Key.Code){
					case 'p':
						if(game.IsRunning()){
							if(game.IsPaused()){
								fade = FADING_OUT;
							}
							else{
								fade = FADING_IN;
							}
							fadeCountdown = .2;
							game.TogglePause();
						}
						break;
					case 'w':
						break;
					case 's':
						break;
					case 'd':
						player->StartMotion(Ship::East);
						break;
					case 'a':
						player->StartMotion(Ship::West);
						break;
				}
			}
			else if(Event.Type == sf::Event::KeyReleased){
				switch(Event.Key.Code){
					case 'd':
						player->StopMotion(Ship::East);
						break;
					case 'a':
						player->StopMotion(Ship::West);
						break;
				}
			}
        }
		
		App.SetActive();
		
		App.Clear();
		
		float height = App.GetHeight();
		sf::Vector2f oldCenter = view.GetCenter();
		
		int mBound = abs((view.GetCenter().y - height) - height/2) / height;
		float elapsed = App.GetFrameTime();
		
		App.Draw(gradientSprites[mBound - 1]);
		App.Draw(gradientSprites[mBound]);
		App.Draw(gradientSprites[mBound + 1]);
		
		static int playerVal = 0;
		vector<Ship>::iterator playerIt;
		
		if(game.IsPaused() == false && fade == IDLE){ //paused games should not update the players
			
			//player update loop			
			for(playerIt = playerList.begin(); playerIt < playerList.end(); playerIt++){
				int cIndex = abs(playerIt->GetPosition().y - height) / height;
				vector<kd::Vector2f> lPoints = spline.GetCurve(cIndex)->GetPoints();
				vector<kd::Vector2f> lPixels;
				PointsToPixels(lPoints, lPixels, App);
				
				int forwardVal = DistanceFromCurve(lPixels, playerIt->GetPosition() + sf::Vector2f(15, 15)); //distance between the curve and this player, on their y position. Used to determine speed.
				if(forwardVal > width) //infinitely far from curve, thats not allowed. No motion in that case.
					forwardVal = 50;
				else
					forwardVal = width - forwardVal; //invert, further away from the curve should be smaller
				forwardVal = (forwardVal*forwardVal)/1000; //square and scale down
				int t = playerIt->GetPosition().y;
				
				sf::Vector2f moved = playerIt->Update(forwardVal, elapsed, App, playerList);
				
				if(&(*playerIt) == player){ //player char. Move the camera and update his vars
					view.SetCenter(view.GetCenter().x, player->GetPosition().y - 200);
					playerVal = forwardVal;
				}
				else {
					AIUpdatePlayer(*playerIt, spline.GetCurve(cIndex), elapsed, &App);
				}

				
				App.Draw(*playerIt);
			}
		}
		else{ //draw but don't update anything
			for(playerIt = playerList.begin(); playerIt < playerList.end(); playerIt++){
				App.Draw(*playerIt);
			}
		}
		
		vector<Emitter>::iterator emitIt;
		for(emitIt = effects.begin(); emitIt < effects.end(); emitIt++){
			emitIt->Move(0, view.GetCenter().y - oldCenter.y);
			emitIt->Update(elapsed);
			App.Draw(*emitIt);
		}
		
		/*Display the HUD*/
		
		if(playerVal != 1){
			playerVal *= (400.f/((width*width)/1000));
			powerBar.SetPointPosition(0, 0, 0);
			powerBar.SetPointPosition(1, playerVal, 0);
			powerBar.SetPointPosition(2, playerVal, 18);
			powerBar.SetPointPosition(3, 0, 18);
		}
		powerBorder.SetPosition(view.GetCenter() - view.GetHalfSize() + sf::Vector2f(5, 5));
		powerBar.SetPosition(powerBorder.GetPosition() + sf::Vector2f(1, 1));
		App.Draw(powerBorder);
		App.Draw(powerBar);
		
		//determine the rank of player char
		int pointOfRef = player->GetPosition().y;
		int playerPlace = 0; //initial place is zero. The iterator will pass over the player as well, so the offset is made there
		for(playerIt = playerList.begin(); playerIt < playerList.end(); playerIt++){
			if(playerIt->GetPosition().y < pointOfRef)
				playerPlace++;
		}
		switch(playerPlace){
			case 1:
				placingText.SetText("1st");
				break;
			case 2:
				placingText.SetText("2nd");
				break;
			case 3:
				placingText.SetText("3rd");
				break;
			case 4:
				placingText.SetText("4th");
				break;
			case 5:
				placingText.SetText("5th");
				break;
			case 6:
				placingText.SetText("6th");
				break;
		}
		placingText.SetPosition(view.GetCenter() - view.GetHalfSize() + sf::Vector2f(App.GetWidth() - 50, 5));
		App.Draw(placingText);
		
		if(game.IsPaused() || fade != IDLE){ //display and fading of pause screen
			if(fade == FADING_IN){
				int c = backdrop.GetColor().a;
				int alpha = backdrop.GetColor().a + 150.f * elapsed/fadeCountdown;
				
				backdrop.SetColor(sf::Color(0, 0, 0, alpha));
				if(alpha >= 150){
					fadeCountdown = 0;
					fade = IDLE;
				}
			}
			else if(fade == FADING_OUT){
				float alpha = backdrop.GetColor().a - 150.f * elapsed/fadeCountdown;
				backdrop.SetColor(sf::Color(0, 0, 0, alpha));
				if(alpha <= 10){
					fadeCountdown = 0;
					fade = IDLE;
				}
			}
			backdrop.SetPosition(view.GetCenter() - view.GetHalfSize());
			App.Draw(backdrop);
		}
		
		if(game.IsReady()){ //Countdown screen, only appear at start of game
			static int timer = -1;
			if(timer < 0)
				timer = 3;
			
			if(timer == 0){
				timer = -1;
				fadeCountdown = .2f;
				fade = FADING_OUT;
				game.Start();
			}
			
			char timerStr[5];
			if(timer > 0)
				sprintf(timerStr, "%d", timer);
			else
				sprintf(timerStr, "");

			sf::String timerText(timerStr, sf::Font::GetDefaultFont(), 30);
			timerText.SetPosition(view.GetCenter() - sf::Vector2f(15, 15));
			App.Draw(timerText);
			sf::Sleep(1.f);
			timer--;
		}
		
	/*
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef((view.GetCenter().x - 400.f) / 10.f, (20.f/600.f) * (view.GetCenter().y - 300.f), -10.f);
		glRotatef(0,0,0,0);
		
		spline.DrawCurve().DrawControls();
		
		*/
		
        App.Display();
		
    }
 	
	createGradients.Terminate();
	
    return EXIT_SUCCESS;
}
