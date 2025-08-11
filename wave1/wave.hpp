#ifndef WAVE_HPP
#define WAVE_HPP

#include <vector>
#include <iostream>
#include <chrono>

#include <glad/glad.h>

class Wave {
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	GLfloat* vertices;
	GLuint* elements;
	size_t elements_size;

public:
	Wave(double side_length, size_t edges_per_side, double height = 0.0) {
		size_t vertices_size = 3 * (edges_per_side + 1) * (edges_per_side + 1);
		elements_size = 6 * edges_per_side * edges_per_side;
		vertices = new GLfloat[vertices_size];
		elements = new GLuint[elements_size];
		double delta = side_length / edges_per_side;
		double origin = -side_length / 2;

		// generate vertex coordinates from (origin, origin) to (-origin, -origin)
		for (size_t row = 0; row <= edges_per_side; row++) {
			for (size_t column = 0; column <= edges_per_side; column++) {
				vertices[row * (edges_per_side + 1) * 3 + column * 3 + 0] = origin + delta * column;
				vertices[row * (edges_per_side + 1) * 3 + column * 3 + 1] = height;
				vertices[row * (edges_per_side + 1) * 3 + column * 3 + 2] = origin + delta * row;
			}
		}

		// generate triangles
		size_t vertices_per_side = edges_per_side + 1;
		for (size_t row = 0; row < edges_per_side; row++) {
			for (size_t column = 0; column < edges_per_side; column++) {
				size_t top_left = vertices_per_side * row + column;
				
				// top left triangle
				elements[row * edges_per_side * 6 + column * 6 + 0] = top_left;
				elements[row * edges_per_side * 6 + column * 6 + 1] = top_left + vertices_per_side;
				elements[row * edges_per_side * 6 + column * 6 + 2] = top_left + 1;

				// bottom right triangle
				elements[row * edges_per_side * 6 + column * 6 + 3] = top_left + 1;
				elements[row * edges_per_side * 6 + column * 6 + 4] = top_left + vertices_per_side;
				elements[row * edges_per_side * 6 + column * 6 + 5] = top_left + vertices_per_side + 1;
			}
		}

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		glBindVertexArray(vao);
		
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices_size, vertices, GL_STATIC_DRAW);
		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*) 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * elements_size, elements, GL_STATIC_DRAW);
	}

	void draw() const {
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, elements_size, GL_UNSIGNED_INT, 0);
	}

	~Wave() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ebo);

		delete[] vertices;
		delete[] elements;
	}
};

#endif