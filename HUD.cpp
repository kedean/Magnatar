#include "HUD.h"

sf::Drawable* kd::HUD::AddWidget(string label, sf::Drawable* item, sf::Vector2f position, float lifetime){
	AddWidgetRef(label, item, position, lifetime);
	_managed[label] = item;
	return item;
}
sf::Drawable* kd::HUD::AddWidgetRef(string label, sf::Drawable* item, sf::Vector2f position, float lifetime){
	map<string, sf::Drawable*>::iterator ifExists = _widgets.find(label);
	if(ifExists != _widgets.end()){
		RemoveWidget(label);
	}
	
	_widgets.insert(_widgets.end(), pair<string, sf::Drawable*>(label, item));
	_widgets[label]->SetPosition(position);
	if(lifetime != 0)
		_mortals[item] = _internal_clock.GetElapsedTime() + lifetime;
	_widgetsOrdered.push_back(item);
	return item;
}
kd::HUD& kd::HUD::RemoveWidget(string label){
	map<string, sf::Drawable*>::iterator ifManaged = _managed.find(label);
	if(ifManaged != _widgets.end()){ //widgets marked as managed are freed before being erased
		delete ifManaged->second;
		_managed.erase(ifManaged);
	}
	
	map<string, sf::Drawable*>::iterator drawableIt = _widgets.find(label);
	sf::Drawable* drawablePointer = drawableIt->second;
	_widgets.erase(drawableIt);
	vector<sf::Drawable*> newWidgetsOrdered;
	for(int i = 0; i < _widgetsOrdered.size(); i++){
		if(drawablePointer != _widgetsOrdered[i])
			newWidgetsOrdered.push_back(_widgetsOrdered[i]);
	}
	_widgetsOrdered.swap(newWidgetsOrdered);
	return *this;
}
void kd::HUD::Render(sf::RenderTarget& target) const{
	vector<sf::Drawable*>::const_iterator it;
	for(it = _widgetsOrdered.begin(); it < _widgetsOrdered.end(); it++){
		map<sf::Drawable*, float>::const_iterator life = _mortals.find(*it);
		if(life == _mortals.end() || (*life).second > _internal_clock.GetElapsedTime()){
			target.Draw(**(it));
		}
	}
}
void kd::HUD::CleanWidgets(){
	map<string, sf::Drawable*>::iterator it;
	for(it = _widgets.begin(); it != _widgets.end(); it++){
		map<sf::Drawable*, float>::iterator life = _mortals.find(it->second);
		if(life != _mortals.end() && life->second <= _internal_clock.GetElapsedTime()){
			RemoveWidget(it->first);
			return CleanWidgets(); //tail recursive method to prevent the iterator from becoming bad, redoing the method will reset the iterator (which is now invalid)
		}
	}
}