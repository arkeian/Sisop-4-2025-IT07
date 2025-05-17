#define FUSE_USE_VERSION 31
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

static const char *host_dir = "/it24_host";

char* reverse(const char *str) {
    int len = strlen(str);
    char *reversed = malloc(len + 1);
    for (int i = 0; i < len; i++) {
        reversed[i] = str[len - i - 1];
    }
    reversed[len] = '\0';
    return reversed;
}

int is_dangerous(const char *filename) {
    return strstr(filename, "nafis") || strstr(filename, "kimcun");
}

void log_activity(const char *action, const char *path) {
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm);

    FILE *log_file = fopen("/var/log/it24/it24.log", "a");
    if (log_file) {
        fprintf(log_file, "[%s] %s: %s\n", timestamp, action, path);
        fclose(log_file);
    }
}

static int antink_getattr(const char *path, struct stat *stbuf) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", host_dir, path);

    char *base = strrchr(path, '/') + 1;
    if (base == NULL) base = (char*)path + 1;
    
    char *reversed = reverse(base);
    char danger_path[1024];
    snprintf(danger_path, sizeof(danger_path), "%s/%.*s/%s", 
        host_dir, (int)(strlen(path) - strlen(base)), path, reversed);

    if (is_dangerous(reversed) && access(danger_path, F_OK) == 0) {
        int res = lstat(danger_path, stbuf);
        free(reversed);
        return res == -1 ? -errno : 0;
    }
    free(reversed);

    int res = lstat(full_path, stbuf);
    return res == -1 ? -errno : 0;
}

static int antink_readdir(const char *path, void *buf, fuse_fill_dir_t filler, 
                          off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", host_dir, path);

    DIR *dir = opendir(full_path);
    if (!dir) return -errno;

    struct dirent *de;
    while ((de = readdir(dir)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;

        char *name = de->d_name;
        char *display_name = name;

        if (is_dangerous(name)) {
            display_name = reverse(name);
        } else {
            display_name = strdup(name);
        }

        if (filler(buf, display_name, &st, 0)) break;
        free(display_name);
    }

    closedir(dir);
    return 0;
}

static int antink_open(const char *path, struct fuse_file_info *fi) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", host_dir, path);

    char *base = strrchr(path, '/') + 1;
    char *reversed = reverse(base);
    char danger_path[1024];
    snprintf(danger_path, sizeof(danger_path), "%s/%.*s/%s", 
        host_dir, (int)(strlen(path) - strlen(base)), path, reversed);

    if (is_dangerous(reversed) && access(danger_path, F_OK) == 0) {
        strcpy(full_path, danger_path);
    }
    free(reversed);

    int res = open(full_path, fi->flags);
    if (res == -1) return -errno;

    fi->fh = res;
    return 0;
}

static int antink_read(const char *path, char *buf, size_t size, off_t offset, 
                       struct fuse_file_info *fi) {
    int res = pread(fi->fh, buf, size, offset);
    if (res == -1) return -errno;

    char *base = strrchr(path, '/') + 1;
    char *reversed = reverse(base);
    int danger = is_dangerous(reversed);
    free(reversed);

    if (!danger) {
        for (int i = 0; i < res; i++) {
            if (isalpha(buf[i])) {
                if (buf[i] >= 'A' && buf[i] <= 'Z') {
                    buf[i] = 'A' + (buf[i] - 'A' + 13) % 26;
                } else {
                    buf[i] = 'a' + (buf[i] - 'a' + 13) % 26;
                }
            }
        }
    }

    log_activity("READ", path);
    return res;
}

static int antink_release(const char *path, struct fuse_file_info *fi) {
    (void) path;
    close(fi->fh);
    return 0;
}

static struct fuse_operations antink_ops = {
    .getattr    = antink_getattr,
    .open       = antink_open,
    .read       = antink_read,
    .readdir    = antink_readdir,
    .release    = antink_release,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &antink_ops, NULL);
}
