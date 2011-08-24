#pragma once

#include<map>
#include<vector>
#include<string>
#include<SFML/Graphics.hpp>

using namespace std;

/*
 Class for storing and displaying HUD (heads up display) components.
 The HUD is automatically positioned to (0, 0), and each component is drawn relative to the screen origin, not the view origin.
 */

namespace kd{
	class HUD : public sf::Drawable{
	private:
		map<string, sf::Drawable*> _widgets; //map of widgets
		vector<sf::Drawable*> _widgetsOrdered;
	public:
		~HUD(); //HUD will manage all memory given to it. Do not attempt to delete any objects given via AddWidget.
		sf::Drawable* AddWidget(string label, sf::Drawable* item, sf::Vector2f position=sf::Vector2f(0,0));
		kd::HUD& RemoveWidget(string label);
		
		sf::Drawable* operator[] (string index){
			return (*(_widgets.find(index))).second;
		}
		
		virtual void Render(sf::RenderTarget& target) const;
	};
	
	typedef HUD HeadsUpDisplay;
};