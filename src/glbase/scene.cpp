#include "scene.h"
#include <iostream>
#include <cmath>
#include "BoundingBox.h"

#pragma region NODE
 int x = 0;
GLint Node::uniform_model = -1, Node::uniform_color = -1;
GLint Node::attribute_position = 1, Node::attribute_normal = 2;

void Node::InitializePreLink(GLuint program)
{
	glBindAttribLocation(program, attribute_position, "in_position");
	glBindAttribLocation(program, attribute_normal, "in_normal");
}

void Node::InitializePostLink(GLuint program)
{
	uniform_model = glGetUniformLocation(program, "model");
	uniform_color = glGetUniformLocation(program, "color");
}

Node::Node()
	: _transform(), _children(), _parent(nullptr)
{
	
}

void Node::SetTransform(const mat4 &transform)
{
	_transform = transform;
}



void Node::AddChild(Node *child)
{
	_children.push_back(child);
	child->_parent = this;
}

Node* Node::GetParent()
{
	return _parent;
}

glm::mat4 Node::fullTransform()
{
	if (_parent == nullptr)
		
		return _transform;
	else
		return _parent->fullTransform() * _transform;
}


#pragma endregion

#pragma region SHAPE
Shape::Shape()
{}
void Shape::Render()
{ 
	glUniformMatrix4fv(uniform_model, 1, GL_FALSE, glm::value_ptr(fullTransform()));
	ApplyTransformation();
	
	glUniform3fv(uniform_color, 1, glm::value_ptr(_color));
	
}


void Shape::SetAABoundingBox(std::array<vec3,2> &coordsToBeSet, std::array<vec3,2>& coords){
	coordsToBeSet = coords;
}
void Shape::SetBoundingBox(std::array<vec3, 8> &coordsToBeSet, std::array<vec3, 8> &coords){
	coordsToBeSet = coords;
}
void Shape::ApplyTransformation(){
	std::array<vec3, 8> arrayTemp;
	for (int i = 0; i < 8; i++){
	    glm::vec4 v4(init_bbox_coords[i], 1);
	    v4 = (fullTransform()*v4);
		vec3 v3(v4);
		arrayTemp[i] = v3;
	}   
	bbox_coords = arrayTemp;
	aabbox_coords[1] = GetAABBBFromBB(arrayTemp)[1];
	aabbox_coords[0] = GetAABBBFromBB(arrayTemp)[0];
		
}

std::array<vec3,2> Shape::GetBoundingBox() const{
	return aabbox_coords;
}

Shape::~Shape()
{
	if (_vertexBuffer != BAD_BUFFER)
		glDeleteBuffers(1, &_vertexBuffer);

	if (_indexBuffer != BAD_BUFFER)
		glDeleteBuffers(1, &_indexBuffer);
}

#pragma endregion

#pragma region BOX
Box::Box(vec3 size, vec3 color) : _size(size)
{	
	
	_vertexBuffer = _indexBuffer = BAD_BUFFER;
	_color = color;
	 vertices =  
	{ {
		{ vec3(0, 0, 0), vec3(0, -1, 0) },
		{ vec3(1, 0, 0), vec3(0, -1, 0) },
		{ vec3(0, 0, 1), vec3(0, -1, 0) },

		{ vec3(0, 0, 1), vec3(0, -1, 0) },
		{ vec3(1, 0, 0), vec3(0, -1, 0) },
		{ vec3(1, 0, 1), vec3(0, -1, 0) },


		{ vec3(1, 0, 0), vec3(1, 0, 0) },
		{ vec3(1, 1, 0), vec3(1, 0, 0) },
		{ vec3(1, 0, 1), vec3(1, 0, 0) },

		{ vec3(1, 0, 1), vec3(1, 0, 0) },
		{ vec3(1, 1, 0), vec3(1, 0, 0) },
		{ vec3(1, 1, 1), vec3(1, 0, 0) },


		{ vec3(1, 1, 0), vec3(0, 1, 0) },
		{ vec3(0, 1, 1), vec3(0, 1, 0) },
		{ vec3(1, 1, 1), vec3(0, 1, 0) },

		{ vec3(0, 1, 0), vec3(0, 1, 0) },
		{ vec3(0, 1, 1), vec3(0, 1, 0) },
		{ vec3(1, 1, 0), vec3(0, 1, 0) },


		{ vec3(0, 1, 1), vec3(-1, 0, 0) },
		{ vec3(0, 1, 0), vec3(-1, 0, 0) },
		{ vec3(0, 0, 1), vec3(-1, 0, 0) },

		{ vec3(0, 1, 0), vec3(-1, 0, 0) },
		{ vec3(0, 0, 0), vec3(-1, 0, 0) },
		{ vec3(0, 0, 1), vec3(-1, 0, 0) },


		{ vec3(0, 0, 1), vec3(0, 0, 1) },
		{ vec3(1, 0, 1), vec3(0, 0, 1) },
		{ vec3(0, 1, 1), vec3(0, 0, 1) },

		{ vec3(1, 0, 1), vec3(0, 0, 1) },
		{ vec3(1, 1, 1), vec3(0, 0, 1) },
		{ vec3(0, 1, 1), vec3(0, 0, 1) },


		{ vec3(0, 0, 0), vec3(0, 0, -1) },
		{ vec3(0, 1, 0), vec3(0, 0, -1) },
		{ vec3(1, 0, 0), vec3(0, 0, -1) },

		{ vec3(1, 0, 0), vec3(0, 0, -1) },
		{ vec3(0, 1, 0), vec3(0, 0, -1) },
		{ vec3(1, 1, 0), vec3(0, 0, -1) }
	} };
	
	for (unsigned int x = 0; x < 36; x++){
		vertices[x].position = (vertices[x].position - 0.5f) * _size;
	}
	SetAABoundingBox(aabbox_coords, GetAABBBFromVertices(vertices));

	SetBoundingBox(init_bbox_coords, GetBBFromAABB(aabbox_coords));

	// Create Vertex Array Objects
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	// Generate Vertex Buffer
	glGenBuffers(1, &_vertexBuffer);

	// Fill Vertex Buffer
	glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

	// Set Vertex Attributes
	glEnableVertexAttribArray(attribute_position);
	glVertexAttribPointer(attribute_position, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPositionNormal), (const GLvoid*)0);
	glEnableVertexAttribArray(attribute_normal);
	glVertexAttribPointer(attribute_normal, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPositionNormal), (const GLvoid*)(0 + sizeof(vec3)));

	glBindVertexArray(0);

	debugGLError();
}

void Box::Render()
{
	Shape::Render();
	glBindVertexArray(_vao);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}


#pragma endregion

#pragma region CYLINDER
const int Cylinder::slices = 360;
Cylinder::Cylinder(double radius, double height, vec3 color) : _radius(radius), _height(height)
{
	_vertexBuffer = _indexBuffer = BAD_BUFFER;
	_color = color;
	
	// Middle vertex of top circle
	vertices[0] = { vec3(0, _height / 2, 0), vec3(0, 1, 0) };
	// Middle vertex of bottom circle
	vertices[slices] = { vec3(0, -(_height / 2), 0), vec3(0, -1, 0) };

	for (int i = 1; i < slices; i++)

	{
		double theta = 2 * glm::pi<double>() / slices - i;
		//vertices of top circle
		vertices[i] = { vec3(sin(theta)*_radius, _height / 2, cos(theta)*_radius), vec3(0, 1, 0) };
		//vertices of bottom circle
		vertices[i + slices] = { vec3(sin(theta)*_radius, -(_height / 2), cos(theta)*_radius), vec3(0, -1, 0) };
		//vertices of sides
		vertices[2 * i + slices * 2] = { vec3(sin(theta)*_radius, _height / 2, cos(theta)*_radius), glm::normalize(vec3(sin(theta)*_radius, 0, cos(theta)*_radius)) };
		vertices[2 * i + (slices * 2 + 1)] = { vec3(sin(theta)*_radius, -(_height / 2), cos(theta)*_radius), glm::normalize(vec3(sin(theta)*_radius, 0, cos(theta)*_radius)) };
	}
	SetAABoundingBox(aabbox_coords, GetAABBBFromVertices(vertices));

	SetBoundingBox(init_bbox_coords, GetBBFromAABB(aabbox_coords));



	// Create Vertex Array Object
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	// Generate Vertex Buffer
	glGenBuffers(1, &_vertexBuffer);

	// Fill Vertex Buffer
	glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

	// Set Vertex Attributes
	glEnableVertexAttribArray(attribute_position);
	glVertexAttribPointer(attribute_position, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPositionNormal), (const GLvoid*)0);
	glEnableVertexAttribArray(attribute_normal);
	glVertexAttribPointer(attribute_normal, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPositionNormal), (const GLvoid*)(0 + sizeof(vec3)));

	glBindVertexArray(0);

	debugGLError();
}

void Cylinder::Render()
{
	Shape::Render();
	glBindVertexArray(_vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, slices);
	glDrawArrays(GL_TRIANGLE_FAN, slices, slices);
	glDrawArrays(GL_TRIANGLE_STRIP, slices * 2, slices * 2);
}

#pragma endregion
#pragma region SPHERE

Sphere::Sphere(double radius, vec3 color) : _radius(radius)
{
	_vertexBuffer = _indexBuffer = BAD_BUFFER;
	_color = color;

	const int nStacks = 40;
	const int nSlices = 40;

	VertexPositionNormal vertices[9360];
	int n = 0;

	for (int stack = 0; stack < nStacks; stack++)
	{
		double theta1 = ((double)(stack) / nStacks) * glm::pi<double>();
		double theta2 = ((double)(stack + 1) / nStacks) * glm::pi<double>();

		for (int slice = 0; slice < nSlices; slice++)
		{
			double phi1 = ((double)(slice) / nSlices) * 2 * glm::pi<double>();
			double phi2 = ((double)(slice + 1) / nSlices) * 2 * glm::pi<double>();

			// vec3 vertex1 = vec3(_radius*sin(theta1)*cos(phi1), _radius*sin(theta1)*sin(phi1), _radius*cos(theta1));
			// vec3 vertex2 = vec3(_radius*sin(theta1)*cos(phi2), _radius*sin(theta1)*sin(phi2), _radius*cos(theta1));
			// vec3 vertex3 = vec3(_radius*sin(theta2)*cos(phi2), _radius*sin(theta2)*sin(phi2), _radius*cos(theta2));
			// vec3 vertex4 = vec3(_radius*sin(theta2)*cos(phi1), _radius*sin(theta2)*sin(phi1), _radius*cos(theta2));

			if (stack == 0)
			{
				vec3 vertex1 = vec3(_radius*glm::sin(theta1)*glm::cos(phi1), _radius*glm::sin(theta1)*glm::sin(phi1), _radius*glm::cos(theta1));
				vec3 vertex2 = vec3(_radius*glm::sin(theta2)*glm::cos(phi2), _radius*glm::sin(theta2)*glm::sin(phi2), _radius*glm::cos(theta2));
				vec3 vertex3 = vec3(_radius*glm::sin(theta2)*glm::cos(phi1), _radius*glm::sin(theta2)*glm::sin(phi1), _radius*glm::cos(theta2));
				vec3 normal = -glm::normalize(glm::cross(vertex2 - vertex1, vertex3 - vertex1));
				vertices[n] = { vertex1, normal };
				vertices[n + 1] = { vertex2, normal };
				vertices[n + 2] = { vertex3, normal };
				n += 3;
			}
			else if (stack + 1 == nStacks)
			{
				vec3 vertex1 = vec3(_radius*glm::sin(theta2)*glm::cos(phi2), _radius*glm::sin(theta2)*glm::sin(phi2), _radius*glm::cos(theta2));
				vec3 vertex2 = vec3(_radius*glm::sin(theta1)*glm::cos(phi1), _radius*glm::sin(theta1)*glm::sin(phi1), _radius*glm::cos(theta1));
				vec3 vertex3 = vec3(_radius*glm::sin(theta1)*glm::cos(phi2), _radius*glm::sin(theta1)*glm::sin(phi2), _radius*glm::cos(theta1));
				vec3 normal = -glm::normalize(glm::cross(vertex2 - vertex1, vertex3 - vertex1));
				vertices[n] = { vertex1, normal };
				vertices[n + 1] = { vertex2, normal };
				vertices[n + 2] = { vertex3, normal };
				n += 3;
			}
			else
			{
				vec3 vertex1 = vec3(_radius*glm::sin(theta1)*glm::cos(phi1), _radius*glm::sin(theta1)*glm::sin(phi1), _radius*glm::cos(theta1));
				vec3 vertex2 = vec3(_radius*glm::sin(theta1)*glm::cos(phi2), _radius*glm::sin(theta1)*glm::sin(phi2), _radius*glm::cos(theta1));
				vec3 vertex3 = vec3(_radius*glm::sin(theta2)*glm::cos(phi1), _radius*glm::sin(theta2)*glm::sin(phi1), _radius*glm::cos(theta2));
				vec3 normal1 = -glm::normalize(glm::cross(vertex2 - vertex1, vertex3 - vertex1));
				vertices[n] = { vertex1, normal1 };
				vertices[n + 1] = { vertex2, normal1 };
				vertices[n + 2] = { vertex3, normal1 };

				vec3 vertex4 = vec3(_radius*glm::sin(theta1)*glm::cos(phi2), _radius*glm::sin(theta1)*glm::sin(phi2), _radius*glm::cos(theta1));
				vec3 vertex5 = vec3(_radius*glm::sin(theta2)*glm::cos(phi2), _radius*glm::sin(theta2)*glm::sin(phi2), _radius*glm::cos(theta2));
				vec3 vertex6 = vec3(_radius*glm::sin(theta2)*glm::cos(phi1), _radius*glm::sin(theta2)*glm::sin(phi1), _radius*glm::cos(theta2));
				vec3 normal2 = -glm::normalize(glm::cross(vertex2 - vertex1, vertex3 - vertex1));
				vertices[n + 3] = { vertex4, normal2 };
				vertices[n + 4] = { vertex5, normal2 };
				vertices[n + 5] = { vertex6, normal2 };
				n += 6;
			}
		}
	}

	// Create Vertex Array Object
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	// Generate Vertex Buffer
	glGenBuffers(1, &_vertexBuffer);

	// Fill Vertex Buffer
	glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Set Vertex Attributes
	glEnableVertexAttribArray(attribute_position);
	glVertexAttribPointer(attribute_position, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPositionNormal), (const GLvoid*)0);
	glEnableVertexAttribArray(attribute_normal);
	glVertexAttribPointer(attribute_normal, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPositionNormal), (const GLvoid*)(0 + sizeof(vec3)));

	glBindVertexArray(0);

	debugGLError();

}
void Sphere::Init(const mat4& mat){

}
void Sphere::Render()
{
	Shape::Render();
	glBindVertexArray(_vao);
	glDrawArrays(GL_TRIANGLES, 0, 9360);

}
#pragma endregion

bool Collisions::AABBDetection(const Shape& shape1,const Shape& shape2){
	
	std::array<vec3, 2> b1 = shape1.GetBoundingBox();
	std::array<vec3, 2> b2 = shape2.GetBoundingBox();

	/*std::cout << "Le point min du cannon est " << b1[1].x << std::endl;
	std::cout << "Le point max de la fuse est " << b2[0].x << std::endl;
	std::cout << (b1[0].x > b2[1].x) << std::endl;
	std::cout << (b1[1].x < b2[0].x) << std::endl;
	std::cout << (b1[0].y > b2[1].y) << std::endl;
	std::cout << (b1[1].y < b2[0].y) << std::endl;
	std::cout << (b1[0].z > b2[1].z) << std::endl;
	std::cout << (b1[1].z < b2[0].z) << "\n\n";*/

	return(b1[0].x > b2[1].x &&
		b1[1].x < b2[0].x &&
		b1[0].y > b2[1].y &&
		b1[1].y < b2[0].y &&
		b1[0].z > b2[1].z &&
		b1[1].z < b2[0].z);

}
bool Collisions::OBBDetection(const Shape& shape1, const Shape& shape2){
//TODO
	return false;
}

std::array<vec3, 15> GetNormalsFromBBs(const std::array<vec3, 8>& bb1, const std::array<vec3, 8>& bb2){
	std::array<vec3, 15> normals;
	//TODO
	return normals;
}
