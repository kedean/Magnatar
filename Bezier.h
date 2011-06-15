#ifndef BEZIER_H
#define BEZIER_H

#include<vector>
#ifdef __APPLE__
#include<OpenGL/OpenGL.h>
#include<OpenGL/glu.h>
#else
#include<GL/GL.h>
#include<GL/glu.h>
#endif
#include<cmath>

using namespace std;

namespace kd{
	
	typedef struct Vector2f{
		float x, y;
		Vector2f(float _x=0, float _y=0) : x(_x), y(_y){};
		bool operator !=(const Vector2f& that){
			return !(*this == that);
		}
		bool operator ==(const Vector2f& that){
			return (this->x == that.x && this->y == that.y);
		}
	} Vector2f;
	
	typedef struct Color4f{
		int r, g, b, a;
		Color4f(int _r=255, int _g=255, int _b=255, int _a=255)
		: r(_r), g(_g), b(_b), a(_a){
		}
	} Color4f;
	
#ifndef PI
#define PI 3.14159265
#endif
	
#ifndef RADIANS
#define RADIANS PI/180.f
#endif
	
	inline int Factorial(int k){
		int result = 1;
		int i;
		for (i = 1; i <= k; i++){
			result *= i;
		}
		
		return result;
	}
	inline double BinomialCoefficient(int n, int i){
		return Factorial(n) / (double)(Factorial(i) * Factorial(n - i));
	}
	
	inline float ProjectX(int xValue, int fov, int screenWidth, float zNear){
		float h = 2.0f*zNear*tan(fov/2*RADIANS);
		float w = 1.33333333f*h;
		return float(xValue)/float(screenWidth)*w-w/2;
	}
	
	inline float ProjectY(int yValue, int fov, int screenHeight, float zNear){
		float h = 2.0f*zNear*tan(fov/2*RADIANS);
		return float(screenHeight-yValue)/float(screenHeight)*h-h/2;
	}
	
#define Color3f Color4f
	
	class BezierCurve{
	private:
		vector<Vector2f> _controls; //list of control points
		vector<Vector2f> _points; //list of points that define the curve itself
		
		double _throttle; //controls detail level of the curve, setting this very low results in a series of angled lines. Very high gives the illusion of round corners
		double _vertexRadius; //size of drawn vertices
		
		Color4f _color, _animatedLineColor, _boundingLineColor, _controlColor;
		
		double _canvasTime; //time interval of the most recent call to CalcFrame() that was not interrupted by DrawCurve()
		
		/*variables for controlling the zoom/scale*/
		double _scaleFactor;
		Vector2f _scaleOffsets;
		
		void InitDrawing(); //Sets up opengl drawing contexts
		
	public:
		BezierCurve();
		
		/*display methods*/
		
		BezierCurve& DrawCurve(); //draw the current curve to the canvas
		BezierCurve& DrawControls(); //draw the control points to the canvas as 5px radius circles
		BezierCurve& DrawBoundingLines(); //draw the lines connecting the control points to the canvas
		BezierCurve& DrawCalcLines(); //draw the lines used to calculate the current curve time. When used after a call to Generate(), this will produce no useful results.
		
		/*line calculation*/
		
		BezierCurve& Regenerate(); //Regenerates the points of the curve. Internal only, foruse after calls that alter the control point structure
		
		BezierCurve& CalcFrame(double t); //same function as generate, except only up to the given time t. If drawing is enabled, it will also illustrate the steps used to animate
		Vector2f CalcLineLayer(double t, bool draw=false); //calculate the point at time interval t by recursively (technically used a for loop instead of recursion) performing intersections of lines in the control set. The final intersection is the result. Capable of illustrating the de Casteljau method
		Vector2f Interpolate(double i); //Mathematically calculates the point located at the time interval i using the Bezier formula, slightly faster than CalcLineLayer
		Vector2f Derive(double i); //Mathematically calculates the point located at time interval i on the first derivative of the curve
		
		/*control point manipulation*/
		
		BezierCurve& AddPoint(Vector2f p, bool regen=true); //add the point p as a control
		BezierCurve& AddPoint(int x, int y, bool regen=true); //add the point (x, y) as a control
		Vector2f RemovePoint(Vector2f p, int range=0); //Removes the closest point within the given range of p
		Vector2f RemovePoint(int x, int y, int range=0); //Removes the closet point within the given range of (x, y)
		Vector2f RemovePoint(int index); //Removes the point at the given index. Negative indices indicate a distance from the top, so -1 is the most recent
		
		/*access methods*/
		
		const vector<Vector2f>& GetControls(); //returns a vector of the control points
		const vector<Vector2f>& GetPoints(); //returns a vector of all pixels currently occupied by the curve
		
		/*attribute manipulation*/
		
		BezierCurve& SetThrottle(double throttle); //alter the 'throttle', in the form 1/n, where n is the number of points on the curve to be drawn
		double GetThrottle();
		
		BezierCurve& SetVertexRadius(double radius);
		int GetVertexRadius();
		
		BezierCurve& SetColor(Color4f color); //alter the color of the curve drawn
		Color4f GetColor();
		
		BezierCurve& SetControlColor(Color4f color);
		Color4f GetControlColor();
		
		BezierCurve& SetBoundingLineColor(Color4f color);
		Color4f GetBoundingLineColor();
		
		BezierCurve& SetSize(int width, int height); //alter the size of the canvas
		BezierCurve& Offset(float centerX, float centerY);
		BezierCurve& Scale(double factor, Vector2f center=Vector2f(0,0)); //scale the points by the given factor and centered on the given center, used for zooming. The factor is relative to the original set of points
		BezierCurve& Scale(double factor, float centerX=0, float centerY=0);
		
		BezierCurve& Clear(); //erase all control points and curve points
	};

	class BezierSpline{
	private:
		vector<BezierCurve> _curves; //stores the component curves
		
		vector<Vector2f> _controls; //list of control points
		vector<Vector2f> _points; //list of points that define the curve itself
		
		double _throttle; //controls detail level of the curve, setting this very low results in a series of angled lines. Very high gives the illusion of round corners
		double _vertexRadius; //size of drawn vertices
		
		Color4f _color, _animatedLineColor, _boundingLineColor, _controlColor;
		
		double _canvasTime; //time interval of the most recent call to CalcFrame() that was not interrupted by DrawCurve()
		
		/*variables for controlling the zoom/scale*/
		double _scaleFactor;
		Vector2f _scaleOffsets;
	public:
		BezierSpline();
		/*display methods*/
		
		BezierSpline& DrawCurve(); //draw the current curve to the canvas
		BezierSpline& DrawControls(); //draw the control points to the canvas as 5px radius circles
		BezierSpline& DrawBoundingLines(); //draw the lines connecting the control points to the canvas
		BezierSpline& DrawCalcLines(); //draw the lines used to calculate the current curve time. When used after a call to Generate(), this will produce no useful results.
		
		/*line calculation*/
		
		void Regenerate(); //Regenerates the points of the curve. Internal only, foruse after calls that alter the control point structure
		
		/*curve manipulation*/
		
		BezierSpline& AddCutoff();
		
		/*control point manipulation*/
		
		BezierSpline& AddPoint(Vector2f p, bool regen=true); //add the point p as a control
		BezierSpline& AddPoint(int x, int y, bool regen=true); //add the point (x, y) as a control
		Vector2f RemovePoint(Vector2f p, int range=0); //Removes the closest point within the given range of p
		Vector2f RemovePoint(int x, int y, int range=0); //Removes the closet point within the given range of (x, y)
		Vector2f RemovePoint(int index); //Removes the point at the given index. Negative indices indicate a distance from the top, so -1 is the most recent
		
		/*access methods*/
		
		vector<BezierCurve>& GetCurves(); //returns a vector of the component curves
		BezierCurve* GetCurve(int i);
		const vector<Vector2f>& GetControls(); //returns a vector of the control points
		const vector<Vector2f>& GetPoints(); //returns a vector of all pixels currently occupied by the curve
		
		/*attribute manipulation*/
		
		BezierSpline& SetThrottle(double throttle); //alter the 'throttle', in the form 1/n, where n is the number of points on the curve to be drawn
		double GetThrottle();
		
		BezierSpline& SetVertexRadius(double radius);
		int GetVertexRadius();
		
		BezierSpline& SetColor(Color4f color); //alter the color of the curve drawn
		Color4f GetColor();
		
		BezierSpline& SetControlColor(Color4f color);
		Color4f GetControlColor();
		
		BezierSpline& SetBoundingLineColor(Color4f color);
		Color4f GetBoundingLineColor();
		
		BezierSpline& SetSize(int width, int height); //alter the size of the canvas
		BezierSpline& Offset(float centerX, float centerY);
		BezierSpline& Scale(double factor, Vector2f center=Vector2f(0,0)); //scale the points by the given factor and centered on the given center, used for zooming. The factor is relative to the original set of points
		BezierSpline& Scale(double factor, float centerX=0, float centerY=0);
		
		BezierSpline& Clear(); //erase all control points and curve points		
	};
}
#endif