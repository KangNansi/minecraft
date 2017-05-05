#ifndef __WORLD_H__
#define __WORLD_H__

#include "gl/glew.h"
#include "gl/freeglut.h" 
#include "engine/utils/types_3d.h"
#include "cube.h"
#include "chunk.h"
#include "NYPerlin.h"
#include "engine/render/graph/tex_manager.h"

#include "my_physics.h"

typedef uint8 NYAxis;
#define NY_AXIS_X 0x01
#define NY_AXIS_Y 0x02
#define NY_AXIS_Z 0x04

#define MAT_SIZE 5 //en nombre de chunks
#define MAT_HEIGHT 2 //en nombre de chunks
#define MAT_SIZE_CUBES (MAT_SIZE * NYChunk::CHUNK_SIZE)
#define MAT_HEIGHT_CUBES (MAT_HEIGHT * NYChunk::CHUNK_SIZE)


class NYWorld
{
public :
	NYChunk * _Chunks[MAT_SIZE][MAT_SIZE][MAT_HEIGHT];
	int _MatriceHeights[MAT_SIZE_CUBES][MAT_SIZE_CUBES];
	float _FacteurGeneration;
	NYTexFile * _blocTexture;
	NYTexFile * _blocNormal;
	static const int WATER_LEVEL = 17;
	static const int PERLIN_DIV = 50;


	

	NYWorld()
	{
		_FacteurGeneration = 1.0;

		//On crée les chunks
		for(int x=0;x<MAT_SIZE;x++)
			for(int y=0;y<MAT_SIZE;y++)
				for(int z=0;z<MAT_HEIGHT;z++)
					_Chunks[x][y][z] = new NYChunk();

		for(int x=0;x<MAT_SIZE;x++)
			for(int y=0;y<MAT_SIZE;y++)
				for(int z=0;z<MAT_HEIGHT;z++)
				{
					NYChunk * cxPrev = NULL;
					if(x > 0)
						cxPrev = _Chunks[x-1][y][z];
					NYChunk * cxNext = NULL;
					if(x < MAT_SIZE-1)
						cxNext = _Chunks[x+1][y][z];

					NYChunk * cyPrev = NULL;
					if(y > 0)
						cyPrev = _Chunks[x][y-1][z];
					NYChunk * cyNext = NULL;
					if(y < MAT_SIZE-1)
						cyNext = _Chunks[x][y+1][z];

					NYChunk * czPrev = NULL;
					if(z > 0)
						czPrev = _Chunks[x][y][z-1];
					NYChunk * czNext = NULL;
					if(z < MAT_HEIGHT-1)
						czNext = _Chunks[x][y][z+1];

					_Chunks[x][y][z]->setVoisins(cxPrev,cxNext,cyPrev,cyNext,czPrev,czNext);
				}

					
	}

	inline NYCube * getCube(int x, int y, int z)
	{	
		if(x < 0)x = 0;
		if(y < 0)y = 0;
		if(z < 0)z = 0;
		if(x >= MAT_SIZE * NYChunk::CHUNK_SIZE) x = (MAT_SIZE * NYChunk::CHUNK_SIZE)-1;
		if(y >= MAT_SIZE * NYChunk::CHUNK_SIZE) y = (MAT_SIZE * NYChunk::CHUNK_SIZE)-1;
		if(z >= MAT_HEIGHT * NYChunk::CHUNK_SIZE) z = (MAT_HEIGHT * NYChunk::CHUNK_SIZE)-1;

		return &(_Chunks[x / NYChunk::CHUNK_SIZE][y / NYChunk::CHUNK_SIZE][z / NYChunk::CHUNK_SIZE]->_Cubes[x % NYChunk::CHUNK_SIZE][y % NYChunk::CHUNK_SIZE][z % NYChunk::CHUNK_SIZE]);
	}

	void updateCube(int x, int y, int z)
	{
		if (x < 0)x = 0;
		if (y < 0)y = 0;
		if (z < 0)z = 0;
		if (x >= MAT_SIZE * NYChunk::CHUNK_SIZE)x = (MAT_SIZE * NYChunk::CHUNK_SIZE) - 1;
		if (y >= MAT_SIZE * NYChunk::CHUNK_SIZE)y = (MAT_SIZE * NYChunk::CHUNK_SIZE) - 1;
		if (z >= MAT_HEIGHT * NYChunk::CHUNK_SIZE)z = (MAT_HEIGHT * NYChunk::CHUNK_SIZE) - 1;

		NYChunk * chk = _Chunks[x / NYChunk::CHUNK_SIZE][y / NYChunk::CHUNK_SIZE][z / NYChunk::CHUNK_SIZE];

		chk->disableHiddenCubes();
		chk->toVbo();

		for (int i = 0; i<6; i++)
			if (chk->Voisins[i])
			{
				chk->Voisins[i]->disableHiddenCubes();
				chk->Voisins[i]->toVbo();
			}
	}


	void deleteCube(int x, int y, int z)
	{
		NYCube * cube = getCube(x, y, z);
		cube->_Draw = false;
		cube->_Type = CUBE_AIR;
		updateCube(x, y, z);
	}

	void addCube(int x, int y, int z)
	{
		NYCube * cube = getCube(x, y, z);
		cube->_Draw = true;
		cube->_Type = CUBE_TERRE;
		updateCube(x, y, z);
	}


	//Création d'une pile de cubes
	//only if zero permet de ne générer la  pile que si sa hauteur actuelle est de 0 (et ainsi de ne pas regénérer de piles existantes)
	void load_pile(int x, int y, int height, bool onlyIfZero = true)
	{
		if (onlyIfZero && _MatriceHeights[x][y] != 0)
			return;
		if (height <= 0) height = 1;
		for (int i = 0; i < MAT_HEIGHT_CUBES; i++) {
			if ((i>height && i<WATER_LEVEL) )
				getCube(x, y, i)->_Type = CUBE_EAU;
			else if(i<height)
				getCube(x, y, i)->_Type = CUBE_TERRE;
			else if(i==height)
				getCube(x, y, i)->_Type = CUBE_HERBE;
			else
				getCube(x, y, i)->_Type = CUBE_AIR;
			if(i>height) getCube(x, y, i)->_Draw = true;
			else getCube(x, y, i)->_Draw = false;
		}
		_MatriceHeights[x][y] = height;
	}

	//Creation du monde entier, en utilisant le mouvement brownien fractionnaire
	void generate_piles(int x1, int y1,
		int x2, int y2, 
		int x3, int y3,
		int x4, int y4, int prof, int profMax = -1)
	{
		/*for (int x = 0; x < MAT_SIZE_CUBES; x++)
			for (int y = 0; y < MAT_SIZE_CUBES; y++)
				load_pile(x, y, perlin.sample(x/3, y/3, 1.11)*32);
		return;*/


		if ((x3 - x1)*(x3 - x1) + (y3 - y1)*(y3 - y1) <= 2)
			return;
		int xcenter = (x1 + x2 + x3 + x4)/4;
		int ycenter = (y1 + y2 + y3 + y4)/4;
		int height_center = _MatriceHeights[x1][y1] 
						  + _MatriceHeights[x2][y2] 
						  + _MatriceHeights[x3][y3] 
						  + _MatriceHeights[x4][y4];
		height_center = floor(height_center/4.);
		height_center += (randf() * 16-8);
		load_pile(xcenter, ycenter, height_center);
		int h = floor((_MatriceHeights[x1][y1] + _MatriceHeights[x2][y2] + height_center) / 3.);
		h += (randf()* 16 - 8);
		load_pile(xcenter, y1, h);
		h = floor((_MatriceHeights[x2][y2] + _MatriceHeights[x3][y3] + height_center) / 3.);
		h += (randf() * 16 - 8) ;
		load_pile(x2, ycenter, h);
		h = floor((_MatriceHeights[x3][y3] + _MatriceHeights[x4][y4] + height_center) / 3.);
		h += (randf() * 16 - 8);
		load_pile(xcenter, y3, h);
		h = floor((_MatriceHeights[x4][y4] + _MatriceHeights[x1][y1] + height_center) / 3.);
		h += (randf() * 16 - 8);
		load_pile(x4, ycenter, h);
		generate_piles(x1, y1, xcenter, y1, xcenter, ycenter, x1, ycenter, prof + 1);
		generate_piles(xcenter, y2, x2, y2, x2, ycenter, xcenter, ycenter, prof + 1);
		generate_piles(xcenter, ycenter, x3, ycenter, x3, y3, xcenter, y3, prof + 1);
		generate_piles(x4, ycenter, xcenter, ycenter, xcenter, y4, x4, y4, prof + 1);
	}


	//On utilise un matrice temporaire _MatriceHeightsTmp à déclarer
	//Penser à appeler la fonction a la fin de la génération (plusieurs fois si besoin)
	void lisse(void)
	{
		int _MatriceHeightsTmp[MAT_SIZE_CUBES][MAT_SIZE_CUBES];
		int sizeWidow = 4;
		memset(_MatriceHeightsTmp, 0x00, sizeof(int)*MAT_SIZE_CUBES*MAT_SIZE_CUBES);
		for (int x = 0; x<MAT_SIZE_CUBES; x++)
		{
			for (int y = 0; y<MAT_SIZE_CUBES; y++)
			{
				//on moyenne sur une distance
				int nb = 0;
				for (int i = (x - sizeWidow < 0 ? 0 : x - sizeWidow);
					i < (x + sizeWidow >= MAT_SIZE_CUBES ? MAT_SIZE_CUBES - 1 : x + sizeWidow); i++)
				{
					for (int j = (y - sizeWidow < 0 ? 0 : y - sizeWidow);
						j <(y + sizeWidow >= MAT_SIZE_CUBES ? MAT_SIZE_CUBES - 1 : y + sizeWidow); j++)
					{
						_MatriceHeightsTmp[x][y] += _MatriceHeights[i][j];
						nb++;
					}
				}
				if (nb)
					_MatriceHeightsTmp[x][y] /= nb;
			}
		}

		//On reset les piles
		for (int x = 0; x<MAT_SIZE_CUBES; x++)
		{
			for (int y = 0; y<MAT_SIZE_CUBES; y++)
			{
				load_pile(x, y, _MatriceHeightsTmp[x][y], false);
			}
		}


	}
	
	void perlinage(NYPerlin &perlin, int x, int y) {
		for(int i=0;i<MAT_SIZE_CUBES;i++)
			for (int j = 0; j < MAT_SIZE_CUBES; j++) {
				float sample = perlin.sample((x*MAT_SIZE_CUBES + i) / (float)PERLIN_DIV, (y*MAT_SIZE_CUBES + j) / (float)PERLIN_DIV, 25);
				//Tweaking du sample pour augmenter la range
				sample = sample*(sample * 2)*(sample * 2)-0.15;
				load_pile(i, j, 1 + (MAT_HEIGHT_CUBES*sample));
			}
	}

	void init_world(int _x, int _y, NYTexFile *bloc, NYTexFile *blocn, NYPerlin &perlin, int profmax = -1)
	{
		_cprintf("Creation du monde %f \n",_FacteurGeneration);

		

		//Reset du monde
		for(int x=0;x<MAT_SIZE;x++)
			for(int y=0;y<MAT_SIZE;y++)
				for(int z=0;z<MAT_HEIGHT;z++)
					_Chunks[x][y][z]->reset();
		memset(_MatriceHeights,0x00,MAT_SIZE_CUBES*MAT_SIZE_CUBES*sizeof(int));

		//On charge les 4 coins
		/*load_pile(0,0,MAT_HEIGHT_CUBES/2);
		load_pile(MAT_SIZE_CUBES-1,0,MAT_HEIGHT_CUBES/2);
		load_pile(MAT_SIZE_CUBES-1,MAT_SIZE_CUBES-1, MAT_HEIGHT_CUBES / 2);
		load_pile(0,MAT_SIZE_CUBES-1,MAT_HEIGHT_CUBES/2);

		//On génère a partir des 4 coins
		generate_piles(0,0,
			MAT_SIZE_CUBES-1,0,
			MAT_SIZE_CUBES-1,MAT_SIZE_CUBES-1,
			0,MAT_SIZE_CUBES-1,1,profmax);	
		//Lissage
		lisse();*/
		perlinage(perlin,_x,_y);

		for(int x=0;x<MAT_SIZE;x++)
			for(int y=0;y<MAT_SIZE;y++)
				for (int z = 0; z < MAT_HEIGHT; z++) {
					_Chunks[x][y][z]->disableHiddenCubes();

				}

		_blocTexture = bloc;
		_blocNormal = blocn;

		add_world_to_vbo();
	}

	NYCube * pick(NYVert3Df  pos, NYVert3Df  dir, NYPoint3D * point)
	{
		return NULL;
	}

	//Boites de collisions plus petites que deux cubes
	NYAxis getMinCol(NYVert3Df pos, NYVert3Df dir, float width, float height, float & valueColMin, bool oneShot)
	{
		int x = (int)(pos.X / NYCube::CUBE_SIZE);
		int y = (int)(pos.Y / NYCube::CUBE_SIZE);
		int z = (int)(pos.Z / NYCube::CUBE_SIZE);

		int xNext = (int)((pos.X + width / 2.0f) / NYCube::CUBE_SIZE);
		int yNext = (int)((pos.Y + width / 2.0f) / NYCube::CUBE_SIZE);
		int zNext = (int)((pos.Z + height / 2.0f) / NYCube::CUBE_SIZE);

		int xPrev = (int)((pos.X - width / 2.0f) / NYCube::CUBE_SIZE);
		int yPrev = (int)((pos.Y - width / 2.0f) / NYCube::CUBE_SIZE);
		int zPrev = (int)((pos.Z - height / 2.0f) / NYCube::CUBE_SIZE);

		if (x < 0)	x = 0;
		if (y < 0)	y = 0;
		if (z < 0)	z = 0;

		if (xPrev < 0)	xPrev = 0;
		if (yPrev < 0)	yPrev = 0;
		if (zPrev < 0)	zPrev = 0;

		if (xNext < 0)	xNext = 0;
		if (yNext < 0)	yNext = 0;
		if (zNext < 0)	zNext = 0;

		if (x >= MAT_SIZE_CUBES)	x = MAT_SIZE_CUBES - 1;
		if (y >= MAT_SIZE_CUBES)	y = MAT_SIZE_CUBES - 1;
		if (z >= MAT_HEIGHT_CUBES)	z = MAT_HEIGHT_CUBES - 1;

		if (xPrev >= MAT_SIZE_CUBES)	xPrev = MAT_SIZE_CUBES - 1;
		if (yPrev >= MAT_SIZE_CUBES)	yPrev = MAT_SIZE_CUBES - 1;
		if (zPrev >= MAT_HEIGHT_CUBES)	zPrev = MAT_HEIGHT_CUBES - 1;

		if (xNext >= MAT_SIZE_CUBES)	xNext = MAT_SIZE_CUBES - 1;
		if (yNext >= MAT_SIZE_CUBES)	yNext = MAT_SIZE_CUBES - 1;
		if (zNext >= MAT_HEIGHT_CUBES)	zNext = MAT_HEIGHT_CUBES - 1;

		//On fait chaque axe
		NYAxis axis = 0x00;
		valueColMin = oneShot ? 0.5 : 10000.0f;
		float seuil = 0.00001;
		float prodScalMin = 1.0f;
		if (dir.getMagnitude() > 1)
			dir.normalize();

		//On verif tout les 4 angles de gauche
		if (getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yPrev, zNext)->isSolid() ||
			getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xPrev + 1, yPrev, zPrev)->isSolid() ||
				getCube(xPrev + 1, yPrev, zNext)->isSolid() ||
				getCube(xPrev + 1, yNext, zPrev)->isSolid() ||
				getCube(xPrev + 1, yNext, zNext)->isSolid()) || !oneShot)
			{
				float depassement = ((xPrev + 1) * NYCube::CUBE_SIZE) - (pos.X - width / 2.0f);
				float prodScal = abs(dir.X);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = NY_AXIS_X;
					}
			}
		}

		//float depassementx2 = (xNext * NYCube::CUBE_SIZE) - (pos.X + width / 2.0f);

		//On verif tout les 4 angles de droite
		if (getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid() ||
			getCube(xNext, yNext, zPrev)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xNext - 1, yPrev, zPrev)->isSolid() ||
				getCube(xNext - 1, yPrev, zNext)->isSolid() ||
				getCube(xNext - 1, yNext, zPrev)->isSolid() ||
				getCube(xNext - 1, yNext, zNext)->isSolid()) || !oneShot)
			{
				float depassement = (xNext * NYCube::CUBE_SIZE) - (pos.X + width / 2.0f);
				float prodScal = abs(dir.X);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = NY_AXIS_X;
					}
			}
		}

		//float depassementy1 = (yNext * NYCube::CUBE_SIZE) - (pos.Y + width / 2.0f);

		//On verif tout les 4 angles de devant
		if (getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid() ||
			getCube(xNext, yNext, zPrev)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yNext - 1, zPrev)->isSolid() ||
				getCube(xPrev, yNext - 1, zNext)->isSolid() ||
				getCube(xNext, yNext - 1, zPrev)->isSolid() ||
				getCube(xNext, yNext - 1, zNext)->isSolid()) || !oneShot)
			{
				float depassement = (yNext * NYCube::CUBE_SIZE) - (pos.Y + width / 2.0f);
				float prodScal = abs(dir.Y);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = NY_AXIS_Y;
					}
			}
		}

		//float depassementy2 = ((yPrev + 1) * NYCube::CUBE_SIZE) - (pos.Y - width / 2.0f);

		//On verif tout les 4 angles de derriere
		if (getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yPrev, zNext)->isSolid() ||
			getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yPrev + 1, zPrev)->isSolid() ||
				getCube(xPrev, yPrev + 1, zNext)->isSolid() ||
				getCube(xNext, yPrev + 1, zPrev)->isSolid() ||
				getCube(xNext, yPrev + 1, zNext)->isSolid()) || !oneShot)
			{
				float depassement = ((yPrev + 1) * NYCube::CUBE_SIZE) - (pos.Y - width / 2.0f);
				float prodScal = abs(dir.Y);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = NY_AXIS_Y;
					}
			}
		}

		//On verif tout les 4 angles du haut
		if (getCube(xPrev, yPrev, zNext)->isSolid() ||
			getCube(xPrev, yNext, zNext)->isSolid() ||
			getCube(xNext, yPrev, zNext)->isSolid() ||
			getCube(xNext, yNext, zNext)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yPrev, zNext - 1)->isSolid() ||
				getCube(xPrev, yNext, zNext - 1)->isSolid() ||
				getCube(xNext, yPrev, zNext - 1)->isSolid() ||
				getCube(xNext, yNext, zNext - 1)->isSolid()) || !oneShot)
			{
				float depassement = (zNext * NYCube::CUBE_SIZE) - (pos.Z + height / 2.0f);
				float prodScal = abs(dir.Z);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = NY_AXIS_Z;
					}
			}
		}

		//On verif tout les 4 angles du bas
		if (getCube(xPrev, yPrev, zPrev)->isSolid() ||
			getCube(xPrev, yNext, zPrev)->isSolid() ||
			getCube(xNext, yPrev, zPrev)->isSolid() ||
			getCube(xNext, yNext, zPrev)->isSolid())
		{
			//On verif que resoudre cette collision est utile
			if (!(getCube(xPrev, yPrev, zPrev + 1)->isSolid() ||
				getCube(xPrev, yNext, zPrev + 1)->isSolid() ||
				getCube(xNext, yPrev, zPrev + 1)->isSolid() ||
				getCube(xNext, yNext, zPrev + 1)->isSolid()) || !oneShot)
			{
				float depassement = ((zPrev + 1) * NYCube::CUBE_SIZE) - (pos.Z - height / 2.0f);
				float prodScal = abs(dir.Z);
				if (abs(depassement) > seuil)
					if (abs(depassement) < abs(valueColMin))
					{
						prodScalMin = prodScal;
						valueColMin = depassement;
						axis = NY_AXIS_Z;
					}
			}
		}



		return axis;
	}


	void render_world_vbo(void)
	{
		for(int x=0;x<MAT_SIZE;x++)
			for(int y=0;y<MAT_SIZE;y++)
				for(int z=0;z<MAT_HEIGHT;z++)
				{
					glPushMatrix();
					glTranslatef((float)(x*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE),(float)(y*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE),(float)(z*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE));
					_Chunks[x][y][z]->render();	
					glPopMatrix();
				}
		//render translucides
		for (int x = 0; x<MAT_SIZE; x++)
			for (int y = 0; y<MAT_SIZE; y++)
				for (int z = 0; z<MAT_HEIGHT; z++)
				{
					glPushMatrix();
					glTranslatef((float)(x*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE), (float)(y*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE), (float)(z*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE));
					_Chunks[x][y][z]->render(false);
					glPopMatrix();
				}
	}

	void render_opaque_vbo(void) {
		

		for (int x = 0; x<MAT_SIZE; x++)
			for (int y = 0; y<MAT_SIZE; y++)
				for (int z = 0; z<MAT_HEIGHT; z++)
				{
					glPushMatrix();
					glTranslatef((float)(x*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE), (float)(y*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE), (float)(z*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE));
					_Chunks[x][y][z]->render();
					glPopMatrix();
				}
		
	}



	void render_water_vbo() {
		//glBindTexture(GL_TEXTURE_2D, 0);

		for (int x = 0; x<MAT_SIZE; x++)
			for (int y = 0; y<MAT_SIZE; y++)
				for (int z = 0; z<MAT_HEIGHT; z++)
				{
					glPushMatrix();
					glTranslatef((float)(x*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE), (float)(y*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE), (float)(z*NYChunk::CHUNK_SIZE*NYCube::CUBE_SIZE));
					_Chunks[x][y][z]->render(false);
					glPopMatrix();
				}

	}

	void add_world_to_vbo(void)
	{
		int totalNbVertices = 0;
		
		for(int x=0;x<MAT_SIZE;x++)
			for(int y=0;y<MAT_SIZE;y++)
				for(int z=0;z<MAT_HEIGHT;z++)
				{
					_Chunks[x][y][z]->toVbo();
					totalNbVertices += _Chunks[x][y][z]->_NbVertices;
				}

		Log::log(Log::ENGINE_INFO,(toString(totalNbVertices) + " vertices in VBO").c_str());
	}

	void render_world_old_school(void)
	{
		for (int x = 0; x < MAT_SIZE; x++) {
			for (int y = 0; y < MAT_SIZE; y++) {
				for (int z = 0; z < MAT_HEIGHT; z++) {
					glPushMatrix();
					glTranslatef(x*NYChunk::CHUNK_SIZE, y*NYChunk::CHUNK_SIZE, z*NYChunk::CHUNK_SIZE);
					_Chunks[x][y][z]->render();
					glPopMatrix();
				}
			}
		}
					

	}	


	bool interDroiteCube(NYVert3Df origine, NYVert3Df direction, int x, int y, int z, NYVert3Df &inter) {
		bool result = false;
		NYVert3Df point_inter;
		NYVert3Df nearest_point;

		int xw = x*NYCube::CUBE_SIZE;
		int yw = y*NYCube::CUBE_SIZE;
		int zw = z*NYCube::CUBE_SIZE;

		NYVert3Df A[6] = { NYVert3Df(xw + NYCube::CUBE_SIZE, yw, zw),
			NYVert3Df(xw, yw + NYCube::CUBE_SIZE, zw),
			NYVert3Df(xw, yw, zw + NYCube::CUBE_SIZE),
			NYVert3Df(xw, yw, zw),
			NYVert3Df(xw, yw, zw),
			NYVert3Df(xw, yw, zw) };
		NYVert3Df B[6] = { NYVert3Df(xw + NYCube::CUBE_SIZE, yw + NYCube::CUBE_SIZE, zw),
			NYVert3Df(xw + NYCube::CUBE_SIZE, yw + NYCube::CUBE_SIZE, zw),
			NYVert3Df(xw + NYCube::CUBE_SIZE, yw, zw + NYCube::CUBE_SIZE),
			NYVert3Df(xw, yw + NYCube::CUBE_SIZE, zw),
			NYVert3Df(xw + NYCube::CUBE_SIZE, yw, zw),
			NYVert3Df(xw + NYCube::CUBE_SIZE, yw, zw) };
		NYVert3Df C[6] = { NYVert3Df(xw + NYCube::CUBE_SIZE, yw + NYCube::CUBE_SIZE, zw + NYCube::CUBE_SIZE),
			NYVert3Df(xw + NYCube::CUBE_SIZE, yw + NYCube::CUBE_SIZE, zw + NYCube::CUBE_SIZE),
			NYVert3Df(xw + NYCube::CUBE_SIZE, yw + NYCube::CUBE_SIZE, zw + NYCube::CUBE_SIZE),
			NYVert3Df(xw, yw + NYCube::CUBE_SIZE, zw + NYCube::CUBE_SIZE),
			NYVert3Df(xw + NYCube::CUBE_SIZE, yw, zw + NYCube::CUBE_SIZE),
			NYVert3Df(xw + NYCube::CUBE_SIZE, yw + NYCube::CUBE_SIZE, zw) };
		NYVert3Df D[6] = { NYVert3Df(xw + NYCube::CUBE_SIZE, yw, zw + NYCube::CUBE_SIZE),
			NYVert3Df(xw, yw, zw + NYCube::CUBE_SIZE),
			NYVert3Df(xw, yw + NYCube::CUBE_SIZE, zw),
			NYVert3Df(xw, yw, zw + NYCube::CUBE_SIZE),
			NYVert3Df(xw, yw, zw + NYCube::CUBE_SIZE),
			NYVert3Df(xw, yw + NYCube::CUBE_SIZE, zw) };

		for (int i = 0; i < 6; i++) {
			if (interDroiteFace(origine, direction, A[i], B[i], C[i], D[i], point_inter)) {
				if (!result) {
					result = true;
					nearest_point = point_inter;
				}
				else if ((nearest_point - origine).getMagnitude() > (point_inter - origine).getMagnitude()){
						nearest_point = point_inter;
				}
			}
		}
		if (result)
			inter = nearest_point;
		return result;
	}

	bool interDroiteMatrice(NYVert3Df origine, NYVert3Df direction, NYVert3Df &inter, NYVert3Df &cube_result) {
		int startx = min(origine.X, (origine + direction).X)/NYCube::CUBE_SIZE;
		int starty = min(origine.Y, (origine + direction).Y) / NYCube::CUBE_SIZE;
		int startz = min(origine.Z, (origine + direction).Z) / NYCube::CUBE_SIZE;
		int endx = max(origine.X, (origine + direction).X) / NYCube::CUBE_SIZE + 1;
		int endy = max(origine.Y, (origine + direction).Y) / NYCube::CUBE_SIZE + 1;
		int endz = max(origine.Z, (origine + direction).Z) / NYCube::CUBE_SIZE + 1;
		NYVert3Df result;
		bool found = false;
		for(int x = startx; x<endx; x++)
			for(int y = starty; y<endy; y++)
				for (int z = startz; z < endz; z++) {
					NYCube *cube = getCube(x,y,z);
					if (cube->_Type == CUBE_AIR || cube->_Type == CUBE_EAU)
						continue;
					if (interDroiteCube(origine, direction, x, y, z, result)) {
						if (!found || (result - origine).getMagnitude() < (inter - origine).getMagnitude()) {
							found = true;
							cube_result = NYVert3Df(x, y, z);
							inter = result;
						}
					}

				}

		return found;
	}
};



#endif