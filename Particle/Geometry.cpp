#pragma once
#include "Geometry.h"
#include <iostream>
#include <vector>


float solve_eq(float a, float b, float c) {
	float x1, x2, discriminant;
	discriminant = b*b - 4*a*c;
	
	if (discriminant > 0.0f) {
		x1 = (-b + sqrt(discriminant)) / (2*a);
		x2 = (-b - sqrt(discriminant)) / (2*a);
		if (x1 >= 0 && x1 <= 1) {
			return x1;
		} else if (x2 >= 0 && x2 <= 1) {
			return x2;
		}
	}
	
	else if (discriminant == 0.0f) {
		x1 = (-b + sqrt(discriminant)) / (2*a);
		if (x1 >= 0 && x1 <= 1) {
			return x1;
		}
	}
	
	return -1;
    
}

//****************************************************
// Plane
//****************************************************

Plane::Plane(const glm::vec3& point, const glm::vec3& normalVect){
	normal = glm::normalize(normalVect);
	dconst = -glm::dot(point, normal);
};

Plane::Plane(const glm::vec3& point0, const glm::vec3& point1, const glm::vec3& point2){
	glm::vec3 v1 = point1 - point0;
	glm::vec3 v2 = point2 - point0;
	normal = glm::normalize(glm::cross(v1, v2));
	dconst = -glm::dot(point0, normal);
};

void Plane::setPosition(const glm::vec3& newPos){
	dconst = -glm::dot(newPos, normal);
};

bool Plane::isInside(const glm::vec3& point){
	float dist;
	dist = glm::dot(point, normal) + dconst;
	if (dist > 1.e-7)
		return false;
	else
		return true;
};

float Plane::distPoint2Plane(const glm::vec3& point){
	float dist;
	return dist = glm::dot(point, normal) + dconst;
};

glm::vec3 Plane::closestPointInPlane(const glm::vec3& point){
	glm::vec3 closestP;
	float r = (-dconst - glm::dot(point, normal));
	return closestP = point + r*normal;
};

bool Plane::intersecSegment(const glm::vec3& point1, const glm::vec3& point2, glm::vec3& pTall){
	if (distPoint2Plane(point1)*distPoint2Plane(point2) > 0)	return false;
	float r = (-dconst - glm::dot(point1, normal)) / glm::dot((point2 - point1), normal);
	pTall = (1 - r)*point1 + r*point2;
	return true;
};


//****************************************************
// Triangle
//****************************************************
Triangle::Triangle(const glm::vec3& point0, const glm::vec3& point1, const glm::vec3& point2){
	vertex1 = point0;
	vertex2 = point1;
	vertex3 = point2;
	glm::vec3 v1 = point1 - point0;
	glm::vec3 v2 = point2 - point0;
	normal = glm::normalize(glm::cross(v1, v2));
	dconst = -glm::dot(point0, normal);
};

float area(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3) {
	glm::vec3 c = glm::cross(v2-v1, v3-v1);
	float norm = glm::length(c);
	return norm/2.0f;
}

bool Triangle::isInside(const glm::vec3& position) {
	float res = area(position, vertex2, vertex3)+ area(vertex1, position, vertex3) + area(vertex1, vertex2, position) - area(vertex1, vertex2, vertex3);
	return res <= 0.1;
}

//****************************************************
// Sphere
//****************************************************

Sphere::Sphere(const glm::vec3& point, const float& r) {
	center = point;
	radius = r;
}

void Sphere::setPosition(const glm::vec3& newPos) {
	center = newPos;
}

bool Sphere::isInside(const glm::vec3& point) {
	float dis = glm::distance(point, center);
	return dis <= radius;
}

// Returns point, normal
glm::vec3 Sphere::getPointOnBoundary(const glm::vec3& p_n, const glm::vec3& p_a) {
	// glm::vec3 dir = p_n - center;
	// dir = glm::normalize(dir);
	// return center + dir * radius;
	glm::vec3 v = p_n - p_a;
	float alpha, beta, yi;
	alpha = glm::dot(v,v);
	beta = glm::dot(2.0f * v, p_a - center);
	yi = glm::dot(center, center) + glm::dot(p_a, p_a) - glm::dot(2.0f * p_a, center) - (radius*radius);
	float result = solve_eq(alpha, beta, yi);
	glm::vec3 pl;
	if (result >= 0 && result <= 1) pl = p_a + result * v;
	else pl = p_n;
	return pl;
}