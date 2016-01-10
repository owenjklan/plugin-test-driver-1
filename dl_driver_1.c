/**
   dl_driver_1.c - simple test driver program to demonstrate usage
   of the Linux Dynamic Loader.

   Written by Owen Klan  -  10th December, 2015

   owen@digitalsnowflake.net
*/

/* Standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Include file required for the Dynamic Loader wrapper functions */
#include <dlfcn.h>

/* "API" include file for plugins */
#include "dltutorial.h"

/* An array of "plugin functions" */
plugin plugin_slots[MAX_PLUGINS];

/* Function that loads a plugin from a file, into a given slot number.
   Returns 1 on successful load, 0 on failure */
int load_plugin(char *file, int slot) {
    void *handle;
    void (*func)() = NULL;
    char *name = NULL;
    
    if ((handle = dlopen(file, RTLD_LAZY)) == NULL) {
	printf("Failed loading '%s'! dlopen() error:  %s\n",
	       file, dlerror());
	return 0;
    }

    /* Try to resolve the address for 'func()' before we bother saving
       anything. If this fails, close the handle and return failure (0) */
    if ((func = dlsym(handle, "func")) == NULL) {
	printf("Failed locating 'func' in plugin! dlsym() error:  %s\n",
	       dlerror());
	dlclose(handle);
	return 0;
    }

    /* Try to resolve the address for the 'plugin_name' string. */
    if ((name = dlsym(handle, "plugin_name")) == NULL) {
	printf("Failed locating 'plugin_name' in plugin! dlsym() error:  %s\n",
	       dlerror());
	dlclose(handle);
	return 0;
    }

    /* We have successfully opened the library and located the function
       and name symbols. Store this in our specific slot structure */
    plugin_slots[slot].handle = handle;
    plugin_slots[slot].func   = func;
    memset(plugin_slots[slot].name, 0x00, PLUGIN_NAME_LEN);
    strncpy(plugin_slots[slot].name, name, PLUGIN_NAME_LEN);
    
    return 1;
}

/* Function to process the entry to the Load command. */
void load_command(char *args) {
    char *sep_char = strchr(args, ' ');
    char *filename;
    int i = 0;
    int next_slot = 0;   /* Set to one if we use the next free slot */
    int slotnum = 0;
    
    if (sep_char != NULL) {
	*sep_char = '\0';
    } else {
	next_slot = 1;
	
	/* determine next available slot */
	for (i = 0; i < MAX_PLUGINS; i++) {
	    if (plugin_slots[i].handle != NULL)
		continue;   /* Try the next slot */
	    else
		break;      /* First free slot found */
	}

	/* Seeing as we were given just a filename, we'll trim off the
	   ending newline character */
	*(args + strlen(args) - 1) = '\0';
    }

    filename = args;

    /* If a specific slot number wasn't given, process it from the passed
       in argument string */
    if (next_slot == 0)
	slotnum = strtol(sep_char+1, NULL, 10);
    else
	slotnum = i;
    
    printf("Will load '%s' into slot number %d...\n",
	   filename, slotnum);

    if (!load_plugin(filename, slotnum)) {
	printf("Plugin loading failed!\n");
    }
}

/* Unload the plugin in a given slot */
void unload_plugin(int slot) {
    /* Check for valid slot number */
    if (slot < 0 || slot >= MAX_PLUGINS) {
        printf("Invalid slot specified: %d\n", slot);
        return;
    }

    /* If function pointer is NULL then we don't have a plugin loaded */
    if (plugin_slots[slot].func == NULL) {
        printf("Selected slot (%d) doesn't appear to have a plugin loaded. Unload aborted.\n",
               slot);
        return;
    }

    /* Close library file */
    if (dlclose(plugin_slots[slot].handle) != 0) {
        printf("There was an error closing the shared object associated with %s!\n%s\n",
               plugin_slots[slot].name, dlerror());
        return;
    } else {
        /* Zero out plugin slot */
        memset(&plugin_slots[slot], 0x00, sizeof(plugin));
    }
}

/* Run the function that was loaded by a plugin in the given slot */
void run_plugin(int slot) {
    void (*f)() = NULL;

    /* Retrieve function pointer for given plugin slot */
    f = plugin_slots[slot].func;

    /* Check to be sure that we have a valid function pointer */
    if (f == NULL) {
        printf("No function available to run. Aborting.\n");
        return;
    }

    /* Execute the function */
    f();
}

/* Function that displays contents of loaded plugins.
   'type' determines what to display for each slot. If 'type' is
   SHOW_POINTERS, then we print out the pointer value for the plugin
   function, func(). If 'type' is SHOW_NAME then we print the name value.
   If 'type' is SHOW_HANDLE, we print out the value of the 'handle' */
void show_info(int info_type) {
    int i = 0;	
    switch (info_type) {
    case SHOW_POINTERS: {
	for (i = 0; i < MAX_PLUGINS; i++) {
	    /* Print plugin info, only if plugin slot is loaded */
	    if (plugin_slots[i].handle)
		printf("Plugin Slot %2d:  &func():  %p\n",
		       i, plugin_slots[i].func);
	    else
		printf("Plugin Slot %2d:  NULL  (No Plugin Loaded)\n", i);
	}
	break;
    }
    case SHOW_HANDLES: {
	for (i = 0; i < MAX_PLUGINS; i++) {
	    /* Print plugin info, only if plugin slot is loaded */
	    if (plugin_slots[i].handle)
		printf("Plugin Slot %2d:  handle:  %p\n",
		       i, plugin_slots[i].handle);
	    else
		printf("Plugin Slot %2d:  NULL  (No Plugin Loaded)\n", i);
	}
	break;	    
    }
    case SHOW_NAMES: {
	for (i = 0; i < MAX_PLUGINS; i++) {
	    /* Print plugin info, only if plugin slot is loaded */
	    if (plugin_slots[i].handle)
		printf("Plugin Slot %2d:  name:  %s\n",
		       i, plugin_slots[i].name);
	    else
		printf("Plugin Slot %2d:  NULL  (No Plugin Loaded)\n", i);
	}
	break;
    }
    default: {
	printf("Invalid argument to 'show_info()'!  %d given.\n", info_type);
    }
    }
}

/* Display program help, giving available commands and brief summary of each */
void show_help() {
    puts("\nAvailable Commands:");
    puts("help          ...                Display this screen");
    puts("quit          ...                Exit the driver program");
    puts("show [handles|pointers|names]    Show info of given type for each slot");
    puts("load file [slotnum] ...          Load a plugin");
    puts("unload slotnum");
    puts("run slotnum");
}

/* Main command processing loop for the driver program.
   Returns non-zero if we are to keep processing commands.
   Returns zero when the 'quit' command is executed */
int main_loop() {
#define MAX_LINE_LEN  60
    char command[MAX_LINE_LEN];

    memset(command, 0x00, MAX_LINE_LEN);
    
    printf(">> ");
    fflush(stdout);
    fgets(command, MAX_LINE_LEN, stdin);

    /* Blank line means we'll go around again */
    if (command[0] == '\n')
	return 1;
    
    /* Process the input command */
    if (strncmp(command, "quit", 4) == 0) {
	puts("Exiting driver program...");
	return 0;   /* Return zero to exit main loop */
    } else if (strncmp(command, "help", 4) == 0) {
	show_help();
	return 1;
    } else if (strncmp(command, "show", 4) == 0) {
	/* Process remainder of line to determine type of information
	   we want to show */
	if (strncmp((command + 5), "handles", 7) == 0) {
	    show_info(SHOW_HANDLES);
	} else if (strncmp((command + 5), "pointers", 8) == 0) {
	    show_info(SHOW_POINTERS);
	} else if (strncmp((command + 5), "names", 5) == 0) {
	    show_info(SHOW_NAMES);
	} else {
	    printf("'show' command requires argument of 'handles', 'pointers'"
		   ", or 'names'\n");
	    return 1;
	}
	return 1;
    } else if (strncmp(command, "load", 4) == 0) {
	/* Trim trailing newline */
	*(command + strlen(command) - 1) = '\0';
	load_command((command + 5));
	return 1;
    } else if (strncmp(command, "unload", 6) == 0) {
	unload_plugin(strtol((command + 7), NULL, 10));
	return 1;
    } else if (strncmp(command, "run", 3) == 0) {
        run_plugin(strtol((command + 4), NULL, 10));
	return 1;
    } else {
	*(command + strlen(command) - 1) = '\0';
	printf("Unknown command:  '%s'\n", command);
	return 1;
    }

    /* Should never be reached. Explicit return keeps warnings at bay */
    return 1;  /* Let's just keep going */
}

/* main entry point */
int main (int argc, char *argv[]) {
    /* Display entry text */
    puts("Type 'help' for list of commands");
    puts("Type 'quit' to exit");

    while (1) {
	if (main_loop() != 1)
	    break;
    }

    return 0;
}

    
