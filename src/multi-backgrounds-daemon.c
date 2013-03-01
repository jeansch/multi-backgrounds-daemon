#include "gtk/gtk.h"
#include <gconf/gconf-client.h>
#include <stdio.h>
#include <string.h>
#include <libgnomevfs/gnome-vfs.h>

#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>

#define MULTI_BACKGROUND_GCONF_PATH "/desktop/gnome/multi-backgrounds"
#define DEFAULT_BACKGROUND_GCONF_PATH "/desktop/gnome/background"

#define TVALUE_string gchar *
#define TVALUE_int gint
#define TVALUE_bool gboolean

gchar *url_encode(char *str) {
  gchar *coded;
  char *p;
  coded = gnome_vfs_escape_path_string(str);
  p = coded;
  while(p) {
    p = strchr(coded, '%');
    if (p)
      *p = '_';
  }
  return coded;
}



WnckScreen *screen;
GConfClient *client;	

char *default_color_shading_type;
char *default_secondary_color;
char *default_primary_color;
char *default_picture_filename;
char *default_picture_options;
gint default_picture_opacity;
gboolean default_draw_background;

gint color_shading_type_id;
gint secondary_color_id;
gint primary_color_id;
gint picture_filename_id;
gint picture_options_id;
gint picture_opacity_id;
gint draw_background_id;

#define BACKGROUND_CHANGED_CB(type, entry) \
  void background_changed_ ## entry ## _cb (GConfClient *client_cb,	\
					    guint cnxn_id,		\
					    GConfEntry *entry, gpointer user_data) { \
    WnckWorkspace *workspace;						\
    const char *workspace_name;						\
    char key[200];							\
    gchar *urlkey;							\
    TVALUE_ ## type value;						\
    value = gconf_client_get_ ## type(client_cb,			\
				      DEFAULT_BACKGROUND_GCONF_PATH "/" # entry, NULL); \
    workspace = wnck_screen_get_active_workspace(screen);		\
    workspace_name = wnck_workspace_get_name(workspace);		\
    sprintf(key, MULTI_BACKGROUND_GCONF_PATH "/%s/" # entry, workspace_name); \
    urlkey = url_encode(key);						\
    gconf_client_set_ ## type(client_cb, urlkey, value, NULL);		\
    g_free(urlkey);							\
  }

BACKGROUND_CHANGED_CB(string, color_shading_type)
BACKGROUND_CHANGED_CB(string, secondary_color)
BACKGROUND_CHANGED_CB(string, primary_color)
BACKGROUND_CHANGED_CB(string, picture_filename)
BACKGROUND_CHANGED_CB(string, picture_options)
BACKGROUND_CHANGED_CB(int, picture_opacity)
BACKGROUND_CHANGED_CB(bool, draw_background)


void remove_monitors(void) {
#define BACKGROUND_GCONF_DENOTIFY(client, entry)	\
  gconf_client_notify_remove(client, entry ## _id)
  BACKGROUND_GCONF_DENOTIFY (client, color_shading_type);
  BACKGROUND_GCONF_DENOTIFY (client, secondary_color);
  BACKGROUND_GCONF_DENOTIFY (client, primary_color);
  BACKGROUND_GCONF_DENOTIFY (client, picture_filename);
  BACKGROUND_GCONF_DENOTIFY (client, picture_options);
  BACKGROUND_GCONF_DENOTIFY (client, picture_opacity);
  BACKGROUND_GCONF_DENOTIFY (client, draw_background);
}

void add_monitors(void) {
#define BACKGROUND_GCONF_NOTIFY(client, entry)				\
  entry ## _id  = gconf_client_notify_add(client,			\
					  DEFAULT_BACKGROUND_GCONF_PATH "/" # entry, \
					  background_changed_ ## entry ## _cb, NULL, NULL, NULL)
  BACKGROUND_GCONF_NOTIFY (client, color_shading_type);
  BACKGROUND_GCONF_NOTIFY (client, secondary_color);
  BACKGROUND_GCONF_NOTIFY (client, primary_color);
  BACKGROUND_GCONF_NOTIFY (client, picture_filename);
  BACKGROUND_GCONF_NOTIFY (client, picture_options);
  BACKGROUND_GCONF_NOTIFY (client, picture_opacity);
  BACKGROUND_GCONF_NOTIFY (client, draw_background);
}

static void workspace_changed_cb(WnckScreen *screen_cb, gpointer data) {
  WnckWorkspace *workspace;
  const char *workspace_name;	
  char *color_shading_type;
  char *secondary_color;
  char *primary_color;
  char *picture_filename;
  char *picture_options;
  gint picture_opacity;
  gboolean draw_background;
  char key[200];
  gchar *urlkey;
  char set_default = 0;

  workspace = wnck_screen_get_active_workspace(screen_cb);
  workspace_name = wnck_workspace_get_name(workspace);
  
  remove_monitors();
  
#define GET_BACKGROUND_VALUE(client, type, entry, workspace)		\
  sprintf(key, MULTI_BACKGROUND_GCONF_PATH "/%s/" # entry, workspace);	\
  urlkey = url_encode(key);						\
  entry = gconf_client_get_ ## type(client, urlkey, NULL);		\
  g_free(urlkey);							\
  if (entry == NULL) {							\
    entry = default_ ## entry;						\
    set_default = 1;							\
  }
  
  GET_BACKGROUND_VALUE (client, string, color_shading_type, workspace_name);
  GET_BACKGROUND_VALUE (client, string, secondary_color, workspace_name);
  GET_BACKGROUND_VALUE (client, string, primary_color, workspace_name);
  GET_BACKGROUND_VALUE (client, string, picture_options, workspace_name);
  GET_BACKGROUND_VALUE (client, string, picture_filename, workspace_name);
    
  /* opacity */
  if (!set_default) {
    sprintf(key, MULTI_BACKGROUND_GCONF_PATH "/%s/picture_opacity", workspace_name);
    urlkey = url_encode(key);
    picture_opacity = gconf_client_get_int(client, urlkey, NULL);
    g_free(urlkey);
  } else {
    picture_opacity = default_picture_opacity;
  }

  /* draw_background */
  if (!set_default) {
    sprintf(key, MULTI_BACKGROUND_GCONF_PATH "/%s/draw_background", workspace_name);
    urlkey = url_encode(key);
    draw_background = gconf_client_get_bool(client, urlkey, NULL);
    g_free(urlkey);
  } else {
    draw_background = default_draw_background;
  }

#define SET_BACKGROUND_VALUE(client, type, entry, workspace)		\
  gconf_client_set_ ## type(client, DEFAULT_BACKGROUND_GCONF_PATH "/" # entry, entry, NULL)
  SET_BACKGROUND_VALUE (client, string, picture_filename, workspace_name);
  SET_BACKGROUND_VALUE (client, int, picture_opacity, workspace_name);
  SET_BACKGROUND_VALUE (client, string, picture_options, workspace_name);
  SET_BACKGROUND_VALUE (client, bool, draw_background, workspace_name);
  SET_BACKGROUND_VALUE (client, string, color_shading_type, workspace_name);
  SET_BACKGROUND_VALUE (client, string, primary_color, workspace_name);
  SET_BACKGROUND_VALUE (client, string, secondary_color, workspace_name);

  add_monitors();
}


int main(int argc, char **argv) {
  gtk_init (&argc, &argv);
  client = gconf_client_get_default ();
  /* which confs to monitor */
  gconf_client_add_dir (client, DEFAULT_BACKGROUND_GCONF_PATH,
			GCONF_CLIENT_PRELOAD_NONE, NULL);
  gconf_client_add_dir (client, MULTI_BACKGROUND_GCONF_PATH,
			GCONF_CLIENT_PRELOAD_NONE, NULL);
  
#define GET_DEFAULT_BACKGROUND(client, type, entry)			\
  default_ ## entry = gconf_client_get_ ## type(client,			\
						DEFAULT_BACKGROUND_GCONF_PATH "/" # entry, \
						NULL)
  
  GET_DEFAULT_BACKGROUND(client, string, color_shading_type);
  GET_DEFAULT_BACKGROUND(client, bool, draw_background);
  GET_DEFAULT_BACKGROUND(client, string, picture_filename);
  GET_DEFAULT_BACKGROUND(client, int, picture_opacity);
  GET_DEFAULT_BACKGROUND(client, string, picture_options);
  GET_DEFAULT_BACKGROUND(client, string, primary_color);
  GET_DEFAULT_BACKGROUND(client, string, secondary_color);

  screen = wnck_screen_get_default ();
  wnck_screen_force_update (screen);	
  
  g_signal_connect(G_OBJECT (screen), "active_workspace_changed",
		   G_CALLBACK (workspace_changed_cb),
		   NULL);
  add_monitors();
  gtk_main ();
  return 0;
}
