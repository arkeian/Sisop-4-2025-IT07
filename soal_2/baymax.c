#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>

#define RELICS_DIR "relics"
#define LOG_FILE "activity.log"
#define CHUNK_SIZE 1024

// Helper untuk logging
void write_log(const char *action, const char *detail) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) return;

    time_t t = time(NULL);
    struct tm *lt = localtime(&t);
    fprintf(log, "[%04d-%02d-%02d %02d:%02d:%02d] %s: %s\n",
            lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
            lt->tm_hour, lt->tm_min, lt->tm_sec, action, detail);

    fclose(log);
}

static int baymax_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else if (strcmp(path, "/Baymax.jpeg") == 0) {
        stbuf->st_mode = S_IFREG | 0644;
        stbuf->st_nlink = 1;

        // Hitung size gabungan dari 14 pecahan
        char chunk_path[256];
        FILE *f;
        size_t size = 0;
        for (int i = 0; i < 14; i++) {
            sprintf(chunk_path, "%s/Baymax.jpeg.%03d", RELICS_DIR, i);
            f = fopen(chunk_path, "rb");
            if (f) {
                fseek(f, 0, SEEK_END);
                size += ftell(f);
                fclose(f);
            }
        }
        stbuf->st_size = size;
    } else {
        return -ENOENT;
    }
    return 0;
}

static int baymax_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
                          struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);
    filler(buf, "Baymax.jpeg", NULL, 0, 0);
    return 0;
}

static int baymax_open(const char *path, struct fuse_file_info *fi) {
    if (strcmp(path, "/Baymax.jpeg") != 0)
        return -ENOENT;
    write_log("READ", path + 1);
    return 0;
}

static int baymax_read(const char *path, char *buf, size_t size, off_t offset,
                       struct fuse_file_info *fi) {
    if (strcmp(path, "/Baymax.jpeg") != 0)
        return -ENOENT;

    size_t bytes_read = 0;
    int chunk_index = offset / CHUNK_SIZE;
    size_t chunk_offset = offset % CHUNK_SIZE;

    while (size > 0 && chunk_index < 14) {
        char chunk_path[256];
        sprintf(chunk_path, "%s/Baymax.jpeg.%03d", RELICS_DIR, chunk_index);

        FILE *chunk = fopen(chunk_path, "rb");
        if (!chunk) break;

        fseek(chunk, chunk_offset, SEEK_SET);
        size_t to_read = CHUNK_SIZE - chunk_offset;
        if (to_read > size) to_read = size;

        size_t result = fread(buf + bytes_read, 1, to_read, chunk);
        fclose(chunk);

        if (result == 0) break;
        size -= result;
        bytes_read += result;

        chunk_index++;
        chunk_offset = 0;
    }
    return bytes_read;
}

static int baymax_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    char *filename = strdup(path + 1);
    char tmp_path[256];
    sprintf(tmp_path, "%s_temp", filename);
    FILE *f = fopen(tmp_path, "wb");
    if (!f) return -EIO;
    fclose(f);
    fi->fh = (uint64_t)strdup(tmp_path);
    return 0;
}

static int baymax_write(const char *path, const char *buf, size_t size, off_t offset,
                        struct fuse_file_info *fi) {
    char *tmp_path = (char *)fi->fh;
    FILE *f = fopen(tmp_path, "r+b");
    if (!f) f = fopen(tmp_path, "wb");
    if (!f) return -EIO;

    fseek(f, offset, SEEK_SET);
    fwrite(buf, 1, size, f);
    fclose(f);
    return size;
}

static int baymax_release(const char *path, struct fuse_file_info *fi) {
    char *tmp_path = (char *)fi->fh;
    FILE *src = fopen(tmp_path, "rb");
    if (!src) return -EIO;

    char *filename = strdup(path + 1);
    char logline[1024];
    sprintf(logline, "%s -> ", filename);

    int i = 0;
    while (1) {
        char chunk_path[256];
        sprintf(chunk_path, "%s/%s.%03d", RELICS_DIR, filename, i);
        FILE *out = fopen(chunk_path, "wb");
        if (!out) break;

        char buf[CHUNK_SIZE];
        size_t n = fread(buf, 1, CHUNK_SIZE, src);
        if (n == 0) {
            fclose(out);
            remove(chunk_path);
            break;
        }
        fwrite(buf, 1, n, out);
        fclose(out);

        if (i > 0) strcat(logline, ", ");
        char frag[64];
        sprintf(frag, "%s.%03d", filename, i);
        strcat(logline, frag);
        i++;
    }
    fclose(src);
    remove(tmp_path);
    free(tmp_path);

    write_log("WRITE", logline);
    return 0;
}

static int baymax_unlink(const char *path) {
    char *filename = strdup(path + 1);
    int i = 0;
    char chunk_path[256];
    for (; ; i++) {
        sprintf(chunk_path, "%s/%s.%03d", RELICS_DIR, filename, i);
        if (access(chunk_path, F_OK) != 0) break;
        remove(chunk_path);
    }
    char logline[256];
    sprintf(logline, "DELETE: %s.000 - %s.%03d", filename, filename, i - 1);
    write_log("DELETE", logline);
    return 0;
}

static const struct fuse_operations baymax_oper = {
    .getattr = baymax_getattr,
    .readdir = baymax_readdir,
    .open = baymax_open,
    .read = baymax_read,
    .create = baymax_create,
    .write = baymax_write,
    .release = baymax_release,
    .unlink = baymax_unlink,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &baymax_oper, NULL);
}
