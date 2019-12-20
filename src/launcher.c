/* glib and GTK libraries. */
#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Libraries used to create the child process on Linux. */
#if __linux__
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/wait.h>
#endif

#define PROGRAM_TITLE "Program Name"

/* Path to the unity executable. */
#if __linux__
	#define EXEC_PATH "./game/linux_program_name.x86_64"
	#define BANNER_PATH "./banner.png"
#else
	#define EXEC_PATH "./game/win_program_name.exe"
	#define BANNER_PATH "./banner.png"
#endif

/* Names of the available graphics APIs. */
#if __linux__	
	const char *graphic_api_args[] = {"-force-glcore", "-force-vulkan"};
	const char *graphic_api_button_text_names[] = {"Launch with Open GL", "Launch with Vulkan"};
#else
	const char *graphic_api_args[] = {"-force-d3d11", "-force-d3d12", "-force-glcore", "-force-vulkan"};
	const char *graphic_api_button_text_names[] = {"Launch with DirectX11", "Launch with DirectX12", "Launch with Open GL", "Launch with Vulkan"};
#endif
#define GRAPHIC_API_COUNT G_N_ELEMENTS(graphic_api_args)
#define DEFAULT_GRAPHIC_API 0 /* Number of avaliable graphic APIs. */

/* Main program window. */
static GtkWidget *main_window = NULL;

void launch_game(unsigned api_id) {

    printf("Launching game...\n");

	gtk_widget_hide (main_window); // Hides the main windows.

	unsigned argc = 2; /* The arg count. */
	char **argv = (char**)malloc(argc * sizeof(char*)); /* Allocate space to hold the arg list. */

	/* Allocate space for the first arg (program name) string and coppies it to the correct place. */
	argv[0] = (char*)malloc(strlen(1 + EXEC_PATH) * sizeof(char));
	strcpy(argv[0], EXEC_PATH);

	/* Allocate space for the second arg (graphics api) string and coppies it to the correct place. */
	argv[1] = (char*)malloc((1 + strlen(graphic_api_args[api_id])) * sizeof(char));
	strcpy(argv[1], graphic_api_args[api_id]);

	/* Prints the arguments */
	for(int i = 0; i < argc; i++) {
		printf("%s ", argv[i]);
	}
	printf("\n");
	fflush(stdin);

	// Execute the came on Linux.
	#if __linux__

		/* Spawn a child process to run the program. */
		pid_t pid = fork();
		if (pid == 0) { /* Child process */
			execv(EXEC_PATH, argv);
			exit(127); /* only if execv fails */
		}
		else { /* Parent process */
			waitpid(pid, NULL, 0); /* Wait for child to exit */
		}

	# else

	#endif

	// Closes the program when the child process closes.
	gtk_window_close (GTK_WINDOW(main_window));

}

/* Called when a button is pressed. */
static void press_api_launch_button (GtkButton *widget, gpointer user_data) {

	/* Gets the clicked button label to check for graphic API.  */
	const gchar *api_button_name = gtk_button_get_label (widget);
	
	/* Verifies the selected graphics API. */
	unsigned api_id;
	for(unsigned i = 0; i < GRAPHIC_API_COUNT + 1; i++) {
		/* Compares the label in the button to the API button names to determine which was clicked. */
		if (g_strcmp0(api_button_name, graphic_api_button_text_names[i]) == 0) { 
			api_id = i;
		}
	}

	/* Launch the game with the correct API. */
	launch_game(api_id);

}

/* Creates the window and program interface. */
static void activate (GtkApplication *app, gpointer user_data) {

	/* Creates the main window of the program. */
	main_window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (main_window), PROGRAM_TITLE);
	gtk_window_set_default_size (GTK_WINDOW (main_window), 360, 480);
	gtk_window_set_resizable(GTK_WINDOW (main_window), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (main_window), 10);

	/* Creates layout button to put other elements in. */
	GtkWidget *layout = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
	/* Adds grid to main window. */
	gtk_container_add (GTK_CONTAINER (main_window), layout);

	#if defined(BANNER_PATH)
	
		#if __linux__

			/* Checks if the file exists. */
			if(access(BANNER_PATH, F_OK) != -1) {

				/* Creates game banner */
				GtkWidget *banner = gtk_image_new_from_file (BANNER_PATH);
				/* Adds to the grid. */
				gtk_box_pack_start (GTK_BOX (layout), banner, FALSE, FALSE, 0);

			}

		#endif

	#endif

	/* Creates buttons for each API. */
	for(int i = 0; i < GRAPHIC_API_COUNT; i++) {

		/* Creates the button. */
		GtkWidget *api_button = gtk_button_new_with_label (graphic_api_button_text_names[i]);
		/* Adds to the grid. */
		gtk_box_pack_start (GTK_BOX (layout), api_button, FALSE, FALSE, 0);
		/* Connects signal for when the button is pressed. */
		g_signal_connect (api_button, "button-press-event", G_CALLBACK (press_api_launch_button), NULL);

	}

	/* Displays the interface. */
	gtk_widget_show_all (main_window);

}

int main (int argc, char **argv) {

	GtkApplication *app;
	int status;

	app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);

	return status;

}
