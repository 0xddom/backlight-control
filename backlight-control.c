#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>

#define KB_CMD "keyboard"
#define SC_CMD "screen"

/* Use this macros only with literal strings */
#define KB_FILE(f) "/sys/class/leds/smc::kbd_backlight/" f
#define SC_FILE(f) "/sys/class/backlight/intel_backlight/" f
#define BRI "brightness"
#define MAX_BRI "max_brightness"

void usage(char *s) {
    fprintf (stderr, "[USAGE]: %s <keyboard/screen> <value>\n", s);
    exit (1);
}

int read_value(char *path) {
    FILE *fd;
    char buf[255];

    if ((fd = fopen (path, "r")) == NULL) {
        fprintf (stderr, "Can't open file %s: %s\n", path, strerror (errno));
        exit (1);
    }

    if (fread (buf, 255, 1, fd) < 0) {
        fprintf (stderr, "Error while reading the file %s: %s\n", path, strerror (errno));
        exit (1);
    }

    fclose (fd);
    return atoi (buf);
}

void write_value(char *path, int value) {
    FILE *fd;

    if ((fd = fopen (path, "w")) == NULL) {
        fprintf (stderr, "Can't open file %s: %s\n", path, strerror (errno));
        exit (1);
    }

    fprintf (fd, "%d", value);
    fclose (fd);
}

int valid_command(char *cmd) {
    if (strcmp (KB_CMD, cmd) == 0) return 1;
    if (strcmp (SC_CMD, cmd) == 0) return 1;
    return 0;
}

int do_command(int bri, int max_bri, char *action) {
    int relative_change = 0;
    int change_value;
    float new_value;

    if (*action == '+') {
        relative_change = 1;
        action++;
    }
    if (*action == '-') {
        relative_change = -1;
        action++;
    }

    change_value = atoi (action);
    if (change_value < 0 || change_value > 100) {
        fprintf (stderr, "The value must be between 0 and 100. Got: %d\n", change_value);
        exit (1);
    }

    new_value = (float)change_value / 100;
    new_value *= max_bri;
    if (relative_change != 0) {
        new_value *= relative_change;
        new_value = bri + new_value;
    }

    return fmaxf(0, fminf (max_bri, floorf (new_value)));
}

int main(int argc, char **argv) {
    char *bri_path, *max_bri_path;
    int new_value;
    uid_t user_uid;

    if (argc < 3) usage (argv[0]);
    if (!valid_command (argv[1])) usage (argv[0]);

    if (strcmp (KB_CMD, argv[1]) == 0) {
        bri_path = KB_FILE (BRI);
        max_bri_path = KB_FILE (MAX_BRI);
    }
    if (strcmp (SC_CMD, argv[1]) == 0) {
        bri_path = SC_FILE (BRI);
        max_bri_path = SC_FILE (MAX_BRI);
    }

    new_value = do_command (read_value (bri_path), read_value (max_bri_path), argv[2]);

    user_uid = geteuid ();
    if (seteuid (0) < 0) {
        perror (strerror (errno));
        exit (1);
    }
    write_value (bri_path, new_value);
    seteuid (user_uid);
    return 0;
}
