#include "Helpers.h"

double DistanceFromCurve(std::vector<kd::Vector2f>& pixels, sf::Vector2f currentPos){
	std::vector<kd::Vector2f>::iterator it;
	double cVal = INT_MAX;
	for(it = pixels.begin(); it < pixels.end(); it++){ //find least distance to the curve
		if((int)it->y > ((int)currentPos.y - 20) && (int)it->y < ((int)currentPos.y + 20)){
			int dist = abs(currentPos.x - it->x);
			if(dist < cVal)
				cVal = dist;
		}
	}
	return cVal;
}
void PointsToPixels(vector<kd::Vector2f>& points, vector<kd::Vector2f>& pixels, sf::RenderWindow& application){
	float width = application.GetWidth();
	float halfWidth = width/2;
	float height = application.GetHeight();
	float halfHeight = height/2;
	std::vector<kd::Vector2f>::iterator it;
	float oldX=-1, oldY=-1;
	for(it = points.begin(); it < points.end(); it++){
		float x = (width/20)*(it->x) + halfWidth;
		float y = halfHeight - ((height/20)*(it->y));
		
		if(x != oldX || y != oldY){
			pixels.push_back(kd::Vector2f(x, y));
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
		
		int n = it->y + height*level;
		
		for(int i = 0; i < it->x; i++){
			double t = (double)i/it->x;
			if(t < 0)
				t = 0;
			else if(t > 1)
				t = 1;
			sf::Color color = sf::Color((255 * t), (100 * (1 - t)), 0);
			
			if(i > 0 && i < width && n > 0 && n < height)
				destination->SetPixel(i, n, color);
			
		}
		for(int i = it->x; i <= width; i++){
			double t = (double)(i-it->x)/(double)(width-it->x);
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
		if((int)(it->y) == (int)(position.y)){
			int xDist = it->x - position.x;
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

sf::Unicode::Text IntToRank(int place){
	char buff[100];
	
	if(place > 3 && place < 21) //any number between 4 and 20, inclusive, always uses the th suffix
		sprintf(buff, "%dth", place);
	else{ //otherwise, find the last digit
		if(place < 0)
			place = -place;
		while(place > 10){
			place /= 10;
		}
		switch(place){
			case 1:
				sprintf(buff, "%dst", place);
				break;
			case 2:
				sprintf(buff, "%dnd", place);
				break;
			case 3:
				sprintf(buff, "%drd", place);
			default:
				sprintf(buff, "%dth", place); //4-9 and 0 endings use th
		}
	}
	return sf::Unicode::Text(buff);
}