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
	glm::vec3 normal(0,1,0);
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

	//Triangle
	glm::vec3 v1(0, -3, 3.5);
	glm::vec3 v2(-3.5, -1, 0);
	glm::vec3 v3(0,-2,-3.5);
	triangles.push_back(new Triangle(v2,v1,v3));

	TriangleMesh* tri_mesh = new TriangleMesh();
	tri_mesh->addVertex(v1);
	tri_mesh->addVertex(v2);
	tri_mesh->addVertex(v3);
	tri_mesh->addTriangle(1,0,2);

	tri_meshes.push_back(tri_mesh);

	v1 = glm::vec3(0, -2, -3.5);
  v2 = glm::vec3(3.5, -1, 0);
  v3 = glm::vec3(0, -3, 3.5);
	triangles.push_back(new Triangle(v2,v1,v3));

	tri_mesh = new TriangleMesh();
	tri_mesh->addVertex(v1);
	tri_mesh->addVertex(v2);
	tri_mesh->addVertex(v3);
	tri_mesh->addTriangle(1,0,2);

	tri_meshes.push_back(tri_mesh);

	glm::vec3 center(-1,-2, -1);
	Sphere* s = new Sphere(glm::vec3(0,-1, 0), 1.0f);
	spheres.push_back(s);

	

	for (unsigned int i = 0; i < 1000; i++) {
		Particle* part = new Particle(0,0,0);
		particles.push_back(part);
		
		std::vector<float> dists;
		for (Plane* p : planes) {
			dists.push_back(p->distPoint2Plane(part->getCurrentPosition()));
		}
		
		std::vector<float> dists_tri;
		for (Triangle* t : triangles) {
			dists_tri.push_back(t->distPoint2Plane(part->getCurrentPosition()));
		}

		curr_plane_dist.push_back(dists);
		prev_plane_dist.push_back(dists);

		curr_tri_dist.push_back(dists_tri);
		prev_tri_dist.push_back(dists_tri);
	}

	particle_mesh = new TriangleMesh();
	cave = new TriangleMesh();

	sphere_mesh = new TriangleMesh();
	PLYReader reader;
	reader.readMesh("../models/sphere.ply", *sphere_mesh);

	particle_mesh->buildCube(0.03, false, glm::vec3(0,1,0));
	cave->buildCube(5, true, glm::vec3(0.0f));
	delta = 0;
	framerate = 0;
}

Scene::~Scene()
{

}

float rand_f(float a, float b) {
    return ((b - a) * ((float)rand() / RAND_MAX)) + a;
}

void Scene::init_particle(Particle* p, float dt) {

	p->setForce(0, -9.8, 0);
	p->setLifetime(rand_f(1,15));
	switch (part_mode) {
	case 0:			
		p->setPosition(0,1,0);
		p->setVelocity(rand_f(-2,2), 5, rand_f(-2,2));
		break;
	case 1:
		p->setPosition(rand_f(-0.5, 0.5),3,0);
		p->setVelocity(rand_f(-2,2), 0, rand_f(-2,2));
		break;
	}
	if (update_mode == 2) {
		p->setPreviousPosition(p->getCurrentPosition() - dt * p->getVelocity()); 
	}

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

bool Scene::init(int argc, char* argv[])
{
	initShaders();

	part_mode = 0;
	update_mode = 0;
	vector<string> args;
	for (int i = 0; i < argc; i++) {
		args.push_back(string(argv[i]));
	}
	
	int i = 0;
	for (Particle* p : particles) {
		init_particle(p,0);
		colors.push_back(glm::vec4(rand_f(0,1), rand_f(0,1),rand_f(0,1), 1.0f));
		i++;
	}

	currentTime = 0.0f;
	particle_mesh->sendToOpenGL(basicProgram);
	cave->sendToOpenGL(basicProgram);
	for (TriangleMesh* tri_mesh : tri_meshes)
		tri_mesh->sendToOpenGL(basicProgram);
	sphere_mesh->sendToOpenGL(basicProgram);
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

void update_particle_plane(Particle* p, Plane* plane) {
	glm::vec3 new_pos = p->getCurrentPosition() - ((1.0f + p->getBouncing()) * (glm::dot(plane->normal, p->getCurrentPosition()) + plane->dconst) * plane->normal);
	glm::vec3 normal_vel = glm::dot(plane->normal, p->getVelocity()) * plane->normal;
	glm::vec3 tangent_vel = p->getVelocity() - normal_vel;
	glm::vec3 new_vel = p->getVelocity() - (( (1.0f+p->getBouncing()) * glm::dot(plane->normal, p->getVelocity())) * plane->normal);
	new_vel = new_vel - 0.1f * tangent_vel;
	p->setPosition(new_pos);
	p->setVelocity(new_vel);
}

void update_particle_sphere(Particle* p, glm::vec3 normal, glm::vec3 point) {
	Plane* plane = new Plane(point, normal);
	glm::vec3 new_pos = p->getCurrentPosition() - ((1.0f + p->getBouncing()) * (glm::dot(plane->normal, p->getCurrentPosition()) + plane->dconst) * plane->normal);
	glm::vec3 normal_vel = glm::dot(plane->normal, p->getVelocity()) * plane->normal;
	glm::vec3 tangent_vel = p->getVelocity() - normal_vel;
	glm::vec3 new_vel = p->getVelocity() - (( (1.0f+p->getBouncing()) * glm::dot(plane->normal, p->getVelocity())) * plane->normal);
	new_vel = new_vel - 0.1f * tangent_vel;
	p->setPosition(new_pos);
	p->setVelocity(new_vel);
	free(plane);
}

void Scene::update(int deltaTime, bool forward, bool back, bool left, bool right, bool bPlay) 
{
	if (!bPlay) return;
	currentTime += deltaTime;
	framerate = 1.0f/deltaTime * 1000;
	
	for (unsigned int i = 0; i < particles.size(); i++) {
		Particle* p = particles[i];
		switch (update_mode) {
			case 0: // Euler Original
				p->updateParticle(deltaTime/1000.0f, Particle::UpdateMethod::EulerOrig);
				break;
			case 1: // Euler Semi-Implicit
				p->updateParticle(deltaTime/1000.0f, Particle::UpdateMethod::EulerSemi);
				break;
			case 2: // Verlet
				p->updateParticle(deltaTime/1000.0f, Particle::UpdateMethod::Verlet);
				break;
		}
		p->setLifetime(p->getLifetime() - deltaTime/1000.0f);

		for (unsigned int j = 0; j < planes.size(); j++) {
			Plane* plane = planes[j];
			prev_plane_dist[i][j] = curr_plane_dist[i][j];
			curr_plane_dist[i][j] = plane->distPoint2Plane(p->getCurrentPosition());
			if (prev_plane_dist[i][j]*curr_plane_dist[i][j] < 0.0f) { 
				update_particle_plane(p, plane);
				curr_plane_dist[i][j] = -curr_plane_dist[i][j];
			}
		}

		for (unsigned int j = 0; j < triangles.size(); j++) {
			Triangle* tri = triangles[j];
			prev_tri_dist[i][j] = curr_tri_dist[i][j];
			curr_tri_dist[i][j] = tri->distPoint2Plane(p->getCurrentPosition());
			if (prev_tri_dist[i][j]*curr_tri_dist[i][j] < 0.0f) {
				if (!tri->isInside(p->getCurrentPosition())) continue; 
				update_particle_plane(p, tri);
				curr_tri_dist[i][j] = -curr_tri_dist[i][j]; 
			}
		}

		for (unsigned int j = 0; j < spheres.size(); j++) {
			Sphere* s = spheres[j];
			if (s->isInside(p->getCurrentPosition())) {
				glm::vec3 boundary = s->getPointOnBoundary(p->getCurrentPosition(), p->getPreviousPosition());
				glm::vec3 normal = boundary - s->center;
				update_particle_sphere(p, normal, boundary);
			}
		}

		if (p->getLifetime() < 0) {
			init_particle(p, deltaTime/1000.0f);
		}
	}
	glm::mat4 m;
	m = camera.getModelViewMatrix();
	glm::vec3 mov(0);
	// Get forward vector of camera
	glm::vec3 forw;
	//if (!vis_mode) 
	forw = glm::normalize(glm::vec3(m[0][2], 0, m[2][2]));
	//forw = glm::normalize(glm::vec3(m[0][2], m[1][2], m[2][2]));
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
	glm::mat4 mat;
	glm::mat4 matrix;
	for (unsigned int i = 0; i < particles.size(); i++) {
		Particle* p = particles[i];
		// Translate instance to position
		basicProgram.setUniform4f("color", colors[i].x, colors[i].y, colors[i].z, colors[i].w);
		matrix = glm::translate(glm::mat4(1.0f), p->getCurrentPosition());
		mat = MV * matrix;
		basicProgram.setUniformMatrix4f("modelview", mat);
		particle_mesh->render();
	}
	basicProgram.setUniform1i("bLighting", 1);
	basicProgram.setUniform4f("color", 0.0f, 0.5f, 1.0f, 1.0f);
	mat = MV;
	basicProgram.setUniformMatrix4f("modelview", mat);
	cave->render();

	basicProgram.setUniform4f("color", 1.0f, 0.5f, 0.0f, 1.0f);
	
	glDisable(GL_CULL_FACE);

	for (TriangleMesh* tri_mesh : tri_meshes) 
		tri_mesh->render();

	glEnable(GL_CULL_FACE);
	//Sphere
	basicProgram.setUniform4f("color", 0.4f, 0.0f, 0.4f, 1.0f);

	for (Sphere* s : spheres) {
		matrix = glm::translate(glm::mat4(1.0f), s->center);
		matrix = glm::scale(matrix, glm::vec3(2,2,2));
		mat = MV * matrix;
		basicProgram.setUniformMatrix4f("modelview", mat);
		sphere_mesh->render();
	}

	text.render("Press M to change particle generation mode!", glm::vec2(20, 20), 16, glm::vec4(1, 1, 1, 1));

	text.render("Press U to change particle update mode!", glm::vec2(20, 40), 16, glm::vec4(1, 1, 1, 1));

	if (part_mode) {
		text.render("Particle Mode: Waterfall", glm::vec2(20, 60), 16, glm::vec4(1, 1, 1, 1));
	} else {
		text.render("Particle Mode: Fountain", glm::vec2(20, 60), 16, glm::vec4(1, 1, 1, 1));
	}

	switch (update_mode) {
		case 0:
			text.render("Update Mode: Euler Original", glm::vec2(20, 80), 16, glm::vec4(1, 1, 1, 1));
			break;
		case 1:
			text.render("Update Mode: Euler Semi-Implicit", glm::vec2(20, 80), 16, glm::vec4(1, 1, 1, 1));
			break;
		case 2:
			text.render("Update Mode: Verlet", glm::vec2(20, 80), 16, glm::vec4(1, 1, 1, 1));
			break;
	}
	
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

void Scene::toggleParticleMode() {
	if (part_mode == 1) part_mode = 0;
	else part_mode++;
}

void Scene::toggleUpdateMode() {
	if (update_mode == 2) update_mode = 0;
	else update_mode++;
}