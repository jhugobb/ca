#include <GL/glew.h>
#include <GL/glut.h>
#include "Application.h"


void Application::init(int argc, char** argv)
{
	glClearColor(1.f, 1.f, 1.f, 1.0f);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	bPlay = scene.init(argc, argv);
	
	for(unsigned int i=0; i<256; i++)
	{
	  keys[i] = false;
	  specialKeys[i] = false;
	}
	mouseButtons[0] = false;
	mouseButtons[1] = false;
	lastMousePos = glm::ivec2(-1, -1);
}

bool Application::loadMesh(const char *filename)
{
  return scene.loadMesh(filename);
}

bool Application::update(int deltaTime)
{
	scene.update(deltaTime, keys[119], keys[115], keys[97], keys[100], bPlay);
	
	return bPlay;
}

void Application::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (bPlay)
		scene.render();
}

void Application::resize(int width, int height)
{
  glViewport(0, 0, width, height);
  scene.getCamera().resizeCameraViewport(width, height);
}

void Application::keyPressed(int key)
{
	switch (key) {
		case 27:
			bPlay = false;
			break;
		case 109:
			scene.toggleParticleMode();
			break;
		case 117:
			scene.toggleUpdateMode();
			break;
		case 98:
			scene.toggleBending();
			break;
		case 121:
			scene.toggleStretch();
			break;
		case 105:
			scene.toggleShear();
			break;
		case 114:
			scene.resetScene(false);
			break;
	}
	keys[key] = true;
}

void Application::keyReleased(int key)
{
	keys[key] = false;
}

void Application::specialKeyPressed(int key)
{
	specialKeys[key] = true;
}

void Application::specialKeyReleased(int key)
{
	specialKeys[key] = false;
	if(key == GLUT_KEY_F1)
	  scene.switchPolygonMode();
}

void Application::mouseMove(int x, int y)
{
  // Rotation
	if(mouseButtons[0] && lastMousePos.x != -1)
	  scene.getCamera().rotateCamera(0.5f * (y - lastMousePos.y), 0.5f * (x - lastMousePos.x));

	// Zoom
	if(mouseButtons[1] && lastMousePos.x != -1)
	  scene.getCamera().zoomCamera(0.01f * (y - lastMousePos.y));

 	lastMousePos = glm::ivec2(x, y);
}

void Application::mousePress(int button)
{
  mouseButtons[button] = true;
}

void Application::mouseRelease(int button)
{
  mouseButtons[button] = false;
  if(!mouseButtons[0] && !mouseButtons[1])
    lastMousePos = glm::ivec2(-1, -1);
}

bool Application::getKey(int key) const
{
	return keys[key];
}

bool Application::getSpecialKey(int key) const
{
	return specialKeys[key];
}

void Application::cleanUpScene() {
	scene.cleanup();
}

void Application::setDelta(double delta) {
	scene.setDelta(delta);
}

