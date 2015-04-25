/*
 * vincenty_direct.c - Implementation of Thaddeus Vincenty's direct method.
 *
 * Copyright (C) 2015		Andrew Clayton <andrew@digital-domain.net>
 *
 * Licensed under the GNU General Public License V2
 * See COPYING
 */

#include <stdlib.h>
#include <math.h>

#include "vincenty_direct.h"

/*
 * Vincenty Direct formula http://en.wikipedia.org/wiki/Vincenty%27s_formulae
 *
 * This function is based on the implementattion from
 * http://www.gavaghan.org/blog/free-source-code/geodesy-library-vincentys-formula/
 */
void vincenty_direct(struct geo_info *gi, double *lat, double *lon)
{
	double a_squared = A * A;
	double b_squared = B * B;
	double phi1 = gi->lat * DEG_TO_RAD;
	double alpha1 = gi->bearing * DEG_TO_RAD;
	double cos_alpha1 = cos(alpha1);
	double sin_alpha1 = sin(alpha1);
	double s = gi->radius;
	double tan_u1 = (1.0 - F) * tan(phi1);
	double cos_u1 = 1.0 / sqrt(1.0 + tan_u1 * tan_u1);
	double sin_u1 = tan_u1 * cos_u1;

	/* eq. 1 */
	double sigma1 = atan2(tan_u1, cos_alpha1);

	/* eq. 2 */
	double sin_alpha = cos_u1 * sin_alpha1;

	double sin2_alpha = sin_alpha * sin_alpha;
	double cos2_alpha = 1 - sin_alpha;
	double u_squared = cos2_alpha * (a_squared - b_squared) / b_squared;

	/* eq. 3 */
	double a = 1 + (u_squared / 16384) * (4096 + u_squared *
			(-768 + u_squared * (320 - 175 * u_squared)));

	/* eq. 4 */
	double b = (u_squared / 1024) * (256 + u_squared *
			(-128 + u_squared * (74 - 47 * u_squared)));

	/* iterate until there is a negligible change in sigma */
	double s_over_ba = s / (B * a);
	double sigma = s_over_ba;
	double sin_sigma;
	double prev_sigma = s_over_ba;
	double sigma_m2;
	double cos_sigma_m2;
	double cos2_sigma_m2;

	for (;;) {
		/* eq. 5 */
		double delta_sigma;

		sigma_m2 = 2.0 * sigma1 + sigma;
		cos_sigma_m2 = cos(sigma_m2);
		cos2_sigma_m2 = cos_sigma_m2 * cos_sigma_m2;
		sin_sigma = sin(sigma);
		double cos_signma = cos(sigma);

		/* eq. 6 */
		delta_sigma = b * sin_sigma * (cos_sigma_m2 + (b / 4.0) *
				(cos_signma * (-1 + 2 * cos2_sigma_m2) -
				 (b / 6.0) * cos_sigma_m2 *
				 (-3 + 4 * sin_sigma * sin_sigma) *
				 (-3 + 4 * cos2_sigma_m2)));

		/* eq. 7 */
		sigma = s_over_ba + delta_sigma;

		/* break after converging to tolerance */
		if (abs(sigma - prev_sigma) < 0.0000000000001)
			break;

		prev_sigma = sigma;
	}

	sigma_m2 = 2.0 * sigma1 + sigma;
	cos_sigma_m2 = cos(sigma_m2);
	cos2_sigma_m2 = cos_sigma_m2 * cos_sigma_m2;

	double cos_sigma = cos(sigma);
	sin_sigma = sin(sigma);

	/* eq. 8 */
	double phi2 = atan2(sin_u1 * cos_sigma + cos_u1 * sin_sigma *
			cos_alpha1, (1.0 - F) *
			sqrt(sin2_alpha + pow(sin_u1 * sin_sigma - cos_u1 *
					      cos_sigma * cos_alpha1, 2.0)));

	/*
	 * eq. 9
	 * This fixes the pole crossing defect spotted by Matt Feemster.
	 * When a path passes a pole and essentially crosses a line of
	 * latitude twice - once in each direction - the longitude
	 * calculation got messed up. Using Atan2 instead of Atan fixes
	 * the defect. The change is in the next 3 lines.
	 * double tanLambda = sinSigma * sinAlpha1 /
	 * (cosU1 * cosSigma - sinU1*sinSigma*cosAlpha1);
	 * double lambda = atan(tanLambda);
	 */
	double lambda = atan2(sin_sigma * sin_alpha1, cos_u1 *
			cos_sigma - sin_u1 * sin_sigma * cos_alpha1);

	/* eq. 10 */
	double c = (F / 16) * cos2_alpha * (4 + F * (4 - 3 * cos2_alpha));

	/* eq. 11 */
	double l = lambda - (1 - c) * F * sin_alpha *
		(sigma + c * sin_sigma *
		 (cos_sigma_m2 + c *
		  cos_sigma * (-1 + 2 * cos2_sigma_m2)));

#if 0
	/* eq. 12 - final bearing */
	double alpha2 = atan2(sin_alpha, -sin_u1 * sin_sigma + cos_u1 *
			cos_sigma * cos_alpha1);
#endif

	*lat = phi2 / DEG_TO_RAD;
	*lon = gi->lon + (l / DEG_TO_RAD);
}
