#pragma once

#include "engine/utils/types_3d.h"


bool interDroitePlan(NYVert3Df origine, NYVert3Df direction, NYVert3Df normal_plan, NYVert3Df point_plan, NYVert3Df &point_inter) {
	float d = -(normal_plan.X*point_plan.X+normal_plan.Y*point_plan.Y+normal_plan.Z*point_plan.Z);
	float dividende = (normal_plan.X*(direction.X) + normal_plan.Y*(direction.Y) + normal_plan.Z*(direction.Z));
	if (dividende == 0) //la droite est parallèle au plan
		return false;
	float t = (-d - normal_plan.X*origine.X - normal_plan.Y*origine.Y - normal_plan.Z*origine.Z)
		/ dividende;

	point_inter = NYVert3Df(origine.X + direction.X*t, origine.Y + direction.Y*t, origine.Z + direction.Z*t);

	if (t>0.0 && t < 1.0)
		return true;
	else
		return false;
}

bool interDroiteFace(NYVert3Df origine, NYVert3Df direction, NYVert3Df A, NYVert3Df B, NYVert3Df C, NYVert3Df D, NYVert3Df &point_inter) {
	NYVert3Df normal = (B - A).vecProd(C - A);
	if (interDroitePlan(origine, direction, normal, A, point_inter)) {
		NYVert3Df BAP, CBP, DCP, ADP;
		BAP = (B - A).vecProd(point_inter - A);
		CBP = (C - B).vecProd(point_inter - B);
		DCP = (D - C).vecProd(point_inter - C);
		ADP = (A - D).vecProd(point_inter - D);
		if (BAP.scalProd(CBP) > 0 && CBP.scalProd(DCP) > 0 && DCP.scalProd(ADP) > 0 && ADP.scalProd(BAP) > 0)
			return true;
		else
			return false;
	}
	else
		return false;
}