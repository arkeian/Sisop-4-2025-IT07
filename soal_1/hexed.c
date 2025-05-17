#define FUSE_USE_VERSION 31
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>

static const char *root_dir = "Anomal1";       
static const char *image_dir = "Anomal1/image"; 
static const char *log_path = "Anomal1/Conversion.log";

int ends_with(const char *str, const char *suffix) {
    size_t len_str = strlen(str);
    size_t len_suffix = strlen(suffix);
    return (len_str >= len_suffix) && 
           (strcmp(str + len_str - len_suffix, suffix) == 0);
}

unsigned char* hex_to_bin(const char *hexstr, size_t *len) {
    size_t hex_len = strlen(hexstr);
    *len = hex_len / 2;
    unsigned char *bin = malloc(*len);
    for (size_t i = 0; i < hex_len; i += 2) {
        sscanf(hexstr + i, "%2hhx", &bin[i/2]);
    }
    return bin;
}

void create_image_dir() {
    struct stat st = {0};
    if (stat(image_dir, &st) == -1) {
        mkdir(image_dir, 0777);
    }
}

void log_conversion(const char *src, const char *dst) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char date[20], time_str[20];
    strftime(date, sizeof(date), "%Y-%m-%d", t);
    strftime(time_str, sizeof(time_str), "%H:%M:%S", t);

    char log_entry[256];
    snprintf(log_entry, sizeof(log_entry), 
        "[%s][%s]: Successfully converted hexadecimal text %s to %s.\n",
        date, time_str, src, dst);

    FILE *log = fopen(log_path, "a");
    if (log) {
        fputs(log_entry, log);
        fclose(log);
    }
}

static int xmp_open(const char *path, struct fuse_file_info *fi) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", root_dir, path);

    if (ends_with(full_path, ".txt")) {
        FILE *file = fopen(full_path, "r");
        if (file) {
            fseek(file, 0, SEEK_END);
            long size = ftell(file);
            fseek(file, 0, SEEK_SET);
            char *hexstr = malloc(size + 1);
            fread(hexstr, 1, size, file);
            hexstr[size] = '\0';
            fclose(file);

            size_t bin_len;
            unsigned char *bin = hex_to_bin(hexstr, &bin_len);
            free(hexstr);

            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            char timestamp[20];
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H:%M:%S", t);

            char image_name[256];
            const char *base_name = strrchr(full_path, '/') + 1;
            snprintf(image_name, sizeof(image_name), "%s/%s_image_%s.png",
                    image_dir, base_name, timestamp);

            create_image_dir();
            FILE *image = fopen(image_name, "wb");
            if (image) {
                fwrite(bin, 1, bin_len, image);
                fclose(image);
                log_conversion(base_name, image_name);
            }
            free(bin);
        }
    }

    int fd = open(full_path, fi->flags);
    if (fd == -1) return -errno;
    fi->fh = fd;
    return 0;
}

static int xmp_getattr(const char *path, struct stat *stbuf) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", root_dir, path);
    return lstat(full_path, stbuf);
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi) {
    DIR *dp;
    struct dirent *de;
    char full_path[1024];
    
    snprintf(full_path, sizeof(full_path), "%s%s", root_dir, path);
    dp = opendir(full_path);
    if (!dp) return -errno;

    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0)) break;
    }
    closedir(dp);
    return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi) {
    int fd = openat(fi->fh, path, O_RDONLY);
    if (fd == -1) return -errno;
    
    int res = pread(fd, buf, size, offset);
    if (res == -1) res = -errno;
    
    close(fd);
    return res;
}

static struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .open    = xmp_open,
    .read    = xmp_read,
    .readdir = xmp_readdir,
};

int main(int argc, char *argv[]) {
    umask(0);
    return fuse_main(argc, argv, &xmp_oper, NULL);
}
