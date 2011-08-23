#include "HUD.h"

kd::HUD& kd::HUD::AddWidget(string label, sf::Drawable* item, sf::Vector2f position){
	_widgets.insert(_widgets.end(), pair<string, sf::Drawable*>(label, item));
	_widgets[label]->SetPosition(position);
	
	_widgetsOrdered.push_back(item);
	return *this;
}
kd::HUD& kd::HUD::RemoveWidget(string label){
	_widgets.erase(label);
	return *this;
}

void kd::HUD::Render(sf::RenderTarget& target) const{
	vector<sf::Drawable*>::const_iterator it;
	for(it = _widgetsOrdered.begin(); it < _widgetsOrdered.end(); it++){
		target.Draw(**(it));
	}
}