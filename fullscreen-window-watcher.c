#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#define BUF_SIZE 4096
static void tag_window(const char *addr, int add) {
  if (!addr || !addr[0])
    return;
  char cmd[512];
  int ret =
      snprintf(cmd, sizeof(cmd),
               "hyprctl dispatch tagwindow %s%sfullscreen_mode address:%s > /dev/null 2>&1",
               add ? "+" : "-- -", add ? "" : "", addr);
  if (ret < 0 || ret >= sizeof(cmd))
    return;
  system(cmd);
}
static void run_fullscreen_handler(const char *addr, int fs, int workspace) {
  if (!addr || !addr[0])
    return;
  char cmd[512];
  int ret = snprintf(
      cmd, sizeof(cmd),
      "/home/user/.config/hypr/UserScripts/FullscreenHandler.sh %s %d %d > /dev/null 2>&1",
      addr, fs, workspace);
  if (ret < 0 || ret >= sizeof(cmd))
    return;
  system(cmd);
}
static void query_active_window(void) {
  FILE *fp = popen("hyprctl activewindow -j", "r");
  if (!fp) {
    fprintf(stderr, "Failed to query active window\n");
    return;
  }
  char buf[BUF_SIZE];
  char address[128] = {0};
  int fullscreen = -1;
  int workspace = -1;
  int in_workspace = 0;
  while (fgets(buf, sizeof(buf), fp)) {
    if (strstr(buf, "\"address\"")) {
      sscanf(buf, " \"address\": \"%[^\"]\"", address);
    }
    if (strstr(buf, "\"fullscreen\"")) {
      sscanf(buf, " \"fullscreen\": %d", &fullscreen);
    }
    // Handle json workspace object
    if (strstr(buf, "\"workspace\"")) {
      in_workspace = 1;
    }
    if (in_workspace && strstr(buf, "\"id\"")) {
      sscanf(buf, " \"id\": %d", &workspace);
      in_workspace = 0; 
    }
  }
  pclose(fp);
  if (fullscreen == -1 || !address[0] || workspace == -1)
    return;
  //printf("fullscreen=%d window=%s workspace=%d\n", fullscreen, address, workspace);
  //fflush(stdout);
  if (fullscreen == 1) {
    tag_window(address, 1);
  } else if (fullscreen == 0) {
    tag_window(address, 0);
  }
  run_fullscreen_handler(address, fullscreen, workspace);
}
int main(void) {
  const char *runtime = getenv("XDG_RUNTIME_DIR");
  const char *sig = getenv("HYPRLAND_INSTANCE_SIGNATURE");
  if (!runtime || !sig) {
    fprintf(stderr, "Hyprland environment not detected\n");
    return 1;
  }
  char sockpath[512];
  int ret = snprintf(sockpath, sizeof(sockpath), "%s/hypr/%s/.socket2.sock",
                     runtime, sig);
  if (ret < 0 || ret >= sizeof(sockpath)) {
    fprintf(stderr, "Socket path too long\n");
    return 1;
  }
  int fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("socket");
    return 1;
  }
  struct sockaddr_un addr = {0};
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, sockpath, sizeof(addr.sun_path) - 1);
  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("connect");
    close(fd);
    return 1;
  }

  // Normalize workspaces
  char cmd[512];
  int resetRet = snprintf(
      cmd, sizeof(cmd),
      "/home/user/.config/hypr/UserScripts/FullscreenHandler.sh %s %d %d > /dev/null 2>&1",
      "discard", -1, -1);
  if (resetRet < 0 || resetRet >= sizeof(cmd))
    return 1;
  system(cmd);
  
  // Watch for changes
  char buf[BUF_SIZE];
  while (1) {
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    if (n < 0) {
      if (errno == EINTR)
        continue;
      perror("read");
      break;
    }
    if (n == 0)
      break;
    buf[n] = '\0';
    if (strstr(buf, "fullscreen>>")) {
      query_active_window();
    }
  }
  close(fd);
  return 0;
}