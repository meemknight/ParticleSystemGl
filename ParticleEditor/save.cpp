#include "save.h"
#include <fstream>
#include "tools.h"

std::ostream &operator<< (std::ostream &str, const glm::vec3 v)
{
	str << v.x << v.y << v.z;
	return str;
}

#define W(x, y) x.write((char*)&y, sizeof(y)) 
#define R(x, y) x.read((char*)&y, sizeof(y)) 

void save(const char * name, ParticleSystem ps)
{

	std::string n = name;

	std::ofstream f(n, std::ios::out | std::ios::binary | std::ios::trunc);

	if(!f.is_open())
	{
		elog("couldn't open file");
		return;
	}

	W(f, ps.color1);
	W(f, ps.color2);
	W(f, ps.fadeColor);
	W(f, ps.direction1);
	W(f, ps.direction2);
	W(f, ps.speed1);
	W(f, ps.speed2);
	W(f, ps.fadeWeight);
	W(f, ps.gravity);
	W(f, ps.scale);
	W(f, ps.cicleDuration);
	W(f, ps.affectedByLight);
	W(f, ps.kd);
	W(f, ps.count);

	f.close();
}

#undef W

[[deprecated]]
void load(const char * name, ParticleSystem &ps)
{
	std::string n = name;

	std::ifstream f(n, std::ios::in | std::ios::binary);

	if (!f.is_open())
	{
		elog("couldn't open file");
		return;
	}

	ps.cleanup();

	R(f, ps.color1);
	R(f, ps.color2);
	R(f, ps.fadeColor);
	R(f, ps.direction1);
	R(f, ps.direction2);
	R(f, ps.speed1);
	R(f, ps.speed2);
	R(f, ps.fadeWeight);
	R(f, ps.gravity);
	R(f, ps.scale);
	R(f, ps.cicleDuration);
	R(f, ps.affectedByLight);
	R(f, ps.kd);
	R(f, ps.count);

	ps.buildParticleSystem();

	f.close();
}

#undef R
