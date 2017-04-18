#ifndef __AVATAR__
#define __AVATAR__

#include "engine/utils/types_3d.h"
#include "engine/render/camera.h"
#include "world.h"
#include "OpenWorld.h"

class NYAvatar
{
	public :
		NYVert3Df Position;
		NYVert3Df Speed;

		NYVert3Df MoveDir;
		bool Move;
		bool Jump;
		float Height;
		float Width;
		bool avance;
		bool recule;
		bool gauche;
		bool droite;
		bool Standing;

		NYCamera * Cam;
		OpenWorld * World;

		NYAvatar(NYCamera * cam,OpenWorld * world)
		{
			Position = NYVert3Df(0,0,0);
			Height = 1.5f;
			Width = 0.3;
			Cam = cam;
			avance = false;
			recule = false;
			gauche = false;
			droite = false;
			Standing = false;
			Jump = false;
			World = world;
		}


		void render(void)
		{
			glutSolidCube(Width/2);
		}

		void update(float elapsed)
		{
			if (elapsed > 1 / 60.)
				elapsed = 1 / 60.;
			//Calcul des forces
			MoveDir = Cam->_Direction;
			MoveDir.Z = 0;
			MoveDir.normalize();
			NYVert3Df F(0, 0, 0);
			if (avance)
				F += MoveDir*500;
			if (recule)
				F += MoveDir*-500;
			if (gauche)
				F += Cam->_NormVec*-500;
			if (droite)
				F += Cam->_NormVec*500;
			if (Jump)
				F += Cam->_UpRef*500;
			else //gravité
				F += Cam->_UpRef*-500;

			//calcul de l'accélération
			Speed = Speed + F*elapsed*2;

			//position
			Position = Position + Speed*elapsed;
			
			float valueColMin;
			for (int i = 0; i < 6; i++) {
				NYAxis axis = World->getMinCol(Position, Speed, Width, Height, valueColMin, i<3);
				if (axis == NY_AXIS_X) {
					Position.X += valueColMin + (valueColMin < 0 ? -0.01 : 0.01);
					Speed.X = 0;
				}
				if (axis == NY_AXIS_Y) {
					Position.Y += valueColMin + (valueColMin < 0.0 ? -0.01 : 0.01);
					Speed.Y = 0;
				}
				if (axis == NY_AXIS_Z) {
					Position.Z += valueColMin + (valueColMin < 0 ? -0.01 : 0.01);
					Speed.Z = 0;
				}
			}

			//Damping
			Speed = Speed * elapsed * 0.31;

			//Cam->moveTo(Position+NYVert3Df(Width/2,Width/2,Height/2));
		}
};

#endif