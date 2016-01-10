#ifndef __DL_TUTORIAL_H__
#define __DL_TUTORIAL_H__

#define PLUGIN_NAME_LEN   16

/* Definition and typedef for our plugin structure */
struct _plugin {
    char name[PLUGIN_NAME_LEN];  /* 'Name' of the loaded plugin */
    void *handle;      /* Opaque handle used by DL */
    void (*func)();    /* function pointer for plugin function */
};
typedef struct _plugin plugin;

/* Maximum number of plugins to allow */
#define MAX_PLUGINS    4

/* Info types for the show_info() function */
#define SHOW_POINTERS     0
#define SHOW_HANDLES      1
#define SHOW_NAMES        2

#endif  /* __DL_TUTORIAL_H__ */
