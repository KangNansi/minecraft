#pragma once

#include "engine/render/renderer.h"
#include "cube.h"

/**
  * On utilise des chunks pour que si on modifie juste un cube, on ait pas
  * besoin de recharger toute la carte dans le buffer, mais juste le chunk en question
  */
class NYChunk
{
	public :

		static const int CHUNK_SIZE = 16; ///< Taille d'un chunk en nombre de cubes (n*n*n)
		NYCube _Cubes[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]; ///< Cubes contenus dans le chunk

		static const int TEX_SIZE = 128;
		static const int SPRITE_SIZE = 16;

		GLuint _BufWorld; ///< Identifiant du VBO pour le monde
		GLuint _BufWorldT; ///< VBO des faces translucides
		
		static float _WorldVert[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*3*4*6]; ///< Buffer pour les sommets
		static float _WorldCols[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*4*4*6]; ///< Buffer pour les couleurs
		static float _WorldNorm[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*3*4*6]; ///< Buffer pour les normales
		static float _WorldUV[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * 2 * 4 * 6]; ///< Buffer pour les uvs

		static float _WorldVertT[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * 3 * 4 * 6];
		static float _WorldColsT[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * 4 * 4 * 6];
		static float _WorldNormT[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * 3 * 4 * 6];
		static float _WorldUVT[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * 2 * 4 * 6];

		static const int SIZE_VERTICE = 3 * sizeof(float); ///< Taille en octets d'un vertex dans le VBO
		static const int SIZE_COLOR = 4 * sizeof(float);  ///< Taille d'une couleur dans le VBO
		static const int SIZE_NORMAL = 3 * sizeof(float);  ///< Taille d'une normale dans le VBO
		static const int SIZE_UV = 2 * sizeof(float);
		
		int _NbVertices; ///< Nombre de vertices dans le VBO (on ne met que les faces visibles)
		int _NbVerticesT;

		NYChunk * Voisins[6];
		
		NYChunk()
		{
			_NbVertices = 0;
			_BufWorld = 0;
			_NbVerticesT = 0;
			_BufWorldT = 0;
			memset(Voisins,0x00,sizeof(void*) * 6);
		}

		void setVoisins(NYChunk * xprev, NYChunk * xnext,NYChunk * yprev,NYChunk * ynext,NYChunk * zprev,NYChunk * znext)
		{
			Voisins[0] = xprev;
			Voisins[1] = xnext;
			Voisins[2] = yprev;
			Voisins[3] = ynext;
			Voisins[4] = zprev;
			Voisins[5] = znext;
		}

		/**
		  * Raz de l'état des cubes (a draw = false)
		  */
		void reset(void)
		{
			for(int x=0;x<CHUNK_SIZE;x++)
				for(int y=0;y<CHUNK_SIZE;y++)
					for(int z=0;z<CHUNK_SIZE;z++)
					{
						_Cubes[x][y][z]._Draw = true;
						_Cubes[x][y][z]._Type = CUBE_AIR;
					}
		}

		//On met le chunk ddans son VBO
		void toVbo(void)
		{
			//Remplissage des buffers de la classe
			_NbVertices = 0;
			_NbVerticesT = 0;

			for(int x=0; x<CHUNK_SIZE; x++)
				for(int y=0; y<CHUNK_SIZE; y++)
					for (int z = 0; z < CHUNK_SIZE; z++) {
						if (_Cubes[x][y][z]._Draw) {
							switch (_Cubes[x][y][z]._Type) {
							case CUBE_EAU:
								vboAddCube(x, y, z, NYColor(0.1, 0.3, 0.6, 0.6), _Cubes[x][y][z]._Type, false);
								break;
							case CUBE_TERRE:
								vboAddCube(x, y, z, NYColor(0.4+randf()/4.0, 0.2, 0.05, 1), _Cubes[x][y][z]._Type);
								break;
							case CUBE_HERBE:
								vboAddCube(x, y, z, NYColor(0.2, 1 + randf() / 8.0, 0.2, 1), _Cubes[x][y][z]._Type);
								break;
							default:
								break;
							}
						}		
					}



			//On le detruit si il existe deja
			if (_BufWorld != 0)
				glDeleteBuffers(1, &_BufWorld);
			if (_BufWorldT != 0)
				glDeleteBuffers(1, &_BufWorldT);

			//Generation VBO opaque
			//Genere un identifiant
			glGenBuffers(1, &_BufWorld);

			//On attache le VBO pour pouvoir le modifier
			glBindBuffer(GL_ARRAY_BUFFER, _BufWorld);

			//On reserve la quantite totale de datas (creation de la zone memoire, mais sans passer les données)
			//Les tailles g_size* sont en octets, à vous de les calculer
			glBufferData(GL_ARRAY_BUFFER,
				_NbVertices * SIZE_VERTICE +
				_NbVertices * SIZE_COLOR +
				_NbVertices * SIZE_NORMAL +
				_NbVertices * SIZE_UV,
				NULL,
				GL_STREAM_DRAW);

			//Check error (la tester ensuite...)
			GLenum error = glGetError();

			//On copie les vertices
			glBufferSubData(GL_ARRAY_BUFFER,
				0, //Offset 0, on part du debut                        
				_NbVertices * SIZE_VERTICE, //Taille en octets des datas copiés
				_WorldVert);  //Datas          

							  //Check error (la tester ensuite...)
			error = glGetError();
			
			//On copie les couleurs
			glBufferSubData(GL_ARRAY_BUFFER,
				_NbVertices * SIZE_VERTICE, //Offset : on se place après les vertices
				_NbVertices * SIZE_COLOR, //On copie tout le buffer couleur : on donne donc sa taille
				_WorldCols);  //Pt sur le buffer couleur       

							  //Check error (la tester ensuite...)
			error = glGetError();

			//On copie les normales (a vous de déduire les params)
			glBufferSubData(GL_ARRAY_BUFFER,
				_NbVertices * (SIZE_VERTICE+SIZE_COLOR),
				_NbVertices * SIZE_NORMAL,
				_WorldNorm);

			error = glGetError();

			//On copie les uvs
			glBufferSubData(GL_ARRAY_BUFFER,
				_NbVertices * (SIZE_VERTICE + SIZE_COLOR + SIZE_NORMAL),
				_NbVertices * SIZE_UV,
				_WorldUV);

			while ((error = glGetError()) != GL_NO_ERROR) {
				cerr << "OpenGL error: " << error << endl;
			}

			//Generation buffer translucide

			//Genere un identifiant
			glGenBuffers(1, &_BufWorldT);

			//On attache le VBO pour pouvoir le modifier
			glBindBuffer(GL_ARRAY_BUFFER, _BufWorldT);

			//On reserve la quantite totale de datas (creation de la zone memoire, mais sans passer les données)
			//Les tailles g_size* sont en octets, à vous de les calculer
			glBufferData(GL_ARRAY_BUFFER,
				_NbVerticesT * SIZE_VERTICE +
				_NbVerticesT * SIZE_COLOR +
				_NbVerticesT * SIZE_NORMAL +
				_NbVerticesT * SIZE_UV,
				NULL,
				GL_STREAM_DRAW);

			//Check error (la tester ensuite...)
			error = glGetError();

			//On copie les vertices
			glBufferSubData(GL_ARRAY_BUFFER,
				0, //Offset 0, on part du debut                        
				_NbVerticesT * SIZE_VERTICE, //Taille en octets des datas copiés
				_WorldVertT);  //Datas          

							  //Check error (la tester ensuite...)
			error = glGetError();

			//On copie les couleurs
			glBufferSubData(GL_ARRAY_BUFFER,
				_NbVerticesT * SIZE_VERTICE, //Offset : on se place après les vertices
				_NbVerticesT * SIZE_COLOR, //On copie tout le buffer couleur : on donne donc sa taille
				_WorldColsT);  //Pt sur le buffer couleur       

							  //Check error (la tester ensuite...)
			error = glGetError();

			//On copie les normales (a vous de déduire les params)
			glBufferSubData(GL_ARRAY_BUFFER,
				_NbVerticesT * (SIZE_VERTICE + SIZE_COLOR),
				_NbVerticesT * SIZE_NORMAL,
				_WorldNormT);

			error = glGetError();

			//On copie les normales (a vous de déduire les params)
			glBufferSubData(GL_ARRAY_BUFFER,
				_NbVerticesT * (SIZE_VERTICE + SIZE_COLOR + SIZE_NORMAL),
				_NbVerticesT * SIZE_UV,
				_WorldUVT);

			while ((error = glGetError()) != GL_NO_ERROR) {
				cerr << "OpenGL error: " << error << endl;
			}

			//On debind le buffer pour eviter une modif accidentelle par le reste du code
			glBindBuffer(GL_ARRAY_BUFFER, 0);

		}

		void vboAddCube(int x, int y, int z, NYColor color, NYCubeType type, bool opaque = true) {

			NYCube * cubeXPrev = NULL;
			NYCube * cubeXNext = NULL;
			NYCube * cubeYPrev = NULL;
			NYCube * cubeYNext = NULL;
			NYCube * cubeZPrev = NULL;
			NYCube * cubeZNext = NULL;

			if (x == 0 && Voisins[0] != NULL)
				cubeXPrev = &(Voisins[0]->_Cubes[CHUNK_SIZE - 1][y][z]);
			else if (x > 0)
				cubeXPrev = &(_Cubes[x - 1][y][z]);

			if (x == CHUNK_SIZE - 1 && Voisins[1] != NULL)
				cubeXNext = &(Voisins[1]->_Cubes[0][y][z]);
			else if (x < CHUNK_SIZE - 1)
				cubeXNext = &(_Cubes[x + 1][y][z]);

			if (y == 0 && Voisins[2] != NULL)
				cubeYPrev = &(Voisins[2]->_Cubes[x][CHUNK_SIZE - 1][z]);
			else if (y > 0)
				cubeYPrev = &(_Cubes[x][y - 1][z]);

			if (y == CHUNK_SIZE - 1 && Voisins[3] != NULL)
				cubeYNext = &(Voisins[3]->_Cubes[x][0][z]);
			else if (y < CHUNK_SIZE - 1)
				cubeYNext = &(_Cubes[x][y + 1][z]);

			if (z == 0 && Voisins[4] != NULL)
				cubeZPrev = &(Voisins[4]->_Cubes[x][y][CHUNK_SIZE - 1]);
			else if (z > 0)
				cubeZPrev = &(_Cubes[x][y][z - 1]);

			if (z == CHUNK_SIZE - 1 && Voisins[5] != NULL)
				cubeZNext = &(Voisins[5]->_Cubes[x][y][0]);
			else if (z < CHUNK_SIZE - 1)
				cubeZNext = &(_Cubes[x][y][z + 1]);
			
			//Face X pos
			if (_Cubes[x][y][z]._Type!=CUBE_EAU &&(cubeXNext==nullptr
				|| (_Cubes[x][y][z]._Type != cubeXNext->_Type && !cubeXNext->isSolid()))) {
				addVertice(NYVert3Df(x+1, y, z), color, NYVert3Df(1, 0, 0), getTexUV(type, 1, 2), opaque);
				addVertice(NYVert3Df(x+1, y+1, z), color, NYVert3Df(1, 0, 0), getTexUV(type, 1, 3), opaque);
				addVertice(NYVert3Df(x+1, y+1, z+1), color, NYVert3Df(1, 0, 0), getTexUV(type, 1, 0), opaque);
				addVertice(NYVert3Df(x+1, y, z+1), color, NYVert3Df(1, 0, 0), getTexUV(type, 1, 1), opaque);
			}
			//Face Y pos
			if (_Cubes[x][y][z]._Type != CUBE_EAU && (cubeYNext == nullptr
				|| (_Cubes[x][y][z]._Type != cubeYNext->_Type && !cubeYNext->isSolid()))) {
				addVertice(NYVert3Df(x, y + 1, z), color, NYVert3Df(0, 1, 0), getTexUV(type, 1, 3), opaque);
				addVertice(NYVert3Df(x, y + 1, z + 1), color, NYVert3Df(0, 1, 0), getTexUV(type, 1, 0), opaque);
				addVertice(NYVert3Df(x + 1, y + 1, z + 1), color, NYVert3Df(0, 1, 0), getTexUV(type, 1, 1), opaque);
				addVertice(NYVert3Df(x + 1, y + 1, z), color, NYVert3Df(0, 1, 0), getTexUV(type, 1, 2), opaque);
			}
			//Face Z pos
			if (cubeZNext == nullptr
				|| (_Cubes[x][y][z]._Type != cubeZNext->_Type && !cubeZNext->isSolid())) {
				addVertice(NYVert3Df(x, y, z + 1), color, NYVert3Df(0, 0, 1), getTexUV(type, 0, 0), opaque);
				addVertice(NYVert3Df(x + 1, y, z + 1), color, NYVert3Df(0, 0, 1), getTexUV(type, 0, 1), opaque);
				addVertice(NYVert3Df(x + 1, y + 1, z + 1), color, NYVert3Df(0, 0, 1), getTexUV(type, 0, 2), opaque);
				addVertice(NYVert3Df(x, y + 1, z + 1), color, NYVert3Df(0, 0, 1), getTexUV(type, 0, 3), opaque);
			}
			//Face X neg
			if (_Cubes[x][y][z]._Type != CUBE_EAU && (cubeXPrev == nullptr
				|| (_Cubes[x][y][z]._Type != cubeXPrev->_Type && !cubeXPrev->isSolid()))) {
				addVertice(NYVert3Df(x, y, z), color, NYVert3Df(-1, 0, 0), getTexUV(type, 1, 3), opaque);
				addVertice(NYVert3Df(x, y, z + 1), color, NYVert3Df(-1, 0, 0), getTexUV(type, 1, 0), opaque);
				addVertice(NYVert3Df(x, y + 1, z + 1), color, NYVert3Df(-1, 0, 0), getTexUV(type, 1, 1), opaque);
				addVertice(NYVert3Df(x, y + 1, z), color, NYVert3Df(-1, 0, 0), getTexUV(type, 1, 2), opaque);
			}
			//Face Y neg
			if (_Cubes[x][y][z]._Type != CUBE_EAU && (cubeYPrev == nullptr
				|| (_Cubes[x][y][z]._Type != cubeYPrev->_Type && !cubeYPrev->isSolid()))) {
				addVertice(NYVert3Df(x, y, z), color, NYVert3Df(0, -1, 0), getTexUV(type, 1, 2), opaque);
				addVertice(NYVert3Df(x + 1, y, z), color, NYVert3Df(0, -1, 0), getTexUV(type, 1, 3), opaque);
				addVertice(NYVert3Df(x + 1, y, z + 1), color, NYVert3Df(0, -1, 0), getTexUV(type, 1, 0), opaque);
				addVertice(NYVert3Df(x, y, z + 1), color, NYVert3Df(0, -1, 0), getTexUV(type, 1, 1), opaque);
			}
			//Face Z neg
			if (cubeZPrev == nullptr
				|| (_Cubes[x][y][z]._Type != cubeZPrev->_Type && !cubeZPrev->isSolid())) {
				addVertice(NYVert3Df(x, y, z), color, NYVert3Df(0, 0, -1), getTexUV(type, 2, 0), opaque);
				addVertice(NYVert3Df(x, y + 1, z), color, NYVert3Df(0, 0, -1), getTexUV(type, 2, 1), opaque);
				addVertice(NYVert3Df(x + 1, y + 1, z), color, NYVert3Df(0, 0, -1), getTexUV(type, 2, 2), opaque);
				addVertice(NYVert3Df(x + 1, y, z), color, NYVert3Df(0, 0, -1), getTexUV(type, 2, 3), opaque);
			}
		}

		void addVertice(NYVert3Df v, NYColor c, NYVert3Df n, NYVert2Df uv, bool opaque = true) {
			int position = (opaque?_NbVertices*3:_NbVerticesT*3);
			float *vert, *cols, *norm, *uvs;
			vert = opaque ? _WorldVert : _WorldVertT;
			cols = opaque ? _WorldCols : _WorldColsT;
			norm = opaque ? _WorldNorm : _WorldNormT;
			uvs = opaque ? _WorldUV : _WorldUVT;
				//add vertice
				vert[position] = v.X;
				vert[position + 1] = v.Y;
				vert[position + 2] = v.Z;
				//add color
				position = opaque ? _NbVertices*4 : _NbVerticesT*4;
				cols[position] = c.R;
				cols[position+1] = c.V;
				cols[position+2] = c.B;
				cols[position+3] = c.A;
				//add normal
				position = opaque ? _NbVertices*3 : _NbVerticesT*3;
				norm[position] = n.X;
				norm[position+1] = n.Y;
				norm[position+2] = n.Z;
				
				//add UVs
				position = opaque ? _NbVertices * 2 : _NbVerticesT*2;
				uvs[position] = uv.X;
				uvs[position + 1] = uv.Y;

				if (opaque) _NbVertices++;
				else _NbVerticesT++;
		}

		NYVert2Df getTexUV(NYCubeType type, int face, int vert) {
			float bloc_size = SPRITE_SIZE / (float)TEX_SIZE;
			switch (type) {
			case CUBE_AIR:
				return NYVert2Df(0, 0);
			case CUBE_EAU:
				return NYVert2Df(0, 0);
			case CUBE_HERBE:
				if (face == 0) //Haut du cube
					if (vert == 0)
						return NYVert2Df(0, 0);//haut-gauche
					else if (vert == 1)
						return NYVert2Df(bloc_size, 0);//haut-droite
					else if (vert == 2)
						return NYVert2Df(bloc_size, bloc_size);//bas-droite
					else
						return NYVert2Df(0, bloc_size);//bas-gauche;
				else if(face == 1) //Coté du cube
					if (vert == 0)
						return NYVert2Df(bloc_size, 0);//haut-gauche
					else if (vert == 1)
						return NYVert2Df(bloc_size*2, 0);//haut-droite
					else if (vert == 2)
						return NYVert2Df(bloc_size*2, bloc_size);//bas-droite
					else
						return NYVert2Df(bloc_size, bloc_size);//bas-gauche;
				else //Bas du cube
					if (vert == 0)
						return NYVert2Df(bloc_size*2, 0);//haut-gauche
					else if (vert == 1)
						return NYVert2Df(bloc_size*3, 0);//haut-droite
					else if (vert == 2)
						return NYVert2Df(bloc_size*3, bloc_size);//bas-droite
					else
						return NYVert2Df(bloc_size*2, bloc_size);//bas-gauche;
			case CUBE_TERRE:
					//Meme texture sur chaque face
					if (vert == 0)
						return NYVert2Df(0, bloc_size);//haut-gauche
					else if (vert == 1)
						return NYVert2Df(bloc_size, bloc_size);//haut-droite
					else if (vert == 2)
						return NYVert2Df(bloc_size, bloc_size*2);//bas-droite
					else
						return NYVert2Df(0, bloc_size*2);//bas-gauche;
			default:
				return NYVert2Df(0, 0);	
			}
		}

		void render(bool opaque = true)
		{
			glEnable(GL_COLOR_MATERIAL);
			glEnable(GL_LIGHTING);

			GLuint buffer = opaque ? _BufWorld : _BufWorldT;
			int nb_vertices = opaque ? _NbVertices : _NbVerticesT;

			//On bind le buuffer
			glBindBuffer(GL_ARRAY_BUFFER, buffer);
			NYRenderer::checkGlError("glBindBuffer");

			//On active les datas que contiennent le VBO
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);
			glEnableClientState(GL_NORMAL_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

			//On place les pointeurs sur les datas, aux bons offsets
			glVertexPointer(3, GL_FLOAT, 0, (void*)(0));
			glColorPointer(4, GL_FLOAT, 0, (void*)(nb_vertices*SIZE_VERTICE));
			glNormalPointer(GL_FLOAT, 0, (void*)(nb_vertices*SIZE_VERTICE + nb_vertices*SIZE_COLOR));
			glTexCoordPointer(2, GL_FLOAT, 0, (void*)(nb_vertices*SIZE_VERTICE + nb_vertices*SIZE_COLOR + nb_vertices*SIZE_NORMAL));
			

			//On demande le dessin
			glDrawArrays(GL_QUADS, 0, nb_vertices);

			//On cleane
			glDisableClientState(GL_COLOR_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);
		}

		/**
		  * On verifie si le cube peut être vu
		  */
		bool test_hidden(int x, int y, int z)
		{
			NYCube * cubeXPrev = NULL; 
			NYCube * cubeXNext = NULL; 
			NYCube * cubeYPrev = NULL; 
			NYCube * cubeYNext = NULL; 
			NYCube * cubeZPrev = NULL; 
			NYCube * cubeZNext = NULL; 

			if(x == 0 && Voisins[0] != NULL)
				cubeXPrev = &(Voisins[0]->_Cubes[CHUNK_SIZE-1][y][z]);
			else if(x > 0)
				cubeXPrev = &(_Cubes[x-1][y][z]);

			if(x == CHUNK_SIZE-1 && Voisins[1] != NULL)
				cubeXNext = &(Voisins[1]->_Cubes[0][y][z]);
			else if(x < CHUNK_SIZE-1)
				cubeXNext = &(_Cubes[x+1][y][z]);

			if(y == 0 && Voisins[2] != NULL)
				cubeYPrev = &(Voisins[2]->_Cubes[x][CHUNK_SIZE-1][z]);
			else if(y > 0)
				cubeYPrev = &(_Cubes[x][y-1][z]);

			if(y == CHUNK_SIZE-1 && Voisins[3] != NULL)
				cubeYNext = &(Voisins[3]->_Cubes[x][0][z]);
			else if(y < CHUNK_SIZE-1)
				cubeYNext = &(_Cubes[x][y+1][z]);

			if(z == 0 && Voisins[4] != NULL)
				cubeZPrev = &(Voisins[4]->_Cubes[x][y][CHUNK_SIZE-1]);
			else if(z > 0)
				cubeZPrev = &(_Cubes[x][y][z-1]);

			if(z == CHUNK_SIZE-1 && Voisins[5] != NULL)
				cubeZNext = &(Voisins[5]->_Cubes[x][y][0]);
			else if(z < CHUNK_SIZE-1)
				cubeZNext = &(_Cubes[x][y][z+1]);

			if( cubeXPrev == NULL || cubeXNext == NULL ||
				cubeYPrev == NULL || cubeYNext == NULL ||
				cubeZPrev == NULL || cubeZNext == NULL )
				return false;

			if( cubeXPrev->isSolid() == true && //droite
				cubeXNext->isSolid() == true && //gauche
				cubeYPrev->isSolid() == true && //haut
				cubeYNext->isSolid() == true && //bas
				cubeZPrev->isSolid() == true && //devant
				cubeZNext->isSolid() == true )  //derriere
				return true;
			return false;
		}

		void disableHiddenCubes(void)
		{
			for(int x=0;x<CHUNK_SIZE;x++)
				for(int y=0;y<CHUNK_SIZE;y++)
					for(int z=0;z<CHUNK_SIZE;z++)
					{
						_Cubes[x][y][z]._Draw = true;
						if(test_hidden(x,y,z))
							_Cubes[x][y][z]._Draw = false;
					}
		}


};