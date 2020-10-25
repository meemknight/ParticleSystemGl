#include "ParticleSystem.h"
#include <random>
#include <functional>
#include <utility>
#include <fstream>
#include <nmmintrin.h>

std::random_device rng;
std::uniform_real_distribution<float> negDist(-2.0, 2.0);
std::uniform_real_distribution<float> upDist(4.0, 10.0);
std::uniform_real_distribution<float> zeroOneDist(0, 1);

#define R(x, y) x.read((char*)&y, sizeof(y)) 


float mix(float a, float b, float r)
{
	return a * r + b * (1 - r);
}


static const GLfloat vertexBufferData[] = {
 -0.5f, -0.5f, 0.0f,
 0.5f, -0.5f, 0.0f,
 -0.5f, 0.5f, 0.0f,
 0.5f, 0.5f, 0.0f,
};

ParticleSystem::ParticleSystem(unsigned int count, float cicleDuration, ShaderProgram &sp, glm::vec3 color1, glm::vec3 color2,
	glm::vec3 direction1, glm::vec3 direction2,
	float speed1, float speed2) :
	color1(color1), color2(color2), count(count), sp(sp),
	cicleDuration(cicleDuration), direction1(direction1),
	direction2(direction2), speed1(speed1), speed2(speed2)
{
	buildParticleSystem();
}


void ParticleSystem::draw(float deltaTime)
{
	if(running)
	{
		std::uniform_real_distribution<float> xDist(direction1.x, direction2.x);
		std::uniform_real_distribution<float> yDist(direction1.y, direction2.y);
		std::uniform_real_distribution<float> zDist(direction1.z, direction2.z);
		std::uniform_real_distribution<float> speedDist(speed1, speed2);

		float particleDuration = cicleDuration / count;
		float fadvance = deltaTime / particleDuration + accumulatedAdvance;
		int particleAdvance = fadvance;
		accumulatedAdvance = fadvance - particleAdvance;

		int overflow = currentParticle + particleAdvance - count;
		int newPosition;

		if (overflow > 0)
		{
			particleAdvance = count - currentParticle;
			newPosition = overflow;
		}
		else
		{
			newPosition = currentParticle + particleAdvance;
		}

#pragma region resetParticles


		for (int i = currentParticle; i < currentParticle + particleAdvance; i++)
		{
			ParticlePositionsX[i] = position.x;			
		}
	
		for (int i = 0; i < overflow; i++)
		{
			ParticlePositionsX[i] = position.x;
		}

		for (int i = currentParticle; i < currentParticle + particleAdvance; i++)
		{
			ParticlePositionsY[i] = position.y;
		}
	
		for (int i = 0; i < overflow; i++)
		{
			ParticlePositionsY[i] = position.y;
		}

		for (int i = currentParticle; i < currentParticle + particleAdvance; i++)
		{
			ParticlePositionsZ[i] = position.z;
		}

		for (int i = 0; i < overflow; i++)
		{
			ParticlePositionsZ[i] = position.z;
		}


		for (int i = currentParticle; i < currentParticle + particleAdvance; i++)
		{
			glm::vec3 drag = glm::normalize(glm::vec3{ xDist(rng), yDist(rng), zDist(rng) });
			drag *= speedDist(rng);

			ParticleDragX[i] = drag.x;
			ParticleDragY[i] = drag.y;
			ParticleDragZ[i] = drag.z;
		}

		for (int i = 0; i < overflow; i++)
		{
			glm::vec3 drag = glm::normalize(glm::vec3{ xDist(rng), yDist(rng), zDist(rng) });
			drag *= speedDist(rng);

			ParticleDragX[i] = drag.x;
			ParticleDragY[i] = drag.y;
			ParticleDragZ[i] = drag.z;
		}

		currentParticle = newPosition;

#pragma endregion

	
#pragma region apply drag

		/*
		auto drag = glm::vec3(ParticleDrag[i]) * deltaTime;
		ParticlePositionsX[i] += drag.x;
		ParticlePositionsY[i] += drag.y;
		ParticlePositionsZ[i] += drag.z;

		ParticleDrag[i] += gravity * deltaTime;
		*/

		__m128 _deltaTime4 = _mm_set1_ps(deltaTime);
		__m128 _gravityDtX4 = _mm_set1_ps(gravity.x * deltaTime);
		__m128 _gravityDtY4 = _mm_set1_ps(gravity.y* deltaTime);
		__m128 _gravityDtZ4 = _mm_set1_ps(gravity.z * deltaTime);

		for (int i = 0; i < count; i+=4)
		{
			//ParticlePositionsX[i] += ParticleDragX[i] * deltaTime;
			__m128 *_pos4 = (__m128*)&ParticlePositionsX[i];
			__m128 *_drag4 = (__m128*)&ParticleDragX[i];
			*_pos4 = _mm_fmadd_ps(*_drag4, _deltaTime4, *_pos4);

			//ParticleDragX[i] += gravity.x * deltaTime;
			*_drag4 = _mm_add_ps(*_drag4, _gravityDtX4);
		}

	
		for (int i = 0; i < count; i+=4)
		{
			//ParticlePositionsY[i] += ParticleDragY[i] * deltaTime;
			__m128 *_pos4 = (__m128*)&ParticlePositionsY[i];
			__m128 *_drag4 = (__m128*)&ParticleDragY[i];
			*_pos4 = _mm_fmadd_ps(*_drag4, _deltaTime4, *_pos4);

			//ParticleDragY[i] += gravity.y * deltaTime;
			*_drag4 = _mm_add_ps(*_drag4, _gravityDtY4);

		}

		for (int i = 0; i < count; i+=4)
		{
		//ParticlePositionsZ[i] += ParticleDragZ[i] * deltaTime;
			__m128 *_pos4 = (__m128*)&ParticlePositionsZ[i];
			__m128 *_drag4 = (__m128*)&ParticleDragZ[i];
			*_pos4 = _mm_fmadd_ps(*_drag4, _deltaTime4, *_pos4);

		//ParticleDragZ[i] += gravity.z * deltaTime;
			*_drag4 = _mm_add_ps(*_drag4, _gravityDtZ4);
		
		}



#pragma endregion


		//shape
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexShapeId);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//positions
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, vertexPositionId);
		//glBufferData(GL_ARRAY_BUFFER, count * 3 * sizeof(float), ParticlePositions, GL_STREAM_DRAW);
		//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBufferData(GL_ARRAY_BUFFER, count * 3 * sizeof(float), nullptr, GL_STREAM_DRAW);
		
		float *buff = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		//elog(buff);
		int lastFlush = 0;
		for(int i=0; i<count; i++)
		{
			buff[3 * i + 0] = ParticlePositionsX[i];
			buff[3 * i + 1] = ParticlePositionsY[i];
			buff[3 * i + 2] = ParticlePositionsZ[i];

			if(i % 1000 == 0)
			{
				glFlushMappedBufferRange(GL_ARRAY_BUFFER, lastFlush * 3 * sizeof(float), (count - lastFlush) * 3 * sizeof(float));
				lastFlush = count;
			}
		}
		glFlushMappedBufferRange(GL_ARRAY_BUFFER, lastFlush * 3 * sizeof(float), (count - lastFlush) * 3 * sizeof(float));

		glUnmapBuffer(GL_ARRAY_BUFFER);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);


		//colors
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, vertexColorsId);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glVertexAttribDivisor(0, 0);
		glVertexAttribDivisor(1, 1);
		glVertexAttribDivisor(2, 1);

		sp.uniformi("count", count);
		sp.uniformi("firstPos", currentParticle);
		sp.uniform("u_fadeColor", fadeColor.x, fadeColor.y, fadeColor.z);
		sp.uniform("u_fadeWeight", fadeWeight);
		sp.uniform("u_scale", scale);
		sp.uniform("u_kd", kd);
		glm::mat4 position = camera->getObjectToWorld();
		//resetting the rotation
		//projection[0][0] = 1;
		//projection[1][1] = 1;
		//projection[2][2] = 1;

		//projection[0][1] = 0;
		//projection[1][0] = 0;
		//projection[0][2] = 0;
		//projection[2][0] = 0;
		//projection[2][1] = 0;
		//projection[1][2] = 0;

		glm::mat4 projection = camera->getProjectionMatrix();

		glUniformMatrix4fv(sp.getUniformLocation("projectionMatrix"), 1, GL_FALSE, &projection[0][0]);
		glUniformMatrix4fv(sp.getUniformLocation("positionMatrix"), 1, GL_FALSE, &position[0][0]);

		sp.bind();
		if (light && affectedByLight)
		{
			light->bind(sp);
			unsigned int u = sp.getSoubRutineLocation("p_withL", GL_VERTEX_SHADER);
			glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &u);


		}
		else
		{
			int variableIndex = glGetSubroutineUniformLocation(sp.id, GL_VERTEX_SHADER,
				"u_lProgram");

			unsigned int u = sp.getSoubRutineLocation("p_outL", GL_VERTEX_SHADER);

			int n = 0;
			glGetIntegerv(GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS, &n);
			GLuint* indices = new GLuint[n];
			indices[variableIndex] = u;

			glUniformSubroutinesuiv(GL_VERTEX_SHADER, n, indices);

			delete[] indices;

		}

		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);
	}

}

void ParticleSystem::cleanup()
{
	glDeleteBuffers(1, &vertexShapeId);
	glDeleteBuffers(1, &vertexPositionId);
	glDeleteBuffers(1, &vertexColorsId);

	delete[] ParticlePositionsX;
	delete[] ParticlePositionsY;
	delete[] ParticlePositionsZ;
	delete[] ParticleDragX;
	delete[] ParticleDragY;
	delete[] ParticleDragZ;

	currentParticle = 0;
	accumulatedAdvance = 0.f;
}

//todo make the shader internal to the particle system
void ParticleSystem::buildParticleSystem()
{
	currentParticle = 0;
	accumulatedAdvance = 0.f;

	glGenBuffers(1, &vertexShapeId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexShapeId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferData), vertexBufferData, GL_STATIC_DRAW);

	glGenBuffers(1, &vertexPositionId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexPositionId);
	glBufferData(GL_ARRAY_BUFFER, count * 3 * sizeof(float) , nullptr , GL_STREAM_DRAW);

	glGenBuffers(1, &vertexColorsId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexColorsId);
	
	glm::vec3 *colors;
	colors = new glm::vec3[count];

	for(int i=0; i<count; i++)
	{
		colors[i].r = mix(color1.r, color2.r, zeroOneDist(rng));
		colors[i].g = mix(color1.g, color2.g, zeroOneDist(rng));
		colors[i].b = mix(color1.b, color2.b, zeroOneDist(rng));
	}

	glBufferData(GL_ARRAY_BUFFER, count * sizeof(glm::vec3), colors, GL_STREAM_DRAW);

	delete[] colors;

	ParticlePositionsX = new float[count + 3];
	ParticlePositionsY = new float[count + 3];
	ParticlePositionsZ = new float[count + 3];
	ParticleDragX = new float[count + 3];
	ParticleDragY = new float[count + 3];
	ParticleDragZ = new float[count + 3];
	
	for (int i = 0; i < count; i++)
	{
		ParticlePositionsX[i] = 0;
		ParticlePositionsY[i] = 0;
		ParticlePositionsZ[i] = 0;
	}

	for(int i=0; i<3; i++)
	{
		ParticlePositionsX	[count+i]=0;
		ParticlePositionsY	[count+i]=0;
		ParticlePositionsZ	[count+i]=0;
		ParticleDragX		[count+i]=0;
		ParticleDragY		[count+i]=0;
		ParticleDragZ		[count+i]=0;
	}

	std::uniform_real_distribution<float> xDist(direction1.x, direction2.x);
	std::uniform_real_distribution<float> yDist(direction1.y, direction2.y);
	std::uniform_real_distribution<float> zDist(direction1.z, direction2.z);
	std::uniform_real_distribution<float> speedDist(speed1, speed2);


	for(int i=0; i<count; i++)
	{
		glm::vec3 drag = glm::normalize(glm::vec3{ xDist(rng), yDist(rng), zDist(rng) });
		drag *= speedDist(rng);

		ParticleDragX[i] = drag.x;
		ParticleDragY[i] = drag.y;
		ParticleDragZ[i] = drag.z;
	}

}

void ParticleSystem::loadParticleSystem(const char * name)
{

		std::ifstream f(name, std::ios::in | std::ios::binary);

		if (!f.is_open())
		{
			elog("couldn't open file");
			return;
		}

		R(f, this->color1);
		R(f, this->color2);
		R(f, this->fadeColor);
		R(f, this->direction1);
		R(f, this->direction2);
		R(f, this->speed1);
		R(f, this->speed2);
		R(f, this->fadeWeight);
		R(f, this->gravity);
		R(f, this->scale);
		R(f, this->cicleDuration);
		R(f, this->affectedByLight);
		R(f, this->kd);
		R(f, this->count);

		this->buildParticleSystem();

		f.close();
	

}

#undef R