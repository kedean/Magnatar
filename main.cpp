#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "Ship.h"
#include "Bezier.h"

double DistanceFromCurve(std::vector<kd::Vector2f>& pixels, sf::Vector2f currentPos){
	std::vector<kd::Vector2f>::iterator it;
	double cVal = INT_MAX;
	for(it = pixels.begin(); it < pixels.end(); it++){ //find least distance to the curve
		if(it->second > (int)currentPos.y - 100 && it->second < (int)currentPos.y + 100){
			int dist = abs(currentPos.x - it->first);
			if(dist < cVal)
				cVal = dist;
		}
	}
	return cVal;
}
int NumDigits(int number){
	if(number < 0)
		number = 0-number;
	int count = 0;
	while(number > 0){
		count++;
		number /= 10;
	}
	return count;
}
void PointsToPixels(vector<kd::Vector2f>& points, vector<kd::Vector2f>& pixels, sf::RenderWindow& application){
	float width = application.GetWidth();
	float halfWidth = width/2;
	float height = application.GetHeight();
	float halfHeight = height/2;
	std::vector<kd::Vector2f>::iterator it;
	float oldX=-1, oldY=-1;
	for(it = points.begin(); it < points.end(); it++){
		float x = (width/20)*(it->first) + halfWidth;
		float y = halfHeight - ((height/20)*(it->second));
		
		if(x != oldX || y != oldY){
			pixels.push_back(make_pair(x, y));
			oldX = x;
			oldY = y;
		}
	}
}
void ParseToGradient(int level, kd::BezierCurve* curve, sf::RenderWindow& application, sf::Image* destination){	
	float width = application.GetWidth();
	float height = application.GetHeight();
	
	destination->Create(width, height, sf::Color(0, 255, 0));
	destination->SetSmooth(false);
	
	int xOld = -1, yOld = -1; //used to avoid duplicate values, as bezier curves can produce when converted to int values
	
	vector<kd::Vector2f> points = curve->GetPoints();
	vector<kd::Vector2f> pixels;
	PointsToPixels(points, pixels, application);
	
	std::vector<kd::Vector2f>::iterator it;
	for(it = pixels.begin(); it < pixels.end(); it++){
		
		int n = it->second + height*level;
		
		for(int i = 0; i < it->first; i++){
			double t = (double)i/it->first;
			if(t < 0)
				t = 0;
			else if(t > 1)
				t = 1;
			sf::Color color = sf::Color((255 * t), (100 * (1 - t)), 0);
			
			if(i > 0 && i < width && n > 0 && n < height)
				destination->SetPixel(i, n, color);
			
		}
		for(int i = it->first; i <= width; i++){
			double t = (double)(i-it->first)/(double)(width-it->first);
			if(t < 0)
				t = 0;
			else if(t > 1)
				t = 1;
			sf::Color color = sf::Color((255 * (1-t)), (100 * t), 0);
			if(i > 0 && i < width && n > 0 && n < height)
				destination->SetPixel(i, n, color);
		}
	}
}

typedef struct ProgressInfo{ //object to be passed into the gradient creation thread
	sf::Image* gradientStorage;
	sf::Sprite* gradientSprites;
	int index;
	sf::RenderWindow* application;
	kd::BezierSpline* spline;
	int cSize;
	bool constant;
	ProgressInfo(int size, sf::RenderWindow* app, sf::Image* storage, sf::Sprite* sprites, kd::BezierSpline* useSpline):
	cSize(size), index(0), application(app), gradientStorage(storage), gradientSprites(sprites), spline(useSpline), constant(false){}
} ProgressInfo;

void ProgressGradients(void* UserData){
	ProgressInfo* info = static_cast<ProgressInfo*>(UserData);
	
	int maxIndex;
	if(info->constant == false){
		maxIndex = info->index+1;
	}
	else{
		maxIndex = info->cSize;
	}
	for(int i = info->index; i < maxIndex; i++){
		kd::BezierCurve* curve = info->spline->GetCurve(i);
		curve->Regenerate();
		ParseToGradient(i, curve, *(info->application), &(info->gradientStorage[i]));
		info->gradientSprites[i] = sf::Sprite(info->gradientStorage[i]);
		info->gradientSprites[i].SetPosition(-1, 0 - (info->application->GetHeight()-3.f)*i);
		++info->index;
		sf::Sleep(0.01f); //delay helps ensure smooth gameplay outside the thread
	}
}

void AIUpdatePlayer(Ship& player, kd::BezierCurve* curve, float elapsed, sf::RenderWindow* App){
	vector<kd::Vector2f> lPoints = curve->GetPoints();
	vector<kd::Vector2f> lPixels;
	PointsToPixels(lPoints, lPixels, *App);
	
	sf::Vector2f position = player.GetPosition();
	
	vector<kd::Vector2f>::iterator it;
	for(it = lPixels.begin(); it < lPixels.end(); it++){
		if((int)(it->second) == (int)(position.y)){
			int xDist = it->first - position.x;
			if(xDist > 0){
				player.StopMotion(Ship::West);
				player.StartMotion(Ship::East, (rand()%25)/10.f * 100);
			}
			else{
				player.StopMotion(Ship::East);
				player.StartMotion(Ship::West, (rand()%25)/10.f * 100);
			}
			return;
		}
	}
}

int main(){
	sf::RenderWindow App(sf::VideoMode(900, 900*sf::VideoMode::GetDesktopMode().Height/sf::VideoMode::GetDesktopMode().Width), "Magnatar", sf::Style::Close);
	//sf::Style::Fullscreen
	App.SetFramerateLimit(120);
	App.PreserveOpenGLStates(true);
	
	sf::View& view = App.GetDefaultView();
	
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
		
		int x = prevP.first + r;
		if(x < -7 || x > 7) //if the new x is outside the threshold, reverse the alteration
			x = prevP.first - r;
		
		int y = n*5 - 10;
		if(n+1 == gSize){ //last point on the curve should be centered, as should first one
			spline.AddPoint(0, y, false);
		}
		else if(n+2 < gSize && n % 4 == 0){ //new curve
			spline.AddPoint(x, y, false).AddCutoff().AddPoint(x, y, false);
			n++;
			spline.AddPoint(x + (x-prevP.first), n*5 - 10, false);
		}
		else{
			spline.AddPoint(x, y, false);
		}
	}
//	spline.Regenerate();
	
	sf::Image playerImages[6];
	vector<Ship> playerList;
	for(int i = 0; i < 6; i++){
		char filename[100];
		sprintf(filename, "ship%d.png", i);
		playerImages[i].LoadFromFile(filename);
		playerList.push_back(Ship(&(playerImages[i]), App.GetDefaultView().GetHalfSize() + sf::Vector2f((i-3)*100, 200), sf::Vector2f(1,1), 180.f));
	}
	Ship* player = &playerList[0]; //instance that is being controlled
	
	int cSize = spline.GetCurves().size();
	
	sf::Image gradientStorage[cSize];
	sf::Sprite gradientSprites[cSize];
	
	ProgressInfo gradientInfo(cSize, &App, gradientStorage, gradientSprites, &spline);
	
	ProgressGradients((void*)&gradientInfo); //preload the first few frames to prevent scratch at the start
	ProgressGradients((void*)&gradientInfo);
	ProgressGradients((void*)&gradientInfo);
	
	gradientInfo.constant = true;
	
	sf::Thread createGradients(&ProgressGradients, (void*)&gradientInfo); //load the other frames via a thread
	createGradients.Launch();
	
	/*HUD setup*/
	
	int width = App.GetWidth();
	sf::Shape powerBorder = sf::Shape::Rectangle(0, 0, (width*width)/1000 + 2, 20, sf::Color(255, 255, 255, 200));
	sf::Shape powerBar;
	powerBar.AddPoint(0, 0, sf::Color(255, 255, 0, 200));
	
	powerBar.AddPoint((width*width)/1000, 0, sf::Color(255, 0, 0, 200));
	powerBar.AddPoint((width*width)/1000, 18, sf::Color(255, 0, 0, 200));
	powerBar.AddPoint(0, 18, sf::Color(255, 255, 0, 200));
	
	sf::String placingText("1st", sf::Font::GetDefaultFont(), 20);
	
    // Start game loop
    while (App.IsOpened())
    {
		sf::Vector2f center = App.GetDefaultView().GetCenter();
		float height = App.GetHeight();
		
		int mBound = abs((center.y - height) - height/2) / height;
		int tBound = mBound+1;
		int bBound = mBound-1;
		float elapsed = App.GetFrameTime();
		
        // Process events
        sf::Event Event;
        while (App.GetEvent(Event))
        {
            if(Event.Type == sf::Event::Closed)
                App.Close();
			else if(Event.Type == sf::Event::KeyPressed){
				switch(Event.Key.Code){
					case 'p':
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
		
		App.Draw(gradientSprites[bBound]);
		App.Draw(gradientSprites[mBound]);
		App.Draw(gradientSprites[tBound]);
		
		int playerVal;
		
		//player update loop
		vector<Ship>::iterator playerIt;
		
		for(playerIt = playerList.begin(); playerIt < playerList.end(); playerIt++){
			int cIndex = abs(playerIt->GetPosition().y - height) / height;
			vector<kd::Vector2f> lPoints = spline.GetCurve(cIndex)->GetPoints();
			vector<kd::Vector2f> lPixels;
			PointsToPixels(lPoints, lPixels, App);
			
			int forwardVal = DistanceFromCurve(lPixels, playerIt->GetPosition() + sf::Vector2f(15, 15)); //distance between the curve and this player, on their y position. Used to determine speed.
			if(forwardVal == INT_MAX)
				forwardVal = 50;
			else
				forwardVal = width - forwardVal; //invert, further away from the curve should be smaller
			forwardVal = (forwardVal*forwardVal)/1000; //square and scale down
			int t = playerIt->GetPosition().y;
			
			sf::Vector2f moved = playerIt->Update(forwardVal, elapsed, App, playerList);
			
			if(&(*playerIt) == player){ //player char. Move the camera and update his vars
				view.SetCenter(center + Vector2f(0, moved.y));
				playerVal = forwardVal;
			}
			else {
				AIUpdatePlayer(*playerIt, spline.GetCurve(cIndex), elapsed, &App);
			}

			
			App.Draw(*playerIt);
		}
		
		/*Display the HUD*/
		
		if(playerVal != 1){
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
