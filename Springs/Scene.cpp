#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <ctime>
#include <iostream>

#include "Scene.h"
#include "PLYReader.h"

#define GLM_FORCE_RADIANS

#define speed 1
#define g -0.05//-3.8
Scene::Scene()
{


	Plane * plane;
	plane = new Plane(glm::vec3(0,-5,0), glm::vec3(0,1,0));
	planes.push_back(plane);

	Sphere* sp;
	sp = new Sphere(glm::vec3(0,-2,0),1);
	spheres.push_back(sp);

	cave = new TriangleMesh();

	particle_mesh = new TriangleMesh();
	is_rope = false;
	// particle_mesh->buildRope();
	PLYReader reader;
	reader.readMesh("../models/flat_surface.ply", *particle_mesh);
	
	sphere_mesh = new TriangleMesh();
	reader.readMesh("../models/sphere.ply", *sphere_mesh);

	Particle *part;
	for (unsigned int i = 0; i < particle_mesh->getVertices().size(); i++)
	{
		glm::vec3 v = particle_mesh->getVertices()[i];
		initial_pos.push_back(glm::vec3(v));
		part = new Particle(v.x, v.y, v.z);
		part->setMass(1);

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

	cave->buildCube(5, true, glm::vec3(0.0f));
	delta = 0;
	framerate = 0;
}

Scene::~Scene()
{
}

float rand_f(float a, float b)
{
	return ((b - a) * ((float)rand() / RAND_MAX)) + a;
}

void Scene::init_particle(Particle *p, float dt)
{

	p->setForce(0, g, 0);
	// p->setVelocity(0,1,0);
	// p->setLifetime(rand_f(1, 15));
	switch (part_mode)
	{
	case 0:
		// p->setVelocity(rand_f(-2, 2), 5, rand_f(-2, 2));
		break;
	case 1:
		// p->setVelocity(rand_f(-2, 2), 0, rand_f(-2, 2));
		break;
	}
	// if (update_mode == 2)
	// {
	// 	p->setPreviousPosition(p->getCurrentPosition() - dt * p->getVelocity());
	// }
}

std::vector<std::string> split(const std::string &str, const std::string &delim)
{
	std::vector<std::string> tokens;
	unsigned int prev = 0, pos = 0;
	do
	{
		pos = str.find(delim, prev);
		if (pos == std::string::npos)
			pos = str.length();
		std::string token = str.substr(prev, pos - prev);
		if (!token.empty())
			tokens.push_back(token);
		prev = pos + delim.length();
	} while (pos < str.length() && prev < str.length());
	return tokens;
}

bool Scene::init(int argc, char *argv[])
{
	initShaders();

	part_mode = 0;
	update_mode = 2;
	use_stretch = true;
	use_shear = true;
	use_bend = true;
	vector<string> args;
	for (int i = 0; i < argc; i++)
	{
		args.push_back(string(argv[i]));
	}

	matrix_size = sqrt(particles.size());
	int i = 0;
	for (Particle* p : particles) {
		init_particle(p,0);
		i++;
	}

	// for (unsigned int i = 0; i < particles.size(); i++) {
	// 	std::vector<float> js;
	// 	for (unsigned int j = 0; j < particles.size(); j++) {
	// 		js.push_back(glm::distance(particles[i]->getCurrentPosition(), particles[j]->getCurrentPosition()));
	// 	}
	// 	initial_ds.push_back(js);
	// }
	
	initial_d = glm::distance(particles[0]->getCurrentPosition(), particles[1]->getCurrentPosition());
	initial_ds = sqrt(2*initial_d*initial_d);

	currentTime = 0.0f;
	// particle_mesh->sendToOpenGL(basicProgram);
	cave->sendToOpenGL(basicProgram);
	// for (TriangleMesh* tri_mesh : tri_meshes)
	// 	tri_mesh->sendToOpenGL(basicProgram);
	particle_mesh->sendToOpenGL(basicProgram);
	sphere_mesh->sendToOpenGL(basicProgram);
	camera.init(2.0f);
	// Set player position
	// camera.setPlayer(-5 ,0);

	// Select which font you want to use
	if (!text.init("fonts/OpenSans-Regular.ttf"))
		//if(!text.init("fonts/OpenSans-Bold.ttf"))
		//if(!text.init("fonts/DroidSerif.ttf"))
		cout << "Could not load font!!!" << endl;
	return true;
}

bool Scene::loadMesh(const char *filename)
{
	return false;
}

void update_particle_plane(Particle *p, Plane *plane, int update_mode)
{
	glm::vec3 new_pos = p->getCurrentPosition() - ((1.0f + p->getBouncing()) * (glm::dot(plane->normal, p->getCurrentPosition()) + plane->dconst) * plane->normal);
	glm::vec3 normal_vel = glm::dot(plane->normal, p->getVelocity()) * plane->normal;
	glm::vec3 tangent_vel = p->getVelocity() - normal_vel;
	glm::vec3 new_vel = p->getVelocity() - (((1.0f + p->getBouncing()) * glm::dot(plane->normal, p->getVelocity())) * plane->normal);
	new_vel = new_vel - 0.1f * tangent_vel;
	if (update_mode == 2)
	{
		glm::vec3 corrected = p->getPreviousPosition() - 2.0f * (glm::dot(plane->normal, p->getPreviousPosition()) + plane->dconst) * plane->normal;
		p->setPreviousPosition(corrected);
	}
	p->setPosition(new_pos);
	p->setVelocity(new_vel);
}

void update_particle_sphere(Particle *p, glm::vec3 normal, glm::vec3 point, int update_mode)
{
	Plane *plane = new Plane(point, normal);
	glm::vec3 new_pos = p->getCurrentPosition() - ((1.0f + p->getBouncing()) * (glm::dot(plane->normal, p->getCurrentPosition()) + plane->dconst) * plane->normal);
	glm::vec3 normal_vel = glm::dot(plane->normal, p->getVelocity()) * plane->normal;
	glm::vec3 tangent_vel = p->getVelocity() - normal_vel;
	glm::vec3 new_vel = p->getVelocity() - (((1.0f + p->getBouncing()) * glm::dot(plane->normal, p->getVelocity())) * plane->normal);
	new_vel = new_vel - 0.1f * tangent_vel;
	if (update_mode == 2)
	{
		glm::vec3 corrected = p->getPreviousPosition() - 2.0f * (glm::dot(plane->normal, p->getPreviousPosition()) + plane->dconst) * plane->normal;
		p->setPreviousPosition(corrected);
	}
	p->setPosition(new_pos);
	p->setVelocity(new_vel);
	free(plane);
}

glm::vec3 elastic_force(float ks, glm::vec3 pos1, glm::vec3 pos2, double initial_d) {
	glm::vec3 result;

	glm::vec3 vec_ = pos2-pos1;

	result =  ks * glm::normalize(vec_) * (float) (glm::length(vec_) - initial_d);
	return result;
}

glm::vec3 damping_force(float kd, glm::vec3 pos1, glm::vec3 pos2, glm::vec3 vel1, glm::vec3 vel2) {
	glm::vec3 result;

	glm::vec3 vec_ = pos2-pos1;
	glm::vec3 vel = vel2-vel1;

	result = kd * (glm::dot(vel, glm::normalize(vec_))) * glm::normalize(vec_);
	return result;
}

void Scene::update_forces(int i) {
	glm::vec3 final_force(0);
	final_force = glm::vec3(0,g, 0);
	int mat_i = i / matrix_size;
	int mat_j = i % matrix_size;
	
	float ks;
	float kd;
	
	ks = 3;
	kd = 3;


	if (use_stretch) {
		glm::vec3 stretch_force(0);

		// ij, ij-1, ij+1, i-1j, i+1,j
		Particle* p1, *p2;
		if (mat_i - 1 >= 0) {
			p1 = particles[mat_i * matrix_size + mat_j];
			p2 = particles[(mat_i-1)*matrix_size + mat_j];
			stretch_force += elastic_force(ks, p1->getCurrentPosition(), p2->getCurrentPosition(), initial_d);
			stretch_force += damping_force(kd, p1->getCurrentPosition(), p2->getCurrentPosition(), p1->getVelocity(), p2->getVelocity());
		}

		if (mat_i + 1 < matrix_size) {
			p1 = particles[mat_i * matrix_size + mat_j];
			p2 = particles[(mat_i+1)*matrix_size + mat_j];
			stretch_force += elastic_force(ks, p1->getCurrentPosition(), p2->getCurrentPosition(), initial_d);
			stretch_force += damping_force(kd, p1->getCurrentPosition(), p2->getCurrentPosition(), p1->getVelocity(), p2->getVelocity());
		}

		if (mat_j - 1 >= 0) {
			p1 = particles[mat_i * matrix_size + mat_j];
			p2 = particles[mat_i * matrix_size + mat_j - 1];
			stretch_force += elastic_force(ks, p1->getCurrentPosition(), p2->getCurrentPosition(), initial_d);
			stretch_force += damping_force(kd, p1->getCurrentPosition(), p2->getCurrentPosition(), p1->getVelocity(), p2->getVelocity());
		}

		if (mat_j + 1 < matrix_size) {
			p1 = particles[mat_i * matrix_size + mat_j];
			p2 = particles[mat_i * matrix_size + mat_j + 1];
			stretch_force += elastic_force(ks, p1->getCurrentPosition(), p2->getCurrentPosition(), initial_d);
			stretch_force += damping_force(kd, p1->getCurrentPosition(), p2->getCurrentPosition(), p1->getVelocity(), p2->getVelocity());
		}

		final_force += stretch_force;
	}


	if (use_shear) {
		glm::vec3 shear_force(0);

		// ks = 1;
		// kd = 0.01;

		Particle* p1, *p2;
		if (mat_i - 1 >= 0 && mat_j - 1 >= 0) {
			p1 = particles[mat_i * matrix_size + mat_j];
			p2 = particles[(mat_i-1)*matrix_size + mat_j - 1];
			shear_force += elastic_force(ks, p1->getCurrentPosition(), p2->getCurrentPosition(), initial_ds);
			shear_force += damping_force(kd, p1->getCurrentPosition(), p2->getCurrentPosition(), p1->getVelocity(), p2->getVelocity());
		}

		if (mat_i + 1 < matrix_size && mat_j - 1 >= 0) {
			p1 = particles[mat_i * matrix_size + mat_j];
			p2 = particles[(mat_i+1)*matrix_size + mat_j - 1];
			shear_force += elastic_force(ks, p1->getCurrentPosition(), p2->getCurrentPosition(), initial_ds);
			shear_force += damping_force(kd, p1->getCurrentPosition(), p2->getCurrentPosition(), p1->getVelocity(), p2->getVelocity());
		}
		
		if (mat_i - 1 >= 0 && mat_j + 1 < matrix_size) {
			p1 = particles[mat_i * matrix_size + mat_j];
			p2 = particles[(mat_i-1) * matrix_size + mat_j + 1];
			shear_force += elastic_force(ks, p1->getCurrentPosition(), p2->getCurrentPosition(), initial_ds);
			shear_force += damping_force(kd, p1->getCurrentPosition(), p2->getCurrentPosition(), p1->getVelocity(), p2->getVelocity());
		}

		if (mat_i + 1 < matrix_size && mat_j + 1 < matrix_size) {
			p1 = particles[mat_i * matrix_size + mat_j];
			p2 = particles[(mat_i+1) * matrix_size + mat_j + 1];
			shear_force += elastic_force(ks, p1->getCurrentPosition(), p2->getCurrentPosition(), initial_ds);
			shear_force += damping_force(kd, p1->getCurrentPosition(), p2->getCurrentPosition(), p1->getVelocity(), p2->getVelocity());
		}

		final_force += shear_force;
		// ij, i-1j-1, i+1j-1, i-1j+1, i+1j+1
	}

	if (use_bend) {
		glm::vec3 bend_force(0);

		// ks = 0.1;
		// kd = 1.e-002;

		// ij, ij-1, ij+1, i-1j, i+1,j
		Particle* p1, *p2;
		if (mat_i - 2 >= 0) {
			p1 = particles[mat_i * matrix_size + mat_j];
			p2 = particles[(mat_i-2)*matrix_size + mat_j];
			bend_force += elastic_force(ks, p1->getCurrentPosition(), p2->getCurrentPosition(), 2*initial_d);
			bend_force += damping_force(kd, p1->getCurrentPosition(), p2->getCurrentPosition(), p1->getVelocity(), p2->getVelocity());
		}

		if (mat_i + 2 < matrix_size) {
			p1 = particles[mat_i * matrix_size + mat_j];
			p2 = particles[(mat_i+2)*matrix_size + mat_j];
			bend_force += elastic_force(ks, p1->getCurrentPosition(), p2->getCurrentPosition(), 2*initial_d);
			bend_force += damping_force(kd, p1->getCurrentPosition(), p2->getCurrentPosition(), p1->getVelocity(), p2->getVelocity());
		}

		if (mat_j - 2 >= 0) {
			p1 = particles[mat_i * matrix_size + mat_j];
			p2 = particles[mat_i * matrix_size + mat_j - 2];
			bend_force += elastic_force(ks, p1->getCurrentPosition(), p2->getCurrentPosition(), 2*initial_d);
			bend_force += damping_force(kd, p1->getCurrentPosition(), p2->getCurrentPosition(), p1->getVelocity(), p2->getVelocity());
		}

		if (mat_j + 2 < matrix_size) {
			p1 = particles[mat_i * matrix_size + mat_j];
			p2 = particles[mat_i * matrix_size + mat_j + 2];
			bend_force += elastic_force(ks, p1->getCurrentPosition(), p2->getCurrentPosition(), initial_d);
			bend_force += damping_force(kd, p1->getCurrentPosition(), p2->getCurrentPosition(), p1->getVelocity(), p2->getVelocity());
		}

		final_force += bend_force;
	}

	particles[i]->setForce(final_force);
} 

void Scene::update_forces_rope(int i) {
	Particle* p1, *p2;
	glm::vec3 final_force(0);
	final_force = glm::vec3(0,g,0);

	float ks, kd;
	
	ks = 3;
	kd = 3;

	if (use_stretch) {
		glm::vec3 stretch_force(0);
		if (i + 1 < particles.size()) {
			p1 = particles[i];
			p2 = particles[i+1];
			stretch_force += elastic_force(ks, p1->getCurrentPosition(), p2->getCurrentPosition(), initial_d);
			stretch_force += damping_force(kd, p1->getCurrentPosition(), p2->getCurrentPosition(), p1->getVelocity(), p2->getVelocity());
		}

		if (i - 1 >= 0) {
			p1 = particles[i];
			p2 = particles[i-1];
			stretch_force += elastic_force(ks, p1->getCurrentPosition(), p2->getCurrentPosition(), initial_d);
			stretch_force += damping_force(kd, p1->getCurrentPosition(), p2->getCurrentPosition(), p1->getVelocity(), p2->getVelocity());
		}
		final_force += stretch_force;
	}

	particles[i]->setForce(final_force);
}

void Scene::update(int deltaTime, bool forward, bool back, bool left, bool right, bool bPlay)
{
	if (!bPlay)
		return;
	deltaTime = speed*deltaTime;
	currentTime += deltaTime;
	framerate = 1.0f / deltaTime * 1000;

	for (unsigned int i = 0; i < particles.size(); i++) {
		Particle* p = particles[i];
		if (is_rope){
			if (i != 0) 
				update_forces_rope(i);
		} else if (i%matrix_size != matrix_size-1) {
			update_forces(i);
		}
	}

	for (unsigned int i = 0; i < particles.size(); i++) {
		Particle* p = particles[i];
		if (is_rope) {
			if (i == 0) continue;
		}
		else if (i%matrix_size == matrix_size-1) {
			continue;
		}
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
		for (unsigned int j = 0; j < planes.size(); j++) {
			Plane* plane = planes[j];
			prev_plane_dist[i][j] = curr_plane_dist[i][j];
			curr_plane_dist[i][j] = plane->distPoint2Plane(p->getCurrentPosition());
			if (prev_plane_dist[i][j]*curr_plane_dist[i][j] < 0.0f) {
				update_particle_plane(p, plane, update_mode);
				curr_plane_dist[i][j] = -curr_plane_dist[i][j];
			}
		}
		
		for (unsigned int j = 0; j < spheres.size(); j++) {
			Sphere* s = spheres[j];
			if (s->isInside(p->getCurrentPosition())) {
				glm::vec3 boundary = s->getPointOnBoundary(p->getCurrentPosition(), p->getPreviousPosition());
				glm::vec3 normal = boundary - s->center;
				update_particle_sphere(p, normal, boundary, update_mode);
			}
		}
		// p->setLifetime(p->getLifetime() - deltaTime/1000.0f);
		particle_mesh->vertices[i] = p->getCurrentPosition();

	}
		particle_mesh->changeVBO(basicProgram);
		// particle_mesh->sendToOpenGL(basicProgram);

	// 	for (unsigned int j = 0; j < triangles.size(); j++) {
	// 		Triangle* tri = triangles[j];
	// 		prev_tri_dist[i][j] = curr_tri_dist[i][j];
	// 		curr_tri_dist[i][j] = tri->distPoint2Plane(p->getCurrentPosition());
	// 		if (prev_tri_dist[i][j]*curr_tri_dist[i][j] < 0.0f) {
	// 			if (!tri->isInside(p->getCurrentPosition())) continue;
	// 			update_particle_plane(p, tri, update_mode);
	// 			curr_tri_dist[i][j] = -curr_tri_dist[i][j];
	// 		}
	// 	}


	// 	if (p->getLifetime() < 0) {
	// 		init_particle(p, deltaTime/1000.0f);
	// 	}
	// }
	glm::mat4 m;
	m = camera.getModelViewMatrix();
	glm::vec3 mov(0);
	// Get forward vector of camera
	glm::vec3 forw;
	//if (!vis_mode)
	forw = glm::normalize(glm::vec3(m[0][2], 0, m[2][2]));
	//forw = glm::normalize(glm::vec3(m[0][2], m[1][2], m[2][2]));
	glm::vec3 side = glm::normalize(glm::cross(forw, glm::vec3(0, 1, 0)));
	float rate = float(1.0f / deltaTime);
	if (forward)
	{
		mov += rate * forw;
	}
	if (back)
	{
		mov -= rate * forw;
	}
	if (left)
	{
		mov -= rate * side;
	}
	if (right)
	{
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
	// for (unsigned int i = 0; i < particles.size(); i++) {
	// 	Particle* p = particles[i];
	// 	// Translate instance to position
	// 	basicProgram.setUniform4f("color", colors[i].x, colors[i].y, colors[i].z, colors[i].w);
	// 	matrix = glm::translate(glm::mat4(1.0f), p->getCurrentPosition());
	// 	mat = MV * matrix;
	// 	basicProgram.setUniformMatrix4f("modelview", mat);
	// 	particle_mesh->render();
	// }
	basicProgram.setUniform1i("bLighting", 1);
	basicProgram.setUniform4f("color", 0.0f, 0.5f, 1.0f, 1.0f);
	mat = MV;
	basicProgram.setUniformMatrix4f("modelview", mat);
	cave->render();

	basicProgram.setUniform4f("color", 1.0f, 0.5f, 0.0f, 1.0f);

	glDisable(GL_CULL_FACE);

	// for (TriangleMesh* tri_mesh : tri_meshes)
	// 	tri_mesh->render();

	// glEnable(GL_CULL_FACE);
	//Sphere
	particle_mesh->render();
	basicProgram.setUniform4f("color", 0.4f, 0.0f, 0.4f, 1.0f);

	for (Sphere* s : spheres) {
		matrix = glm::translate(glm::mat4(1.0f), s->center);
		matrix = glm::scale(matrix, glm::vec3(1.98,1.98,1.98));
		mat = MV * matrix;
		basicProgram.setUniformMatrix4f("modelview", mat);
		sphere_mesh->render();
	}

	text.render("Press M to change particle generation mode!", glm::vec2(20, 20), 16, glm::vec4(1, 1, 1, 1));

	text.render("Press U to change particle update mode!", glm::vec2(20, 40), 16, glm::vec4(1, 1, 1, 1));

	text.render("Press Y to toggle Stretch!", glm::vec2(20, 60), 16, glm::vec4(1, 1, 1, 1));
	text.render("Press I to toggle Shear!", glm::vec2(20, 80), 16, glm::vec4(1, 1, 1, 1));
	text.render("Press B to toggle Bending!", glm::vec2(20, 100), 16, glm::vec4(1, 1, 1, 1));

	switch (update_mode)
	{
	case 0:
		text.render("Update Mode: Euler Original", glm::vec2(20, 120), 16, glm::vec4(1, 1, 1, 1));
		break;
	case 1:
		text.render("Update Mode: Euler Semi-Implicit", glm::vec2(20, 120), 16, glm::vec4(1, 1, 1, 1));
		break;
	case 2:
		text.render("Update Mode: Verlet", glm::vec2(20, 120), 16, glm::vec4(1, 1, 1, 1));
		break;
	}

	if (use_stretch) 
		text.render("Stretch", glm::vec2(20, 140), 16, glm::vec4(1, 1, 1, 1));
	if (use_shear)
		text.render("Shear", glm::vec2(80, 140), 16, glm::vec4(1, 1, 1, 1));
	if (use_bend)
		text.render("Bend", glm::vec2(130, 140), 16, glm::vec4(1, 1, 1, 1));

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
	if (!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl
				 << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/basic.frag");
	if (!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl
				 << endl;
	}
	basicProgram.init();
	basicProgram.addShader(vShader);
	basicProgram.addShader(fShader);
	basicProgram.link();
	if (!basicProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << basicProgram.log() << endl
				 << endl;
	}
	basicProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}

void Scene::cleanup()
{
}

void Scene::setDelta(double del)
{
	delta = del;
}

void Scene::toggleParticleMode()
{
	if (part_mode == 1)
		part_mode = 0;
	else
		part_mode++;
}

void Scene::toggleUpdateMode()
{
	if (update_mode == 2)
		update_mode = 0;
	else
		update_mode++;
}

void Scene::toggleStretch() {
	use_stretch = !use_stretch;
}

void Scene::toggleShear() {
	use_shear = !use_shear;
}

void Scene::toggleBending() {
	use_bend = !use_bend;
}

void Scene::resetScene(bool is_rope) {
	for (unsigned int i = 0; i < particles.size(); i++) {
		particles[i]->setPosition(initial_pos[i]);
		particles[i]->setVelocity(glm::vec3(0));
		particles[i]->setPreviousPosition(initial_pos[i]);
		init_particle(particles[i], 1/(framerate/1000.0f));
	}
}