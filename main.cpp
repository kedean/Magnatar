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
	
	sf::RenderWindow App(sf::VideoMode(900, 900*sf::VideoMode::GetDesktopMode().Height/sf::VideoMode::GetDesktopMode().Width), "Magnatar", sf::Style::Close);
	//sf::Style::Fullscreen
	App.SetFramerateLimit(120);
	App.PreserveOpenGLStates(true);
	
	sf::View& view = App.GetDefaultView();
	
	Game currentGame("Game Loop", App, settings);
	
	Scene* currentScene = &currentGame;
	
	//Scene* currentScene = &currentGame;
	
    // Start game loop
    while (App.IsOpened())
    {
		// Process events
        sf::Event Event;
        while (App.GetEvent(Event))
        {
            if(Event.Type == sf::Event::Closed)
                App.Close();
			
			currentScene->HandleEvent(Event);
        }
		
		App.SetActive();
		App.Clear();
		
		currentScene->Loop();
		
        App.Display();
		
    }
	
    return EXIT_SUCCESS;
}
