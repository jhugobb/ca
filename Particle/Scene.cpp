#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <ctime>
#include <iostream>

#include "Scene.h"
#include "PLYReader.h"
#include "LOD.h"

#define GLM_FORCE_RADIANS

Scene::Scene()
{	
	//bottom
	glm::vec3 punt(0.0f, -5.0f, 0.0f);
	glm::vec3 normal(0, 1, 0);
	planes.push_back(new Plane(punt, normal));

	//top
	glm::vec3 puntt(0.0f, 5.0f, 0.0f);
	glm::vec3 normalt(0, -1, 0);
	planes.push_back(new Plane(puntt, normalt));

	//back
	glm::vec3 puntb(0.0f, 0.0f, -5.0f);
	glm::vec3 normalb(0, 0, 1);
	planes.push_back(new Plane(puntb, normalb));

	//front
	glm::vec3 puntf(0.0f, 0.0f, 5.0f);
	glm::vec3 normalf(0, 0, -1);
	planes.push_back(new Plane(puntf, normalf));

	//left
	glm::vec3 puntl(5.0f, 0.0f, 0.0f);
	glm::vec3 normall(-1, 0, 0);
	planes.push_back(new Plane(puntl, normall));

	//right
	glm::vec3 puntr(-5.0f, 0.0f, 0.0f);
	glm::vec3 normalr(1, 0, 0);
	planes.push_back(new Plane(puntr, normalr));

	for (unsigned int i = 0; i < 500; i++) {
		Particle* part = new Particle(0,1,0);
		particles.push_back(part);
		std::vector<float> dists;
		for (Plane* p : planes) {
			dists.push_back(p->distPoint2Plane(part->getCurrentPosition()));
		}
		curr_plane_dist.push_back(dists);
		prev_plane_dist.push_back(dists);
	}

	particle_mesh = new TriangleMesh();
	cave = new TriangleMesh();
	particle_mesh->buildCube(0.05, false, glm::vec3(0,1,0));
	cave->buildCube(5, true, glm::vec3(0.0f));
	delta = 0;
	framerate = 0;
}

Scene::~Scene()
{

}

std::vector<std::string> split(const std::string & str, const std::string & delim) {
    std::vector<std::string> tokens;
    unsigned int prev = 0, pos = 0;
    do {
        pos = str.find(delim, prev);
        if (pos == std::string::npos) pos = str.length();
        std::string token = str.substr(prev, pos - prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
    } while (pos < str.length() && prev < str.length());
    return tokens;
}

float rand_f(float a, float b) {
    return ((b - a) * ((float)rand() / RAND_MAX)) + a;
}


bool Scene::init(int argc, char* argv[])
{
	initShaders();

	vector<string> args;
	for (int i = 0; i < argc; i++) {
		args.push_back(string(argv[i]));
	}

	for (Particle* p : particles) {
		p->setForce(0, -9.8, 0);
		p->setLifetime(rand_f(0,15));
		p->setVelocity(rand_f(-2,2), rand_f(1,7), rand_f(-2,2));
		p->setBouncing(0.8f);
		colors.push_back(glm::vec4(rand_f(0,1), rand_f(0,1),rand_f(0,1), 1.0f));
	}

	currentTime = 0.0f;
	particle_mesh->sendToOpenGL(basicProgram);
	cave->sendToOpenGL(basicProgram);
	camera.init(2.0f);
	// Set player position
	// camera.setPlayer(-5 ,0);
	
	// Select which font you want to use
	if(!text.init("fonts/OpenSans-Regular.ttf"))
	//if(!text.init("fonts/OpenSans-Bold.ttf"))
	//if(!text.init("fonts/DroidSerif.ttf"))
		cout << "Could not load font!!!" << endl;
	return true;
}

bool Scene::loadMesh(const char *filename)
{
	return false;
}

void Scene::update(int deltaTime, bool forward, bool back, bool left, bool right, bool bPlay) 
{
	if (!bPlay) return;
	currentTime += deltaTime;
	framerate = 1.0f/deltaTime * 1000;
	
	for (unsigned int i = 0; i < particles.size(); i++) {
		Particle* p = particles[i];
		p->updateParticle(deltaTime/1000.0f, Particle::UpdateMethod::EulerOrig);
		p->setLifetime(p->getLifetime() - deltaTime/1000.0f);

		for (unsigned int j = 0; j < planes.size(); j++) {
			Plane* plane = planes[j];
			prev_plane_dist[i][j] = curr_plane_dist[i][j];
			curr_plane_dist[i][j] = plane->distPoint2Plane(p->getCurrentPosition());
			if (prev_plane_dist[i][j]*curr_plane_dist[i][j] < 0.0f) { 
				//VERY IMPORTANT: only valid for the plane y=0 (floor plane)
				//Must be addapted to a general plane,
				glm::vec3 new_pos = p->getCurrentPosition() - (1+p->getBouncing()) * (plane->normal *  p->getCurrentPosition() + plane->dconst) * plane->normal;
				glm::vec3 new_vel = p->getVelocity() - (1+p->getBouncing()) * (plane->normal *  p->getVelocity()) * plane->normal;
				p->setPosition(new_pos);
				p->setVelocity(new_vel);
				curr_plane_dist[i][j] = -curr_plane_dist[i][j]; //
			}
		}
		if (p->getLifetime() < 0) {
			p->setPosition(0,1,0);
			p->setForce(0, -9.8, 0);
			p->setLifetime(rand_f(0,15));
			p->setVelocity(rand_f(-2,2), rand_f(1,7), rand_f(-2,2));
		}
	}
	glm::mat4 m;
	m = camera.getModelViewMatrix();
	glm::vec3 mov(0);
	// Get forward vector of camera
	glm::vec3 forw;
	if (!vis_mode) forw = glm::normalize(glm::vec3(m[0][2], 0, m[2][2]));
	else forw = glm::normalize(glm::vec3(m[0][2], m[1][2], m[2][2]));
	glm::vec3 side = glm::normalize(glm::cross(forw, glm::vec3(0,1,0)));
	float rate = float(1.0f/deltaTime);
	if (forward){
		mov += rate * forw;
	} 
	if (back){
		mov -= rate * forw;
	}
	if (left){
		mov -= rate * side;
	} 
	if (right){
		mov += rate * side;
	} 
	camera.move(mov);
}

void Scene::render()
{
	glm::mat3 normalMatrix;

	basicProgram.use();
	glm::mat4 MV = camera.getModelViewMatrix();
	basicProgram.setUniformMatrix4f("projection", camera.getProjectionMatrix());
	normalMatrix = glm::inverseTranspose(camera.getModelViewMatrix());
	basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
	glm::vec3 player_pos = camera.getPosition();
	basicProgram.setUniform1i("bLighting", 0);

	// basicProgram.setUniform4f("color", 0.80f, 0.3f, 0.0f, 1.0f);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	// for particle in particles 
	for (unsigned int i = 0; i < particles.size(); i++) {
		Particle* p = particles[i];
		// Translate instance to position
		basicProgram.setUniform4f("color", colors[i].x, colors[i].y, colors[i].z, colors[i].w);
		glm::mat4 matrix = glm::translate(glm::mat4(1.0f), p->getCurrentPosition());
		glm::mat4 mat = MV * matrix;
		basicProgram.setUniformMatrix4f("modelview", mat);
		particle_mesh->render();
	}
	basicProgram.setUniform1i("bLighting", 1);
	basicProgram.setUniform4f("color", 0.0f, 0.5f, 1.0f, 1.0f);
	glm::mat4 mat = MV;
	basicProgram.setUniformMatrix4f("modelview", mat);
	cave->render();


	text.render(std::to_string(framerate), glm::vec2(20, 20), 16, glm::vec4(0, 0, 0, 1));
	
	//cout << "Time render map: " << (end-begin) / CLOCKS_PER_SEC << endl;
}

Camera &Scene::getCamera()
{
  return camera;
}

void Scene::switchPolygonMode()
{
  bPolygonFill = !bPolygonFill;
}

void Scene::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "shaders/basic.vert");
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/basic.frag");
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	basicProgram.init();
	basicProgram.addShader(vShader);
	basicProgram.addShader(fShader);
	basicProgram.link();
	if(!basicProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << basicProgram.log() << endl << endl;
	}
	basicProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}

void Scene::cleanup() {

}

void Scene::setDelta(double del) {
	delta = del;
}

void Scene::toggleVisMode() {
	vis_mode = !vis_mode;
}