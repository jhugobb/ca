#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <ctime>
#include <iostream>

#include "Scene.h"
#include "PLYReader.h"

#define part_size 0.2f
#define volume 12.f
#define part_mass 0.002f
#define smooth_kernel_radius 1.f
#define rest_density 998.2f
#define k_const 5.0f
#define viscosity_miu 3.5f
#define surface_constant 0.0728f
#define surface_limit 6.0f

#define part_bouncing 0.6f
#define target_fps 60.f

bool isPrime(int n)
{
    //loop from 2 to n/2 to check for factors
    for (int i = 2; i <= n/2; i++)
    {
        if (n % i == 0)     //found a factor that isn't 1 or n, therefore not prime
            return false;
    }

    return true;
}

int next_prime(int n) {
	int nextPrime = n;
	bool found = false;

	//loop continuously until isPrime returns true for a number above n
	while (!found)
	{
			nextPrime++;
			if (isPrime(nextPrime))
					found = true;
	}

	return nextPrime;
}

float kernel_poly_6(glm::vec3 pos) {
	float den = 64.0f * M_PI * pow(smooth_kernel_radius, 9);
	if (glm::length(pos) <= smooth_kernel_radius && glm::length(pos) >= 0) {
		float h_2 = pow(smooth_kernel_radius, 2);
		float r_2 =  pow(glm::length(pos), 2);
		return 315.0f/den * (pow(h_2-r_2, 3));
	} else {
		return 0;
	}
}

glm::vec3 kernel_poly_6_gradient(glm::vec3 pos) {
	float den = - 32.0f * M_PI * pow(smooth_kernel_radius, 9);
	float m1 = pow(pow(smooth_kernel_radius, 2) - pow(glm::length(pos),2), 2);
	return 945/den * pos * m1;
}

float kernel_poly_6_laplacian(glm::vec3 pos) {
	float den = - 32.0f * M_PI * pow(smooth_kernel_radius, 9);
	float m1 = pow(smooth_kernel_radius, 2) - pow(glm::length(pos),2);
	float m2 = 3.0f * pow(smooth_kernel_radius, 2) - 7.0f * pow(glm::length(pos),2);
	return (945 / den) * m1 * m2;
}

glm::vec3 kernel_spiky_gradient(glm::vec3 pos) {
	float den = - M_PI * pow(smooth_kernel_radius, 6);
	glm::vec3 norm_pos = glm::normalize(pos);

	return (45.0f/den) * norm_pos * (float) pow(smooth_kernel_radius-glm::length(pos), 2);
}

float kernel_viscosity_laplacian(glm::vec3 pos) {
	float den = M_PI * pow(smooth_kernel_radius, 6);
	return (45.0f/den) * (smooth_kernel_radius - glm::length(pos));
}

void Scene::getNN() {
	for (int i = 0; i < particles.size(); i++) {
		Particle* p = particles[i];
		p->neighs.clear();
	}

	for (int i = 0; i < particles.size(); i++) {
		Particle* p1 = particles[i];
		for (int j = i+1; j < particles.size(); j++) {
			Particle* p2 = particles[j];
			if (glm::distance(p1->pos, p2->pos) <= smooth_kernel_radius){
				p1->neighs.push_back(j);
				p2->neighs.push_back(i);
			}
		}
	}
}

void Scene::calc_forces_particles(float dt) {
	getNN();

	for (uint i = 0; i < particles.size(); i++) {
		// float n_nei =  particles[i]->neighs.size();
		// float n_parts = particles.size();
		// cout << particles[i]->neighs.size() << endl;
		// Calculate Density
		float density = rest_density;
		for (uint j = 0; j < particles[i]->neighs.size(); j++) {
			Particle *p = particles[particles[i]->neighs[j]];
			if (p == particles[i]) continue;
			// cout << "hey" << endl;
			density += p->mass * kernel_poly_6(particles[i]->pos - p->pos);
		}
		particles[i]->density = density;
		particles[i]->transparency = density/rest_density;

		// Calculate Pressure
		particles[i]->pressure = k_const/volume - k_const*rest_density;
	}


	for (uint i = 0; i < particles.size(); i++) {
		// Calculate Internal Forces
		glm::vec3 f_pressure = glm::vec3(0,0,0);
		glm::vec3 f_viscosity = glm::vec3(0,0,0);
		glm::vec3 f_surface = glm::vec3(0,0,0);
		glm::vec3 velocity_correction = glm::vec3(0,0,0);

		for (uint j = 0; j < particles[i]->neighs.size(); j++) {
			Particle *p = particles[particles[i]->neighs[j]];
			if (p == particles[i]) continue;
			glm::vec3 r = particles[i]->pos - p->pos;
			// Pressure
			f_pressure += (-1) * ((p->pressure+particles[i]->pressure)/2) * (p->mass/p->density) * kernel_spiky_gradient(r);
			// Viscosity
			f_viscosity += (p->vel - particles[i]->vel) * (p->mass / p->density) * kernel_viscosity_laplacian(r) * viscosity_miu; 
			// Surface Tension
			glm::vec3 n =  (p->mass / p->density) * kernel_poly_6_gradient(r);
			if (glm::length(n) >= surface_limit) {
				particles[i]->color = glm::vec3(0,1,1);
				f_surface += -surface_constant *  (p->mass / p->density) * kernel_poly_6_laplacian(r) * glm::normalize(n);
			} else particles[i]->color = glm::vec3(0,0,1);

			// Buoyancy is zero

			// Velocity Correction
			velocity_correction += 0.1f * (2.0f*p->mass)/(particles[i]->density + p->density) * kernel_poly_6(r);
		}
		particles[i]->acc = (glm::vec3(0,-9.8,0) + f_pressure + f_surface) / particles[i]->density;
		particles[i]->vel = particles[i]->vel + dt * particles[i]->acc;

		particles[i]->vel += velocity_correction;
	}
}

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

	glm::vec3 center(-1,-2, -1);
	Sphere* s = new Sphere(glm::vec3(0,-1, 0), 1.0f);
	spheres.push_back(s);

	
	float n_particles = cbrt(volume/(4*M_PI * pow(part_size,3)));
	cout << n_particles << endl;
	for (uint i = 0; i < n_particles; i++) {
		for (uint j = 0; j < n_particles; j++) {
			for (uint k = 0; k < n_particles; k++) {
				Particle* part = new Particle(glm::vec3(0, 1, 0) + part_size * glm::vec3(i,j,k), glm::vec3(0,0,0), part_size, glm::vec3(0, 1, 1), 0.5, 10, part_mass);
				part->vel = glm::vec3(0,0.1,0);
				part->color = glm::vec3(0,1,1);
				particles.push_back(part);

				std::vector<float> dists;
				for (Plane* p : planes) {
					dists.push_back(p->distPoint2Plane(part->pos));
				}
				
				std::vector<float> dists_tri;
				for (Triangle* t : triangles) {
					dists_tri.push_back(t->distPoint2Plane(part->pos));
				}

				curr_plane_dist.push_back(dists);
				prev_plane_dist.push_back(dists);

				curr_tri_dist.push_back(dists_tri);
				prev_tri_dist.push_back(dists_tri);
			}
		}
	}

	cout << "Particles size " << particles.size() << endl;
	calc_forces_particles(0.0f);

	particle_mesh = new TriangleMesh();
	cave = new TriangleMesh();

	sphere_mesh = new TriangleMesh();
	PLYReader reader;
	reader.readMesh("../models/sphere.ply", *sphere_mesh);

	particle_mesh->buildCube(0.05, false, glm::vec3(0,1,0));
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

bool Scene::init(int argc, char* argv[])
{
	initShaders();
	int codec = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
	video.open("video.avi", codec, target_fps, cv::Size(1280, 720), true);
	part_mode = 0;
	update_mode = 0;
	vector<string> args;
	for (int i = 0; i < argc; i++) {
		args.push_back(string(argv[i]));
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

void update_particle_plane(Particle* p, Plane* plane, int update_mode) {
	glm::vec3 new_pos = p->pos - ((1.0f + part_bouncing) * (glm::dot(plane->normal, p->pos) + plane->dconst) * plane->normal);
	glm::vec3 normal_vel = glm::dot(plane->normal, p->vel) * plane->normal;
	glm::vec3 tangent_vel = p->vel - normal_vel;
	glm::vec3 new_vel = p->vel - (( (1.0f+part_bouncing) * glm::dot(plane->normal, p->vel)) * plane->normal);
	new_vel = new_vel - 0.7f * tangent_vel;
	if (update_mode == 2) {
		glm::vec3 corrected = p->prev - 2.0f * (glm::dot(plane->normal, p->prev) + plane->dconst) * plane->normal;
		p->prev = corrected;
	}
	p->pos = new_pos;
	p->vel = new_vel;
}

void update_particle_sphere(Particle* p, glm::vec3 normal, glm::vec3 point, int update_mode) {
	Plane* plane = new Plane(point, normal);
	glm::vec3 new_pos = p->pos - ((1.0f + part_bouncing) * (glm::dot(plane->normal, p->pos) + plane->dconst) * plane->normal);
	glm::vec3 normal_vel = glm::dot(plane->normal, p->vel) * plane->normal;
	glm::vec3 tangent_vel = p->vel - normal_vel;
	glm::vec3 new_vel = p->vel - (((1.0f+part_bouncing) * glm::dot(plane->normal, p->vel)) * plane->normal);
	new_vel = new_vel - 0.7f * tangent_vel;
	if (update_mode == 2) {
		glm::vec3 corrected = p->prev - 2.0f * (glm::dot(plane->normal, p->prev) + plane->dconst) * plane->normal;
		p->prev = corrected;
	}
	p->pos = new_pos;
	p->vel = new_vel;
	free(plane);
}

void Scene::update(int deltaTime, bool forward, bool back, bool left, bool right, bool bPlay) 
{
	if (!bPlay) return;
	currentTime += deltaTime;
	framerate = 1.0f/deltaTime * 1000;
	
	for (unsigned int i = 0; i < particles.size(); i++) {
		Particle* p = particles[i];
		p->prev = p->pos;
		p->pos += deltaTime/10000.0f * p->vel;
		// cout << p->pos.x << " " << p->pos.x << " " << p->pos.z << endl;

		for (unsigned int j = 0; j < planes.size(); j++) {
			Plane* plane = planes[j];
			prev_plane_dist[i][j] = curr_plane_dist[i][j];
			curr_plane_dist[i][j] = plane->distPoint2Plane(p->pos);
			if (prev_plane_dist[i][j]*curr_plane_dist[i][j] < 0.0f) { 
				update_particle_plane(p, plane, update_mode);
				curr_plane_dist[i][j] = -curr_plane_dist[i][j];
			}
		}

		for (unsigned int j = 0; j < triangles.size(); j++) {
			Triangle* tri = triangles[j];
			prev_tri_dist[i][j] = curr_tri_dist[i][j];
			curr_tri_dist[i][j] = tri->distPoint2Plane(p->pos);
			if (prev_tri_dist[i][j]*curr_tri_dist[i][j] < 0.0f) {
				if (!tri->isInside(p->pos)) continue; 
				update_particle_plane(p, tri, update_mode);
				curr_tri_dist[i][j] = -curr_tri_dist[i][j]; 
			}
		}

		for (unsigned int j = 0; j < spheres.size(); j++) {
			Sphere* s = spheres[j];
			if (s->isInside(p->pos)) {
				glm::vec3 boundary = s->getPointOnBoundary(p->pos, p->prev);
				glm::vec3 normal = boundary - s->center;
				update_particle_sphere(p, normal, boundary, update_mode);
			}
		}

		calc_forces_particles(deltaTime/10000.0f);

		// p->lifetime = p->lifetime - deltaTime/10000.0f;
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
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	
	// for particle in particles 
	glm::mat4 mat;
	glm::mat4 matrix;
	for (unsigned int i = 0; i < particles.size(); i++) {
		Particle* p = particles[i];
		// Translate instance to position
		basicProgram.setUniform4f("color", p->color.x, p->color.y, p->color.z, p->transparency);
		matrix = glm::translate(glm::mat4(1.0f), p->pos);
		mat = MV * matrix;
		basicProgram.setUniformMatrix4f("modelview", mat);
		particle_mesh->render();
	}
	glDisable(GL_BLEND);
	basicProgram.setUniform1i("bLighting", 1);
	basicProgram.setUniform4f("color", 0.0f, 0.5f, 1.0f, 1.0f);
	mat = MV;
	basicProgram.setUniformMatrix4f("modelview", mat);
	cave->render();

	// glDisable(GL_CULL_FACE);

	// for (TriangleMesh* tri_mesh : tri_meshes) 
	// 	tri_mesh->render();

	// glEnable(GL_CULL_FACE);
	// //Sphere
	basicProgram.setUniform4f("color", 0.4f, 0.0f, 0.4f, 1.0f);

	for (Sphere* s : spheres) {
		matrix = glm::translate(glm::mat4(1.0f), s->center);
		matrix = glm::scale(matrix, glm::vec3(2,2,2));
		mat = MV * matrix;
		basicProgram.setUniformMatrix4f("modelview", mat);
		sphere_mesh->render();
	}

	// text.render("Press M to change particle generation mode!", glm::vec2(20, 20), 16, glm::vec4(1, 1, 1, 1));

	// text.render("Press U to change particle update mode!", glm::vec2(20, 40), 16, glm::vec4(1, 1, 1, 1));

	// if (part_mode) {
	// 	text.render("Particle Mode: Waterfall", glm::vec2(20, 60), 16, glm::vec4(1, 1, 1, 1));
	// } else {
	// 	text.render("Particle Mode: Fountain", glm::vec2(20, 60), 16, glm::vec4(1, 1, 1, 1));
	// }

	// switch (update_mode) {
	// 	case 0:
	// 		text.render("Update Mode: Euler Original", glm::vec2(20, 80), 16, glm::vec4(1, 1, 1, 1));
	// 		break;
	// 	case 1:
	// 		text.render("Update Mode: Euler Semi-Implicit", glm::vec2(20, 80), 16, glm::vec4(1, 1, 1, 1));
	// 		break;
	// 	case 2:
	// 		text.render("Update Mode: Verlet", glm::vec2(20, 80), 16, glm::vec4(1, 1, 1, 1));
	// 		break;
	// }

	cv::Mat pixels( 720, 1280, CV_8UC3 );
	glReadPixels(0, 0, 1280, 720, GL_RGB, GL_UNSIGNED_BYTE, pixels.data );
	cv::Mat cv_pixels( 720, 1280, CV_8UC3 );
	for( int y=0; y<720; y++ ) for( int x=0; x<1280; x++ ) 
	{
			cv_pixels.at<cv::Vec3b>(y,x)[2] = pixels.at<cv::Vec3b>(720-y-1,x)[0];
			cv_pixels.at<cv::Vec3b>(y,x)[1] = pixels.at<cv::Vec3b>(720-y-1,x)[1];
			cv_pixels.at<cv::Vec3b>(y,x)[0] = pixels.at<cv::Vec3b>(720-y-1,x)[2];
	}
	video << cv_pixels;
	
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
	video.release();
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