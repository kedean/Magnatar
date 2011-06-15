#include "Bezier.h"

using namespace kd;

void BezierSpline::Regenerate(){
	std::vector<BezierCurve>::iterator it;
	for(it = _curves.begin(); it < _curves.end(); it++){
		it->Regenerate();
	}
}

BezierSpline::BezierSpline()
: _color(0,0,0), _controlColor(255, 0, 0), _boundingLineColor(0, 0, 255), _animatedLineColor(0, 255, 0), _throttle(0.00001), _vertexRadius(0.1), _scaleOffsets(0,0), _scaleFactor(1), _canvasTime(0){
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.f, 1.f, 1.f, 500.f);
	this->AddCutoff();
}

BezierSpline& BezierSpline::AddCutoff(){
	_curves.push_back(BezierCurve().SetColor(_color).SetControlColor(_controlColor).SetBoundingLineColor(_boundingLineColor).SetThrottle(_throttle).SetVertexRadius(_vertexRadius).Scale(_scaleFactor, _scaleOffsets));
	return *this;
}

BezierSpline& BezierSpline::AddPoint(Vector2f p,  bool regen){
	_curves.back().AddPoint(p, regen);
	_controls.push_back(p);
	return *this;
}
BezierSpline& BezierSpline::AddPoint(int x, int y, bool regen){
	_curves.back().AddPoint(x, y, regen);
	_controls.push_back(Vector2f(x, y));
	return *this;
}
Vector2f BezierSpline::RemovePoint(Vector2f p, int range){
	return RemovePoint(p.x, p.y, range);
}
Vector2f BezierSpline::RemovePoint(int x, int y, int range){
	return _curves.back().RemovePoint(x, y, range);
}
Vector2f BezierSpline::RemovePoint(int index){
	return _curves.back().RemovePoint(index);
}

BezierCurve* BezierSpline::GetCurve(int i){
	return &(_curves[i]);
}
vector<BezierCurve>& BezierSpline::GetCurves(){
	return _curves;
}
const vector<Vector2f>& BezierSpline::GetControls(){
	return _controls;
}
const vector<Vector2f>& BezierSpline::GetPoints(){
	_points.clear();
	
	//merge all point sets
	std::vector<BezierCurve>::iterator it;
	for(it = _curves.begin(); it < _curves.end(); it++){
		std::vector<Vector2f> nPoints = it->GetPoints();
		std::vector<Vector2f>::iterator nIt;
		for(nIt = nPoints.begin(); nIt < nPoints.end(); nIt++)
			_points.push_back(*nIt);
	}
	return _points;
}

BezierSpline& BezierSpline::SetThrottle(double throttle){
	_throttle = throttle;
	std::vector<BezierCurve>::iterator it;
	for(it = _curves.begin(); it < _curves.end(); it++)
		it->SetThrottle(_throttle);
	return *this;
}
double BezierSpline::GetThrottle(){
	return _throttle;
}

BezierSpline& BezierSpline::SetVertexRadius(double radius){
	_vertexRadius = radius;
	std::vector<BezierCurve>::iterator it;
	for(it = _curves.begin(); it < _curves.end(); it++)
		it->SetVertexRadius(_vertexRadius);
	return *this;
}
int BezierSpline::GetVertexRadius(){
	return _vertexRadius;
}

BezierSpline& BezierSpline::SetColor(Color4f color){
	_color = color;
	std::vector<BezierCurve>::iterator it;
	for(it = _curves.begin(); it < _curves.end(); it++)
		it->SetColor(_color);
	return *this;
}
Color4f BezierSpline::GetColor(){
	return _color;
}

BezierSpline& BezierSpline::SetControlColor(Color4f color){
	_controlColor = color;
	std::vector<BezierCurve>::iterator it;
	for(it = _curves.begin(); it < _curves.end(); it++)
		it->SetControlColor(_controlColor);
	return *this;
}
Color4f BezierSpline::GetControlColor(){
	return _controlColor;
}

BezierSpline& BezierSpline::SetBoundingLineColor(Color4f color){
	_boundingLineColor = color;
	std::vector<BezierCurve>::iterator it;
	for(it = _curves.begin(); it < _curves.end(); it++)
		it->SetBoundingLineColor(_boundingLineColor);
	return *this;
}
Color4f BezierSpline::GetBoundingLineColor(){
	return _boundingLineColor;
}

BezierSpline& BezierSpline::Scale(double factor, Vector2f center){
	return this->Scale(center.x, center.y);
}

BezierSpline& BezierSpline::Offset(float centerX, float centerY){
	std::vector<BezierCurve>::iterator it;
	for(it = _curves.begin(); it < _curves.end(); it++)
		it->Offset(centerX, centerY);
	return *this;	
}

BezierSpline& BezierSpline::Scale(double factor, float centerX, float centerY){
	std::vector<BezierCurve>::iterator it;
	for(it = _curves.begin(); it < _curves.end(); it++)
		it->Scale(factor, centerX, centerY);
	return *this;
}


/*draw functions*/

BezierSpline& BezierSpline::DrawCurve(){
	std::vector<BezierCurve>::iterator it;
	for(it = _curves.begin(); it < _curves.end(); it++)
		it->DrawCurve();
	
	return *this;
}
BezierSpline& BezierSpline::DrawControls(){
	std::vector<BezierCurve>::iterator it;
	for(it = _curves.begin(); it < _curves.end(); it++)
		it->DrawControls();
	return *this;
}
BezierSpline& BezierSpline::DrawBoundingLines(){
	std::vector<BezierCurve>::iterator it;
	for(it = _curves.begin(); it < _curves.end(); it++)
		it->DrawBoundingLines();
	return *this;
}
BezierSpline& BezierSpline::DrawCalcLines(){
	std::vector<BezierCurve>::iterator it;
	for(it = _curves.begin(); it < _curves.end(); it++)
		it->DrawCalcLines();
	return *this;
}

BezierSpline& BezierSpline::Clear(){
	_scaleOffsets = Vector2f(0,0);
	_scaleFactor = 1;
	_controls.clear();
	_points.clear();
	_curves.clear();
	return *this;
}