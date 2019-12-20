#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if __linux__
	#include <unistd.h> /* for fork */
	#include <sys/types.h> /* for pid_t */
	#include <sys/wait.h> /* for wait */
#endif

#define PROGRAM_TITLE "Program Name"

#if __LINUX__
	#define EXEC_PATH "./game/program_name.x86_64"
#else
	#define EXEC_PATH "./game/program_name.exe"
#endif

/* Names of the available graphics APIs. */
#if __linux__	
	const char *graphics_apis_args[] = {"-force-glcore", "-force-vulkan"};
	const char *graphics_apis_names[] = {"", "Open GL", "Vulkan"};
#else
	const char *graphics_apis_args[] = {"-force-d3d11", "-force-d3d12", "-force-glcore", "-force-vulkan"};
	const char *graphics_apis_names[] = {"", "DirectX11", "DirectX12", "Open GL", "Vulkan"};
#endif
#define GRAPHICS_API_COUNT G_N_ELEMENTS(graphics_apis_args)

const char *minimal_args[] = {"-screen-width", "1024", "-screen-height", "768", "-screen-fullscreen", "0", "-screen-quality", "Very Low"};
#define MINIMAL_ARGS_COUNT G_N_ELEMENTS(minimal_args)

#define DEFAULT_GRAPHICS_API 0

/* Current graphics api. */
int current_graphics_api;
/* If minimal mode is enabled. */
gboolean minimal_graphics_mode;

/* Main program window. */
GtkWidget *main_window;

static void on_api_dropdown_changed (GtkComboBox *widget, gpointer user_data) {

	GtkComboBox *api_dropdown = widget;

	/* Gets the changed API. */
	gchar *api = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT(api_dropdown));
	
	/* Verifies the selected graphics API. */
	int cur_api = -1;
	for(int i = 0; i < GRAPHICS_API_COUNT + 1; i++) {
		if (g_strcmp0(api, graphics_apis_names[i]) == 0) {
			cur_api = i - 1;
		}
	}

	/* If it's a valid API sets it. */
	if(cur_api != -1) {
		current_graphics_api = cur_api;
	} else { /* Else sets to the default. */
		cur_api = DEFAULT_GRAPHICS_API;
		/* Sets the default option on the graphics API dropdown. */
		gtk_combo_box_set_active (GTK_COMBO_BOX (api_dropdown), DEFAULT_GRAPHICS_API + 1);
	}

	printf("Current graphics API set to %s (%d)\n", graphics_apis_names[cur_api+1], cur_api);

	g_free (api);

}

static void on_toggle_minimal_graphics (GtkToggleButton *widget, gpointer user_data) {

	GtkToggleButton *minimal_toggle = widget;

	/* Gets the changed API. */
	minimal_graphics_mode = gtk_toggle_button_get_active(minimal_toggle);

	if(minimal_graphics_mode == TRUE)
		printf("Minimal graphics mode enabled\n");
	else
		printf("Minimal graphics mode disabled\n");

}

void launch_game(GtkWidget *widget, gpointer data ) {

    printf("Launching game...\n");

	gtk_widget_hide (main_window); // Hides the main windows.

	#if __linux__

		int argc = minimal_graphics_mode ? 2 + MINIMAL_ARGS_COUNT : 2; /* Gets the arg count. */
		char **argv = (char**)malloc(argc * sizeof(char*)); /* Allocate space to hold the arg list. */

		/* Allocate space for the first arg (program name) string and coppies it to the correct place. */
		argv[0] = (char*)malloc(strlen(1 + EXEC_PATH) * sizeof(char));
		strcpy(argv[0], EXEC_PATH);

		/* Allocate space for the second arg (graphics api) string and coppies it to the correct place. */
		argv[1] = (char*)malloc((1 + strlen(graphics_apis_args[current_graphics_api])) * sizeof(char));
		strcpy(argv[1], graphics_apis_args[current_graphics_api]);

		/* Does the same to the remaining args if in minimal graphics mode */
		if(minimal_graphics_mode) {
			for(int i = 0; i < MINIMAL_ARGS_COUNT; i++) {
				argv[2 + i] = (char*)malloc((1 + strlen(minimal_args[i])) * sizeof(char));
				strcpy(argv[2 + i], minimal_args[i]);
			}
		}

		/* Prints the arguments */
		printf("argc: %d argv:", argc);
		for(int i = 0; i < argc; i++) {
			printf(" %s", argv[i]);
		}
		printf("\n");

		/* Spawn a child process to run the program. */
		pid_t pid = fork();
		if (pid == 0) { /* Child process */
			execv(EXEC_PATH, argv);
			exit(127); /* only if execv fails */
		}
		else { /* Parent process */
			waitpid(pid, NULL, 0); /* Wait for child to exit */
		}

	#endif

	// Closes the program when the child process closes.
	gtk_window_close (GTK_WINDOW(main_window));

}

static void activate (GtkApplication *app, gpointer user_data) {

	/* MAIN WINDOW ============================================================================================ */

	/* Creates the main window of the program. */
	main_window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (main_window), PROGRAM_TITLE);
	gtk_window_set_default_size (GTK_WINDOW (main_window), -1, -1);
	gtk_container_set_border_width (GTK_CONTAINER (main_window), 10);

	/* GRID LAYOUT =========================================================================================== */
	GtkWidget *layout = gtk_grid_new ();
	gtk_container_add (GTK_CONTAINER (main_window), layout);

	/* API DROPDOWN ========================================================================================== */

	/* Creates a label to indicate what the combo box is for. */
	GtkWidget *api_dropdown_label = gtk_label_new_with_mnemonic ("Graphics API:");
	gtk_grid_attach (GTK_GRID (layout), api_dropdown_label, 0, 0, 1, 1); /* Adds to the grid. */

	/* Create the combo box and append graphic api names to it. */
	GtkWidget *api_dropdown = gtk_combo_box_text_new ();
	gtk_grid_attach (GTK_GRID (layout), api_dropdown, 0, 2, 1, 1); /* Adds to the grid. */ 
	
	/* Add elements to the dropdown.*/ 
	for (int i = 0; i < GRAPHICS_API_COUNT + 1; i++) {
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (api_dropdown), graphics_apis_names[i]);
	}

	/* Sets the default element of the dropdown. */
	gtk_combo_box_set_active (GTK_COMBO_BOX (api_dropdown), DEFAULT_GRAPHICS_API + 1);
	/* Connects signal for when the window changes. */
	g_signal_connect (api_dropdown, "changed", G_CALLBACK (on_api_dropdown_changed), NULL);

	/* TOGGLE MINIMAL ======================================================================================== */

	/* Creates a label to indicate what the toggle button is for. */
	GtkWidget *minimal_toggle_label = gtk_label_new_with_mnemonic ("Use minimal graphical settings");
	gtk_grid_attach (GTK_GRID (layout), minimal_toggle_label, 0, 3, 1, 1); /* Adds to the grid. */

	GtkWidget *minimal_toggle = gtk_toggle_button_new();
	gtk_grid_attach (GTK_GRID (layout), minimal_toggle, 1, 3, 1, 1); /* Adds to the grid. */
	/* Connects signal for when the toggle changes. */
	g_signal_connect (minimal_toggle, "toggled", G_CALLBACK (on_toggle_minimal_graphics), NULL);

	/* LAUNCH BUTTON ========================================================================================= */
	GtkWidget *launch_button = gtk_button_new_with_label ("Launch Game");
	gtk_grid_attach (GTK_GRID (layout), launch_button, 0, 4, 1, 1); /* Adds to the grid. */
	/* Connects signal for when the button is pressed. */
	g_signal_connect (launch_button, "button-press-event", G_CALLBACK (launch_game), NULL);

	/* ======================================================================================================= */

	gtk_widget_show_all (main_window);

}

int main (int argc, char **argv) {

	current_graphics_api = DEFAULT_GRAPHICS_API;
	minimal_graphics_mode = FALSE;

	GtkApplication *app;
	int status;

	app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);

	return status;

}
