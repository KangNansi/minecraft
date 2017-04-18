#pragma once
#include "world.h"
#include "engine/utils/types_3d.h"
#include <map>

#define WCHUNK_SIZE MAT_SIZE*NYCube::CUBE_SIZE*NYChunk::CHUNK_SIZE

class MapPosition {
public:
	int x;
	int y;

	MapPosition() { x = 0; y = 0; }

	MapPosition(int _x, int _y) {
		x = _x; y = _y;
	}

	bool operator=(const MapPosition &p) {
		return x == p.x && y == p.y;
	}

	bool operator<(const MapPosition &p) const {
		return x < p.x && y < p.y;
	}

	MapPosition& operator+(const MapPosition &p) {
		return MapPosition(x + p.x, y + p.y);
	}
};
typedef struct MapPosition MapPosition;

class OpenWorld {
private:
	std::map<MapPosition, NYWorld*> maps;
	NYVert3Df *camera_position;

public:
	OpenWorld() {};
	~OpenWorld() {};

	void init(NYVert3Df *cam) {
		camera_position = cam;
		MapPosition pos;
		pos.x = camera_position->X / (MAT_SIZE*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE);
		pos.y = camera_position->Y / (MAT_SIZE*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE);

		maps[pos] = new NYWorld();
		maps[pos]->_FacteurGeneration = 5;
		maps[pos]->init_world(pos.x,pos.y);
		

	}

	void renderNear(MapPosition pos, GLuint shader) {
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, maps[pos]->_blocTexture->Texture);
		glUniform1i(glGetUniformLocation(shader, "Texture"), 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, maps[pos]->_blocNormal->Texture);
		glUniform1i(glGetUniformLocation(shader, "TexNormal"), 1);
		glPushMatrix();
		glTranslatef(pos.x*WCHUNK_SIZE, pos.y*WCHUNK_SIZE, 0);
		maps[pos]->render_opaque_vbo();
		glPopMatrix();

		glDisable(GL_TEXTURE_2D);
	}

	void renderFar(MapPosition pos, GLuint shader) {
		
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, maps[pos]->_blocTexture->Texture);
		glUniform1i(glGetUniformLocation(shader, "Texture"), 0);
		glPushMatrix();
		glTranslatef(pos.x*WCHUNK_SIZE, pos.y*WCHUNK_SIZE, 0);
		maps[pos]->render_opaque_vbo();
		glPopMatrix();
		glDisable(GL_TEXTURE_2D);
	}

	void render(GLuint shader, GLuint simple) {
		MapPosition pos;
		pos.x = camera_position->X / (MAT_SIZE*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE);
		if (camera_position->X < 0) pos.x -= 1;
		pos.y = camera_position->Y / (MAT_SIZE*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE);
		if (camera_position->Y < 0) pos.y -= 1;
		for(int i=-1;i<=1;i++)
			for (int j = -1; j <= 1; j++) {
				MapPosition rpos(pos.x + i, pos.y + j);
				if (maps[rpos] == nullptr) {
					maps[rpos] = new NYWorld();
					maps[rpos]->_FacteurGeneration = 5;
					maps[rpos]->init_world(rpos.x, rpos.y);
				}
				renderNear(rpos, shader);
			}
		glUseProgram(simple);
		for(int i=-3;i<=3;i++)
			for (int j = -3; j <= 3; j++) {
				if ((i <= 1 && i >= -1) && (j <= 1 && j >= -1)) continue;
				MapPosition rpos(pos.x + i, pos.y + j);
				if (maps[rpos] == nullptr) {
					maps[rpos] = new NYWorld();
					maps[rpos]->_FacteurGeneration = 5;
					maps[rpos]->init_world(rpos.x,rpos.y);
				}
				renderFar(rpos, simple);
			}
	}

	void render_water(GLuint shader, float elapsed, NYFloatMatrix &invertView) {
		MapPosition pos;
		pos.x = camera_position->X / (MAT_SIZE*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE);
		if (camera_position->X < 0) pos.x -= 1;
		pos.y = camera_position->Y / (MAT_SIZE*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE);
		if (camera_position->Y < 0) pos.y -= 1;
		glUseProgram(shader);

		GLuint elap = glGetUniformLocation(shader, "elapsed");
		glUniform1f(elap, NYRenderer::_DeltaTimeCumul);

		GLuint amb = glGetUniformLocation(shader, "ambientLevel");
		glUniform1f(amb, 0.4);

		GLuint invView = glGetUniformLocation(shader, "invertView");
		glUniformMatrix4fv(invView, 1, true, invertView.Mat.t);
		for (int i = -3; i <= 3; i++)
			for (int j = -3; j <= 3; j++) {
				MapPosition rpos(pos.x + i, pos.y + j);
				if (maps[rpos] == nullptr) {
					maps[rpos] = new NYWorld();
					maps[rpos]->_FacteurGeneration = 5;
					maps[rpos]->init_world(rpos.x,rpos.y);
				}
				glPushMatrix();
				glTranslatef(rpos.x*WCHUNK_SIZE, rpos.y*WCHUNK_SIZE, 0);
				maps[rpos]->render_water_vbo();
				glPopMatrix();
			}
	}

	bool interDroiteMatrice(NYVert3Df origine, NYVert3Df direction, NYVert3Df &inter, NYVert3Df &cube_result) {
		MapPosition pos;
		pos.x = origine.X / (MAT_SIZE*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE);
		pos.y = origine.Y / (MAT_SIZE*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE);
		origine.X -= pos.x*WCHUNK_SIZE;
		origine.Y -= pos.y*WCHUNK_SIZE;
		bool b = maps[pos]->interDroiteMatrice(origine, direction, inter, cube_result);
		inter += NYVert3Df(pos.x*WCHUNK_SIZE, pos.y*WCHUNK_SIZE, 0);
		cube_result += NYVert3Df(pos.x*MAT_SIZE*NYChunk::CHUNK_SIZE, pos.y*MAT_SIZE*NYChunk::CHUNK_SIZE, 0);
		return b;
	}

	void deleteCube(int x, int y, int z) {
		MapPosition pos;
		pos.x = x/(MAT_SIZE*NYChunk::CHUNK_SIZE);
		pos.y = y/(MAT_SIZE*NYChunk::CHUNK_SIZE);
		x = x%(MAT_SIZE*NYChunk::CHUNK_SIZE);
		y = y%(MAT_SIZE*NYChunk::CHUNK_SIZE);
		maps[pos]->deleteCube(x, y, z);
	}

	NYAxis getMinCol(NYVert3Df position, NYVert3Df dir, float width, float height, float & valueColMin, bool oneShot) {
		MapPosition pos;
		pos.x = position.X / (MAT_SIZE*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE);
		if (position.X < 0) pos.x -= 1;
		pos.y = position.Y / (MAT_SIZE*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE);
		if (position.Y < 0) pos.y -= 1;
		position.X -= pos.x*WCHUNK_SIZE;
		position.Y -= pos.y*WCHUNK_SIZE;
		return maps[pos]->getMinCol(position, dir, width, height, valueColMin, oneShot);

	}

};

