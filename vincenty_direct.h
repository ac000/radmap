/*
 * vincenty_direct.h - Implementation of Thaddeus Vincenty's direct method.
 *
 * Copyright (C) 2015		Andrew Clayton <andrew@digital-domain.net>
 *
 * Licensed under the GNU General Public License V2
 * See COPYING
 */

#ifndef _VINCENTY_DIRECT_H_
#define _VINCENTY_DIRECT_H_

#define PI		3.1415926535897932384
/* PI/180 constant */
#define DEG_TO_RAD	0.017453292519943295769236907684886
/* Earth radius at equator (m) */
#define A		6378137.0
/* Earth radius at the poles (m) */
#define B		6356752.314
/* FWGS-84 ellipsiod */
#define F		(1 / 298.257)

/*
 * Structure to hold source location informatio for use in vincenty_direct()
 */
struct geo_info {
        double lat;
        double lon;
        double bearing;
        unsigned int radius;
};

void vincenty_direct(struct geo_info *gi, double *lat, double *lon);

#endif /* _VINCENTY_DIRECT_H_ */
