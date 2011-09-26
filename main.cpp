#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "Helpers.h"
#include "Scene.h"

#include<json_spirit.h>
#include<fstream>
#include<iostream>

int main(){
	json_spirit::mObject settings;
	ReadSettings(&settings);
	bool fullscreenMode = true;
	json_spirit::mValue fullscreenCheck = settings["fullscreen"];
	if(fullscreenCheck.type() == json_spirit::bool_type)
		fullscreenMode = fullscreenCheck.get_bool();
	
	sf::RenderWindow App(sf::VideoMode(900, 900*sf::VideoMode::GetDesktopMode().Height/sf::VideoMode::GetDesktopMode().Width), "Magnatar", 
						 (fullscreenMode ? sf::Style::Close|sf::Style::Fullscreen : sf::Style::Close));
	
	App.SetFramerateLimit(120);
	App.PreserveOpenGLStates(true);
	
	sf::View& view = App.GetDefaultView();
	
	Scene* currentScene = new Menu("Main Menu", App, settings);
	
	while (App.IsOpened())
    {
		// Events that affect the full application scope
        sf::Event Event;
        while (App.GetEvent(Event))
        {
            if(Event.Type == sf::Event::Closed)
                App.Close();
			
			Scene* newScene = currentScene->HandleEvent(Event);
			if(newScene != NULL && newScene != currentScene){
				delete currentScene;
				currentScene = newScene;
			}
			else if(newScene == NULL){
				App.Close();
			}
        }
		
		App.SetActive();
		App.Clear();
		
		currentScene->Loop();
		
        App.Display();
		
    }
	
    return EXIT_SUCCESS;
}
