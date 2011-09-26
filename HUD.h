#pragma once

#include<iostream>
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
		sf::Clock _internal_clock;
		map<sf::Drawable*, float> _mortals; //map of pointers to their lifetime, if applicable
		map<string, sf::Drawable*> _widgets; //map of widgets
		map<string, sf::Drawable*> _managed; //map of widgets whose should be memory managed (deleted when their life is over)
		vector<sf::Drawable*> _widgetsOrdered;
	public:
		~HUD(){
			map<string, sf::Drawable*>::iterator managedIt;
			for(managedIt = _managed.begin(); managedIt != _managed.end(); managedIt++){
				delete managedIt->second;
			}
		}
		sf::Drawable* AddWidgetRef(string label, sf::Drawable* item, sf::Vector2f position=sf::Vector2f(0,0), float lifetime=0);
		sf::Drawable* AddWidget(string label, sf::Drawable* item, sf::Vector2f position=sf::Vector2f(0,0), float lifetime=0);
		
		kd::HUD& RemoveWidget(string label);
		
		sf::Drawable* operator[] (string index){
		//	std::cout << index << " " << (_widgets.find(index))->second << std::endl;
			return (*(_widgets.find(index))).second;
		}
		
		virtual void Render(sf::RenderTarget& target) const;
		
		void CleanWidgets(); //call regularly to keep track of expired widget memory
	};
	
	typedef HUD HeadsUpDisplay;
};