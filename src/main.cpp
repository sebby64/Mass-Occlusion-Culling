#include <glew/glew.h>
#include <glew/wglew.h>

#include <SFML/Graphics.hpp>
#include <iostream>
#include <glm/glm.hpp>
#include "Utils.h"
#include "Globals.h"
#include "Cameras/Camera1stPerson.h"
#include "Transparency.h"

namespace
{
	glm::mat4 projectionMatrix;
	float fzNear;
    float fzFar;
    float frustumScale;
	int width;
	int height;

	GLuint modelViewProjectionUBO;
	GLuint modelViewProjectionUBOBindingIndex;

	Camera1stPerson camera;

	Transparency transparency;

}


void init()
{

	//init OpenGL
	glewInit();
	fzNear = .10f;
    fzFar = 1000.0f;
    float fieldOfViewDeg = 45.0f;
	float fFovRad = fieldOfViewDeg * DEGREE_TO_RAD;
    frustumScale = 1.0f / tan(fFovRad / 2.0f);

	projectionMatrix = glm::mat4(0.0f);
    projectionMatrix[0].x = frustumScale;
    projectionMatrix[1].y = frustumScale;
    projectionMatrix[2].z = (fzFar + fzNear) / (fzNear - fzFar);
    projectionMatrix[2].w = -1.0f;
    projectionMatrix[3].z = (2 * fzFar * fzNear) / (fzNear - fzFar);

	glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

	glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);

	//create model view projection UBO
	glGenBuffers(1, &modelViewProjectionUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, modelViewProjectionUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), NULL, GL_STREAM_DRAW);
	modelViewProjectionUBOBindingIndex = 0;
    glBindBufferBase(GL_UNIFORM_BUFFER, modelViewProjectionUBOBindingIndex, modelViewProjectionUBO);


	//init other things
	Globals::shaderState.initialize();
	Globals::meshLibrary.initialize();
	camera.activate();
	transparency.initTransparency();

}

void resize(int w, int h)
{
	projectionMatrix[0].x = frustumScale * (h / (float)w);
    projectionMatrix[1].y = frustumScale;

	width = w;
	height = h;

    glViewport(0, 0, (GLsizei) w, (GLsizei) h);

	transparency.reshapeTransparency(width, height);
}

void enterFrame()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
	glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	
	Globals::modelViewProjectionMatrix = projectionMatrix * Globals::viewMatrix;
	//orphaning
	glBindBuffer(GL_UNIFORM_BUFFER, modelViewProjectionUBO);
	glm::mat4* Pointer = (glm::mat4*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), 
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);

	*Pointer = Globals::modelViewProjectionMatrix;

	glUnmapBuffer(GL_UNIFORM_BUFFER);

	
	Globals::shaderState.prepareForRender();
	transparency.prepareTransparency(width, height);
	Globals::meshLibrary.render();
	transparency.finalizeTransparency();

}


/*-----------------------------
		Input/Output 
-----------------------------*/

int main (int argc, char **argv)
{
    // Create the main rendering window
	int width = 800;
	int height = 800;
	sf::RenderWindow* window = new sf::RenderWindow(sf::VideoMode(width, height), "Instanced Culling");
	
   // window->SetActive();
    //window->UseVerticalSync(true);
    //window->SetFramerateLimit(100);

	init();
	resize(width, height);

	bool mouseDown = false;
	int prevMouseX = 0;
	int prevMouseY = 0;


	int numFrames = 0;
	sf::Clock clock;

    while (window->IsOpened())
    {

		//framerate
		numFrames++;
		if(clock.GetElapsedTime() > 1.0f)
		{
			std::cout << "fps: " << numFrames << std::endl;
			numFrames = 0;
			clock.Reset();
		}

		enterFrame();
        window->Display();

		
		if(window->GetInput().IsKeyDown(sf::Key::W))
		{
			camera.zoom(2);
		}
		else if(window->GetInput().IsKeyDown(sf::Key::S))
		{
			camera.zoom(-1);
		}
		else if(window->GetInput().IsKeyDown(sf::Key::A))
		{
			camera.pan(-1, 0);
		}
		else if(window->GetInput().IsKeyDown(sf::Key::D))
		{
			camera.pan(1, 0);
		}


        sf::Event myEvent;
        while (window->GetEvent(myEvent))
        {
			switch(myEvent.Type)
			{
				case sf::Event::Resized:

					resize(window->GetWidth(), window->GetHeight());
					break;

				case sf::Event::MouseButtonPressed:

					if(myEvent.MouseButton.Button == sf::Mouse::Left)
					{
						mouseDown = true;
						prevMouseX = myEvent.MouseButton.X;
						prevMouseY = myEvent.MouseButton.Y;
					}
					break;

				case sf::Event::MouseButtonReleased:

					if(myEvent.MouseButton.Button == sf::Mouse::Left)
					{
						mouseDown = false;
					}
					break;

				case sf::Event::MouseMoved:

					if(mouseDown)
					{
						int x = myEvent.MouseMove.X;
						int y = myEvent.MouseMove.Y;

						int mouseXDiff = (x - prevMouseX);
						int mouseYDiff = (y - prevMouseY);

						float scaleFactor = .008f;
						float mouseXDifference = -mouseXDiff * scaleFactor;
						float mouseYDifference = -mouseYDiff * scaleFactor;
						camera.rotate(mouseXDifference,mouseYDifference);

						prevMouseX = x;
						prevMouseY = y;
					}
					break;

				case sf::Event::MouseWheelMoved:
					{
						int delta = myEvent.MouseWheel.Delta;
						float scaleFactor = 1.0f;
						camera.zoom(scaleFactor*delta);
					}
					break;
				case sf::Event::KeyPressed:

					break;

				case sf::Event::Closed:

					window->Close();
					break;
			}
        }
    }
}

