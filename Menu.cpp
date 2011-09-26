#include "Scene.h"
#define HIGHLIGHT_SPEED 2
#define HIGHLIGHT_MIN 150.f
#define MENU_DEBOUNCE 0.2f //time in seconds between menu shifts, if less than this time has passed since the last arrow key hit, it will not register

Menu::Menu(string id, sf::RenderWindow& application, json_spirit::mObject& settings) : Scene(id, application, settings){
	_background = sf::Shape::Rectangle(0,0,application.GetWidth(), application.GetHeight(),sf::Color(0, 0, 0));
	
	_fonts.push_back(sf::Font());
	_fonts.push_back(sf::Font());
	
	if(!_fonts[0].LoadFromFile("Hydrogen.ttf", 60))
		_fonts[0] = sf::Font::GetDefaultFont();
	
	if(!_fonts[1].LoadFromFile("Times New Roman.ttf", 60))
		_fonts[1] = sf::Font::GetDefaultFont();
	
	_textToRender.push_back(sf::String("Magnatar", _fonts[0], 60));
	_textToRender.back().SetPosition((_application.GetWidth() - _textToRender[0].GetCharacterPos(8).x)/2, 100);
	
	
	//Generate the menu items
	
	string menuItems[] = {"New Game", "Create Server", "Connect To Server", "Settings", "Quit"};
	
	for(int i = 0; i < 5; i++){
		_textToRender.push_back(sf::String(menuItems[i], _fonts[0], 30));
		_textToRender.back().SetPosition((_application.GetWidth() - _textToRender[i+1].GetCharacterPos(menuItems[i].size() - 1).x)/2, 250 + i*50);
		_textToRender.back().SetColor(sf::Color(255, 255, 255, 100));
	}
	
	_highlightIndex = 1;
	_highlightAlpha = 255.f;
	_highlightDir = -HIGHLIGHT_SPEED;
}
void Menu::Loop(){
	_application.Draw(_background);
	
	//loop renders all menu items, with the selected one rendred as flashing from grey to white
	for(int i = 0; i < _textToRender.size(); i++){
		if(_highlightIndex == i){ //selected item in the menu
			_highlightAlpha += _highlightDir;
			if(_highlightAlpha >= 255.f)
				_highlightDir = -HIGHLIGHT_SPEED;
			else if(_highlightAlpha <= HIGHLIGHT_MIN)
				_highlightDir = HIGHLIGHT_SPEED;
			
			_textToRender[i].SetColor(sf::Color(255, 255, 255, _highlightAlpha));
		}
		_application.Draw(_textToRender[i]);
	}
	
	_application.Draw(_HUD);
	_HUD.CleanWidgets();
}
Scene* Menu::HandleEvent(sf::Event event){
	
	if(event.Key.Code == sf::Key::Space || event.Key.Code == sf::Key::Return){
		if(_highlightIndex == 1){
			_application.SetActive();
			_application.Clear();
			sf::String loading("...Loading...", _fonts[0], 40);
			loading.SetPosition(_application.GetWidth()/2 - loading.GetCharacterPos(20).x/2, _application.GetHeight()/2 - 50);
			_application.Draw(loading);
			_application.Display();
	 		return (new Game("Game Loop", _application, _settings));
		}
		else if(_highlightIndex == 5){
			return NULL;
		}
		
		else{ //display "Feature not yet implemented" whenever a non-working button is hit
			_HUD.AddWidget("Unimplemented", 
						   new sf::String("Feature not yet implemented", _fonts[1], 30),
						   sf::Vector2f(10, (_application.GetDefaultView().GetHalfSize().y * 2) - 50),
						   1);
		}
	}
	else if(event.Key.Code == sf::Key::Down){
		if(_tapTime.GetElapsedTime() > MENU_DEBOUNCE){ //todo: alter the debounce so that it doesnt affect multiple keypresses
			_tapTime.Reset();
			_textToRender[_highlightIndex].SetColor(sf::Color(255, 255, 255, 100));
			if(_highlightIndex+1 < _textToRender.size())
				_highlightIndex++;
			else
				_highlightIndex = 1;
			_highlightAlpha = 255;
			_highlightDir = -HIGHLIGHT_SPEED;
		}
	}
	else if(event.Key.Code == sf::Key::Up){
		if(_tapTime.GetElapsedTime() > MENU_DEBOUNCE){
			_tapTime.Reset();
			_textToRender[_highlightIndex].SetColor(sf::Color(255, 255, 255, 100));
			if(_highlightIndex > 1)
				_highlightIndex--;
			else
				_highlightIndex = _textToRender.size()-1;
			_highlightAlpha = 255;
			_highlightDir = -HIGHLIGHT_SPEED;
		}
	}
	return this;
}