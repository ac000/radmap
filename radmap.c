/*
 * radmap.c - Simple mapping application using libchamplain
 *
 * Copyright (C) 2015	Andrew Clayton <andrew@digital-domain.net>
 *
 * Licensed under the GNU General Public License V2
 * See COPYING
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <clutter/clutter.h>
#include <champlain/champlain.h>

#include "vincenty_direct.h"

static ClutterActor *coord_label;

static void goto_entity(ChamplainView *map, ClutterActor *actor)
{
	double lat;
	double lon;

	lat = champlain_location_get_latitude(CHAMPLAIN_LOCATION(actor));
	lon = champlain_location_get_longitude(CHAMPLAIN_LOCATION(actor));
	g_object_set(G_OBJECT(map), "zoom-level", 12, NULL);
	champlain_view_center_on(CHAMPLAIN_VIEW(map), lat, lon);
}

static void update_coord_label(ClutterActor *map, double lat, double lon)
{
	char label_text[40];

	snprintf(label_text, sizeof(label_text), "%f, %f", lat, lon);
	clutter_text_set_text(CLUTTER_TEXT(coord_label), label_text);
}

static gboolean map_click(ClutterActor *actor, ClutterButtonEvent *event,
			  ChamplainView *view)
{
	double lon;
	double lat;
	gfloat x;
	gfloat y;
	const char *entity;

	entity = clutter_actor_get_name(actor);

	clutter_event_get_coords((ClutterEvent *)event, &x, &y);
	lon = champlain_view_x_to_longitude(view, x);
	lat = champlain_view_y_to_latitude(view, y);

	printf("%s was clicked @ %f, %f \n", entity, lat, lon);

	if (strcmp(entity, "Map") == 0)
		update_coord_label(actor, lat, lon);
	else
		goto_entity(view, actor);

	return TRUE;
}

static void add_a_polygon(ChamplainView *map, struct geo_info *gi,
			  ClutterColor *poly_color)
{
	int i;
	ChamplainPathLayer *polygon = champlain_path_layer_new();

	for (i = 0; i <= 360; i += 10) {
		double lat;
		double lon;
		ChamplainCoordinate *coord;

		gi->bearing = i;
		vincenty_direct(gi, &lat, &lon);
		printf("lat = %f, lon = %f\n", lat, lon);
		coord = champlain_coordinate_new_full(lat, lon);
		champlain_path_layer_add_node(polygon,
				CHAMPLAIN_LOCATION(coord));
	}

	champlain_path_layer_set_stroke_color(polygon, poly_color);
	champlain_path_layer_set_fill_color(polygon, poly_color);
	champlain_path_layer_set_stroke_width(polygon, 0.0);
	champlain_path_layer_set_fill(polygon, TRUE);
	champlain_view_add_layer(map, CHAMPLAIN_LAYER(polygon));
}

static ChamplainMarkerLayer *create_marker_layer(ChamplainView *view)
{
	ChamplainMarkerLayer *mlayer;
	char string[512];
	FILE *fp;

	mlayer = champlain_marker_layer_new();

	fp = fopen("markers", "r");
	if (!fp) {
		printf("Can't open markers file: (markers)\n");
		return mlayer;
	}

	printf("Found markers for :-\n");
	while (fgets(string, 512, fp)) {
		ClutterActor *marker;
		ClutterColor *poly_color;
		char entity[80];
		double lat;
		double lon;
		unsigned int radius;
		unsigned int red;
		unsigned int green;
		unsigned int blue;
		unsigned int alpha;
		struct geo_info gi;

		sscanf(string, "%79[^|]|%lf|%lf|%d|%d|%d|%d|%d[^\n]",
				entity, &lat, &lon, &radius,
				&red, &green, &blue, &alpha);
		if (entity[0] == '#')
			continue;

		if (entity[0] == '\n')
			break;

		printf("%s, lat %f, lon %f, radius %d\n",
				entity, lat, lon, radius);
		marker = champlain_label_new_with_text(entity, "Sans 10",
				NULL, NULL);
		champlain_location_set_location(CHAMPLAIN_LOCATION(marker),
				lat, lon);
		clutter_actor_set_reactive(marker, TRUE);
		clutter_actor_set_name(marker, entity);
		g_signal_connect(marker, "button-release-event",
				G_CALLBACK(map_click), view);
		champlain_marker_layer_add_marker(mlayer,
			CHAMPLAIN_MARKER(marker));

		if (radius == 0)
			continue;

		poly_color = clutter_color_new(red, green, blue, alpha);
		gi.lat = lat;
		gi.lon = lon;
		gi.radius = radius;
		add_a_polygon(view, &gi, poly_color);
		clutter_color_free(poly_color);
	}
	fclose(fp);

	return mlayer;
}

int main(int argc, char *argv[])
{
	ClutterActor *stage;
	ClutterActor *map;
	ChamplainMarkerLayer *markers;
	int err;

	err = clutter_init(&argc, &argv);
	if (err != CLUTTER_INIT_SUCCESS)
		exit(EXIT_FAILURE);

	stage = clutter_stage_new();
	clutter_actor_set_size(stage, 1050, 750);

	/* Create the map view */
	map = champlain_view_new();
	clutter_actor_set_size(map, 1050, 750);
	clutter_actor_add_child(stage, map);
	clutter_actor_set_name(map, "Map");
	clutter_actor_set_reactive(map, TRUE);
	champlain_view_set_kinetic_mode(CHAMPLAIN_VIEW(map), TRUE);
	g_signal_connect(map, "button-press-event", G_CALLBACK(map_click),
			map);
	g_signal_connect(map, "destroy", G_CALLBACK(clutter_main_quit), NULL);

	/* Create the markers and marker layer */
	markers = create_marker_layer(CHAMPLAIN_VIEW(map));
	champlain_view_add_layer(CHAMPLAIN_VIEW(map),
			CHAMPLAIN_LAYER(markers));

	/* Setup the map co-ordinates label */
	coord_label = clutter_text_new_with_text("Sans 10", "");
	clutter_actor_add_child(map, coord_label);
	clutter_actor_show(coord_label);

	/* Finish initialising the map view */
	g_object_set(G_OBJECT(map), "zoom-level", 2, NULL);
	champlain_view_center_on(CHAMPLAIN_VIEW(map), 0.0, 0.0);

	clutter_actor_show(stage);
	clutter_main();

	exit(EXIT_SUCCESS);
}
