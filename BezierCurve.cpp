#include "Bezier.h"
#include<iostream>

using namespace kd;

Vector2f BezierCurve::Interpolate(double i){
	
	int n = _controls.size() - 1; //n should actually be one less than the number of coords
	
	double x = 0, y = 0; //output coords
	
	double iN = 1; //at the end this will be i to the nth power
	
	double j = 1 - i; //additive inverse of i
	double jN = 1;
	for(int k = 0; k < n; k++) //jN is j to the nth power, will be 1 by the end
		jN *= j;
	
	for(int k = 0; k <= n; k++){
		double multiplier = BinomialCoefficient(n, k) * iN * jN; //middle values are multiplied by n, then in any case by the current powers of i and j
		iN *= i;
		jN /= j;
		
		x += _controls[k].x * multiplier;
		y += _controls[k].y * multiplier;
	}
	return Vector2f(x, y);
}

Vector2f BezierCurve::Derive(double i){
	
	int n = _controls.size() - 1; //n should actually be one less than the number of coords
	
	vector<Vector2f> q_controls;
	
	for(int k = 0; k < n; k++){
		q_controls.push_back(Vector2f(n*(_controls[k+1].x - _controls[k].x), n*(_controls[k+1].y - _controls[k].y)));
	}
	
	n--;
	
	double x = 0, y = 0; //output coords
	
	double iN = 1; //at the end this will be i to the nth power
	
	double j = 1 - i; //additive inverse of i
	double jN = 1;
	for(int k = 0; k < n; k++) //jN is j to the nth power, will be 1 by the end
		jN *= j;
	for(int k = 0; k <= n; k++){
		double multiplier = BinomialCoefficient(n, k) * iN * jN; //middle values are multiplied by n, then in any case by the current powers of i and j
		iN *= i;
		jN /= j;
		
		x += q_controls[k].x * multiplier;
		y += q_controls[k].y * multiplier;
	}
	return Vector2f(x, y);
}

Vector2f BezierCurve::CalcLineLayer(double t, bool draw){
	//alternate implementation of Interpolate()
	
	int maxControl = _controls.size()-1; //do not operate on the last control, since it will be just an endpoint to a line
	if(maxControl == 0) //only one control, so there are no control sets
		return _controls[0];
	else if(maxControl == -1) //no controls
		return Vector2f(0, 0);
	
	Vector2f subControls[maxControl+1]; //array of control subsets. Will get smaller as the loop iterates, but its size is always the x sub group
	for(int i = 0; i <= maxControl; i++){ //copy all controls over as the initial set. must be copy by value, not reference, since subControls is significantly altered by execution
		subControls[i] = _controls[i];
	}
	
	for(; maxControl > 0; maxControl--){
		for(int i = 0; i < maxControl; i++){
			subControls[i].x += (subControls[i+1].x - subControls[i].x)*t;
			subControls[i].y += (subControls[i+1].y - subControls[i].y)*t;
			
			if(draw && i != 0){ //if the draw parameter was enabled, draw the line segments which are used to calculate the resultant point
				glBegin(GL_LINES);
				glColor4f(_animatedLineColor.r, _animatedLineColor.g, _animatedLineColor.b, _animatedLineColor.a);
				glVertex2f(subControls[i-1].x, subControls[i-1].y);
				glVertex2f(subControls[i].x, subControls[i].y);
				glEnd();
			}
		}
	}
	
	return subControls[0];
}

BezierCurve& BezierCurve::Regenerate(){
	if(_controls.size() != 0){ //no control points, no curve
		_points.clear();
		
		for(double t = 0; t < 1; t+=_throttle){
			
			Vector2f p = this->CalcLineLayer(t);
			
			if(_points.size() == 0 || p != _points.back()){ //only draw pixels inside the image space
				_points.push_back(p);
			}
		}
		_canvasTime = 0;
	}
	return *this;
}

BezierCurve::BezierCurve()
: _color(0,0,0), _controlColor(255, 0, 0), _boundingLineColor(0, 0, 255), _animatedLineColor(0, 255, 0), _throttle(0.00001), _vertexRadius(0.1), _scaleOffsets(0,0), _scaleFactor(1), _canvasTime(0){
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.f, 1.f, 1.f, 500.f);
	_controls.clear();
}

BezierCurve& BezierCurve::AddPoint(Vector2f p,  bool regen){
	_controls.push_back(p);
	if(regen)
		Regenerate();
	return *this;
}
BezierCurve& BezierCurve::AddPoint(int x, int y, bool regen){
	_controls.push_back(Vector2f(x, y));
	if(regen)
		Regenerate();
	return *this;
}
Vector2f BezierCurve::RemovePoint(Vector2f p, int range){
	return RemovePoint(p.x, p.y, range);
}
Vector2f BezierCurve::RemovePoint(int x, int y, int range){
	vector<Vector2f>::iterator it;
	int range_2 = range*range; //range^2, since the search uses pythagorean theorem
	
	for(it = _controls.begin(); it < _controls.end(); it++){
		if(((*it).x - x)*((*it).x - x) + ((*it).y - y)*((*it).y - y) <= range_2){ //distance formula. If the distance between this point and the clicked spot is less than 5, its within threshold
			_controls.erase(it);
			Regenerate();
			return *it;
		}
	}
	
	return Vector2f(-1, -1);
}
Vector2f BezierCurve::RemovePoint(int index){
	if(index >= (signed int) _controls.size()){ //if the index is too high, error by returning -1
		return Vector2f(-1, -1);
	}
	else if(index < 0){ //negative index is treated as that many from the top, so -1 is the last element
		index = _controls.size() + index;
	}
	
	Vector2f p = *(_controls.begin() + index); //store the point for returning
	
	//remove the point from the system
	_controls.erase(_controls.begin() + index);
	Regenerate();
	
	return p;
}

const vector<Vector2f>& BezierCurve::GetControls(){
	return _controls;
}
const vector<Vector2f>& BezierCurve::GetPoints(){
	return _points;
}

BezierCurve& BezierCurve::SetThrottle(double throttle){
	_throttle = throttle;
	Regenerate();
	return *this;
}
double BezierCurve::GetThrottle(){
	return _throttle;
}

BezierCurve& BezierCurve::SetVertexRadius(double radius){
	_vertexRadius = radius;
	return *this;
}
int BezierCurve::GetVertexRadius(){
	return _vertexRadius;
}

BezierCurve& BezierCurve::SetColor(Color4f color){
	_color = color;
	Regenerate();
	return *this;
}
Color4f BezierCurve::GetColor(){
	return _color;
}

BezierCurve& BezierCurve::SetControlColor(Color4f color){
	_controlColor = color;
	Regenerate();
	return *this;
}
Color4f BezierCurve::GetControlColor(){
	return _controlColor;
}

BezierCurve& BezierCurve::SetBoundingLineColor(Color4f color){
	_boundingLineColor = color;
	Regenerate();
	return *this;
}
Color4f BezierCurve::GetBoundingLineColor(){
	return _boundingLineColor;
}

BezierCurve& BezierCurve::Scale(double factor, Vector2f center){
	return this->Scale(center.x, center.y);
}

BezierCurve& BezierCurve::Offset(float centerX, float centerY){
	
	vector<Vector2f>::iterator it;
	
	for(it = _controls.begin(); it < _controls.end(); it++){
		it->x = (((it->x + _scaleOffsets.x)/_scaleFactor)) - centerX; //undo the last scale so that the point is completely unscaled, then scale it to the new factor
		it->y = (((it->y + _scaleOffsets.y)/_scaleFactor)) - centerY;
	}
	
	if(_canvasTime != 0){ //if an animation was in progress, start over and reanimate silently up to the point where the animation was paused
		double max_t = _canvasTime;
		for(double t = 0; t < max_t; t+=_throttle){
			CalcFrame(t);
		}
		CalcFrame(max_t);
	} else{
		Regenerate();
	}
	
	_scaleOffsets.x = centerX;
	_scaleOffsets.y = centerY;
	return *this;	
}

BezierCurve& BezierCurve::Scale(double factor, float centerX, float centerY){
	double offsetX = centerX*(factor-1);
	double offsetY = centerY*(factor-1);
	
	vector<Vector2f>::iterator it;
	
	for(it = _controls.begin(); it < _controls.end(); it++){
		it->x = (((it->x + _scaleOffsets.x)/_scaleFactor) * factor) - offsetX; //undo the last scale so that the point is completely unscaled, then scale it to the new factor
		it->y = (((it->y + _scaleOffsets.y)/_scaleFactor) * factor) - offsetY;
	}
	
	if(_canvasTime != 0){ //if an animation was in progress, start over and reanimate silently up to the point where the animation was paused
		double max_t = _canvasTime;
		for(double t = 0; t < max_t; t+=_throttle){
			CalcFrame(t);
		}
		CalcFrame(max_t);
	} else{
		Regenerate();
	}
	
	_scaleFactor = factor;
	_scaleOffsets.x = offsetX;
	_scaleOffsets.y = offsetY;
	return *this;
}


/*draw functions*/

void BezierCurve::InitDrawing(){
	
}

BezierCurve& BezierCurve::DrawCurve(){
	this->InitDrawing();
	
	glBegin(GL_LINE_STRIP);
	glColor4f(_color.r, _color.g, _color.b, _color.a);
	for(int i = 0; i < _points.size(); i++){
		glVertex3f(_points[i].x, _points[i].y, 0);
	}
	glEnd();
	
	return *this;
}
BezierCurve& BezierCurve::DrawControls(){
	InitDrawing();
	std::vector<Vector2f>::iterator it;
	glColor4f(_controlColor.r, _controlColor.g, _controlColor.b, _controlColor.a);
	for(it = _controls.begin(); it < _controls.end(); it++){
		glBegin(GL_POLYGON);
		for(int ang = 0; ang < 360; ang++){
			glVertex2f(_vertexRadius*cos(ang*PI/180) + it->x, _vertexRadius*sin(ang*PI/180) + it->y);
		}
		glEnd();
	}
	
	return *this;
}
BezierCurve& BezierCurve::DrawBoundingLines(){
	InitDrawing();
	std::vector<Vector2f>::iterator it;
	glLoadIdentity();
	glBegin(GL_LINE_STRIP);
	glColor4f(_boundingLineColor.r, _boundingLineColor.g, _boundingLineColor.b, _boundingLineColor.a);
	for(it = _controls.begin(); it < _controls.end(); it++){
		glVertex2f(it->x, it->y);
	}
	glEnd();
	
	return *this;
}
BezierCurve& BezierCurve::DrawCalcLines(){
	InitDrawing();
	Vector2f p = this->CalcLineLayer(_canvasTime, true);
	
	glBegin(GL_POLYGON);
	glColor4f(_controlColor.r, _controlColor.g, _controlColor.b, _controlColor.a);
	for(int ang = 0; ang < 360; ang++){
		glVertex2f(_vertexRadius*cos(ang*RADIANS) + p.x, _vertexRadius*sin(ang*RADIANS) + p.y);
	}
	glEnd();
	
	return *this;
}

BezierCurve& BezierCurve::CalcFrame(double t){
	if(_controls.size() != 0){ //do not do any animating with no control points
		
		if(t == 0){ //on the x time iteration the pixels must be cleared
			_points.clear();
		}
		
		Vector2f p = this->CalcLineLayer(t);
		
		if(_points.size() == 0 || p != _points.back()){ //only draw pixels inside the image space
			_points.push_back(p);
		}
		
	}
	_canvasTime = t; //store the time for later use as the most recently animated frame
	return *this;
}
BezierCurve& BezierCurve::Clear(){
	_scaleOffsets = Vector2f(0,0);
	_scaleFactor = 1;
	_controls.clear();
	_points.clear();
	return *this;
}