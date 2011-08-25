#include "Scene.h"
#define HIGHLIGHT_SPEED 2

Menu::Menu(string id, sf::RenderWindow& application, json_spirit::mObject& settings) : Scene(id, application, settings){
	_background = sf::Shape::Rectangle(0,0,application.GetWidth(), application.GetHeight(),sf::Color(0, 0, 0));
	
	_textToRender.push_back(sf::String("Magnatar"));
	_textToRender.back().SetPosition((_application.GetWidth() - _textToRender[0].GetCharacterPos(8).x)/2, 100);
	
	_textToRender.push_back(sf::String("New Game", sf::Font::GetDefaultFont(), 20));
	_textToRender.back().SetPosition((_application.GetWidth() - _textToRender[1].GetCharacterPos(8).x)/2, 300);
	_textToRender.back().SetColor(sf::Color(255, 255, 255, 100));
	
	_textToRender.push_back(sf::String("Quit", sf::Font::GetDefaultFont(), 20));
	_textToRender.back().SetPosition((_application.GetWidth() - _textToRender[1].GetCharacterPos(4).x)/2, 350);
	_textToRender.back().SetColor(sf::Color(255, 255, 255, 100));
	
	_highlightIndex = 1;
	_highlightAlpha = 255.f;
	_highlightDir = -HIGHLIGHT_SPEED;
}
void Menu::Loop(){
	_application.Draw(_background);
	
	for(int i = 0; i < _textToRender.size(); i++){
		if(_highlightIndex == i){ //selected item in the menu
			_highlightAlpha += _highlightDir;
			if(_highlightAlpha >= 255.f)
				_highlightDir = -HIGHLIGHT_SPEED;
			else if(_highlightAlpha <= 100.f)
				_highlightDir = HIGHLIGHT_SPEED;
			_textToRender[i].SetColor(sf::Color(255, 255, 255, _highlightAlpha));
		}
		_application.Draw(_textToRender[i]);
	}
}
Scene* Menu::HandleEvent(sf::Event event){
	if(event.Key.Code == sf::Key::Space || event.Key.Code == sf::Key::Return){
		if(_highlightIndex == 1){
			_application.SetActive();
			_application.Clear();
			sf::String loading("Loading...", sf::Font::GetDefaultFont(), 20);
			loading.SetPosition(_application.GetWidth()/2 - loading.GetCharacterPos(17).x/2, _application.GetHeight()/2 - 50);
			_application.Draw(loading);
			_application.Display();
			return (new Game("Game Loop", _application, _settings));
		}
		else if(_highlightIndex == 2){
			return NULL;
		}
	}
	else if(event.Key.Code == sf::Key::Down){
		if(_highlightIndex+1 < _textToRender.size()){
			_textToRender[_highlightIndex].SetColor(sf::Color(255, 255, 255, 100));
			_highlightIndex++;
			_highlightAlpha = 255;
			_highlightDir = -HIGHLIGHT_SPEED;
		}
	}
	else if(event.Key.Code == sf::Key::Up){
		if(_highlightIndex > 1){
			_textToRender[_highlightIndex].SetColor(sf::Color(255, 255, 255, 100));
			_highlightIndex--;
			_highlightAlpha = 255;
			_highlightDir = -HIGHLIGHT_SPEED;
		}
	}
	return this;
}