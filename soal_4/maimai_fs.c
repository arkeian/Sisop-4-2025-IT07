#define _POSIX_C_SOURCE 200809L
#define FUSE_USE_VERSION 31
#include <fuse.h>
#include <zlib.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

#define BUFFER 256
#define CHUNK 16384

static const char *riildir = "chiho";

static int mai_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
    const char *truePath = path;
    if (strncmp(path, "/7sref/", 7) == 0)
    {
        char tmp[BUFFER];
        const char *whereUscore = path + 7;
        int delimiterLoc = strcspn(whereUscore, "_");
        snprintf(tmp, BUFFER, "/%.*s/%s", delimiterLoc, whereUscore, whereUscore + delimiterLoc + 1);
        truePath = tmp;
    }
    char fullPath[BUFFER];
    int res;
    (void)fi;
    memset(stbuf, 0, sizeof(*stbuf));

    if (strcmp(truePath, "/") == 0)
    {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }
    else if (strcmp(truePath, "/starter") == 0 || strcmp(truePath, "/metro") == 0 ||
             strcmp(truePath, "/dragon") == 0 || strcmp(truePath, "/blackrose") == 0 ||
             strcmp(truePath, "/heaven") == 0 || strcmp(truePath, "/skystreet") == 0 ||
             strcmp(truePath, "/7sref") == 0)
    {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }
    if (strcmp(truePath, "/starter") == 0 || strcmp(truePath, "/metro") == 0 ||
        strcmp(truePath, "/dragon") == 0 || strcmp(truePath, "/blackrose") == 0 ||
        strcmp(truePath, "/heaven") == 0 || strcmp(truePath, "/skystreet") == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s", riildir, truePath);
        res = lstat(fullPath, stbuf);

        if (res == -1)
        {
            return -errno;
        }
    }
    else if (strncmp(truePath, "/starter/", 9) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.mai", riildir, truePath);
        res = lstat(fullPath, stbuf);

        if (res == -1)
        {
            return -errno;
        }
    }
    else if (strncmp(truePath, "/metro/", 7) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.ccc", riildir, truePath);
        res = lstat(fullPath, stbuf);

        if (res == -1)
        {
            return -errno;
        }
    }
    else if (strncmp(truePath, "/dragon/", 8) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.rot", riildir, truePath);
        res = lstat(fullPath, stbuf);

        if (res == -1)
        {
            return -errno;
        }
    }
    else if (strncmp(truePath, "/blackrose/", 11) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.bin", riildir, truePath);
        res = lstat(fullPath, stbuf);

        if (res == -1)
        {
            return -errno;
        }
    }
    else if (strncmp(truePath, "/skystreet/", 11) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.gz", riildir, truePath);
        res = lstat(fullPath, stbuf);

        if (res == -1)
        {
            return -errno;
        }
    }
    else if (strncmp(truePath, "/heaven/", 8) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.enc", riildir, truePath);
        res = lstat(fullPath, stbuf);

        if (res == -1)
        {
            return -errno;
        }
    }

    return -ENOENT;
}
static int mai_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags)
{
    const char *truePath = path;
    if (strncmp(path, "/7sref/", 7) == 0)
    {
        char tmp[BUFFER];
        const char *whereUscore = path + 7;
        int delimiterLoc = strcspn(whereUscore, "_");
        snprintf(tmp, BUFFER, "/%.*s/%s", delimiterLoc, whereUscore, whereUscore + delimiterLoc + 1);
        truePath = tmp;
    }
    char dirPath[BUFFER];
    char *ext;

    if (strcmp(truePath, "/") == 0)
    {
        filler(buf, ".", NULL, 0, 0);
        filler(buf, "..", NULL, 0, 0);
        filler(buf, "starter", NULL, 0, 0);
        filler(buf, "metro", NULL, 0, 0);
        filler(buf, "dragon", NULL, 0, 0);
        filler(buf, "blackrose", NULL, 0, 0);
        filler(buf, "skystreet", NULL, 0, 0);
        filler(buf, "heaven", NULL, 0, 0);
        filler(buf, "7sref", NULL, 0, 0);
        return 0;
    }
    if (!(strcmp(truePath, "/starter") == 0 || strcmp(truePath, "/metro") == 0 ||
          strcmp(truePath, "/dragon") == 0 || strcmp(truePath, "/blackrose") == 0 ||
          strcmp(truePath, "/heaven") == 0 || strcmp(truePath, "/skystreet") == 0))
    {
        return -ENOENT;
    }
    else
    {
        snprintf(dirPath, BUFFER, "%s%s", riildir, truePath);
    }
    if (strncmp(truePath, "/starter/", 9) == 0)
    {
        ext = ".mai";
    }
    else if (strncmp(truePath, "/metro/", 7) == 0)
    {
        ext = ".ccc";
    }
    else if (strncmp(truePath, "/dragon/", 8) == 0)
    {
        ext = ".rot";
    }
    else if (strncmp(truePath, "/blackrose/", 11) == 0)
    {
        ext = ".bin";
    }
    else if (strncmp(truePath, "/skystreet/", 11) == 0)
    {
        ext = ".gz";
    }
    else if (strncmp(truePath, "/heaven/", 8) == 0)
    {
        ext = ".enc";
    }

    DIR *dp;
    struct dirent *de;
    (void)offset;
    (void)fi;
    (void)flags;

    dp = opendir(dirPath);

    if (dp == NULL)
    {
        return -errno;
    }

    while ((de = readdir(dp)) != NULL)
    {
        int fileNameLen = strlen(de->d_name);
        int ExtLen = strlen(ext);
        if (fileNameLen > ExtLen && strcmp(de->d_name + fileNameLen - ExtLen, ext) == 0)
        {
            char fileWoutExt[BUFFER];
            snprintf(fileWoutExt, (fileNameLen - ExtLen) + 1, "%.*s", fileNameLen - ExtLen, de->d_name);
            filler(buf, fileWoutExt, NULL, 0, 0);
        }
    }
    closedir(dp);
    return 0;
}
static int mai_open(const char *path, struct fuse_file_info *fi)
{
    const char *truePath = path;
    if (strncmp(path, "/7sref/", 7) == 0)
    {
        char tmp[BUFFER];
        const char *whereUscore = path + 7;
        int delimiterLoc = strcspn(whereUscore, "_");
        snprintf(tmp, BUFFER, "/%.*s/%s", delimiterLoc, whereUscore, whereUscore + delimiterLoc + 1);
        truePath = tmp;
    }

    char fullPath[BUFFER];
    if (strncmp(truePath, "/starter/", 9) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.mai", riildir, truePath);
    }
    else if (strncmp(truePath, "/metro/", 7) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.ccc", riildir, truePath);
    }
    else if (strncmp(truePath, "/dragon/", 8) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.rot", riildir, truePath);
    }
    else if (strncmp(truePath, "/blackrose/", 11) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.bin", riildir, truePath);
    }
    else if (strncmp(truePath, "/skystreet/", 11) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.gz", riildir, truePath);
    }
    else if (strncmp(truePath, "/heaven/", 8) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.enc", riildir, truePath);
    }
    else
    {
        return -ENOENT;
    }
    int res;

    res = open(fullPath, fi->flags, 0666);
    if (res == -1)
    {
        return -errno;
    }
    close(res);
    return 0;
}
static int mai_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    const char *truePath = path;
    if (strncmp(path, "/7sref/", 7) == 0)
    {
        char tmp[BUFFER];
        const char *whereUscore = path + 7;
        int delimiterLoc = strcspn(whereUscore, "_");
        snprintf(tmp, BUFFER, "/%.*s/%s", delimiterLoc, whereUscore, whereUscore + delimiterLoc + 1);
        truePath = tmp;
    }
    char fullPath[BUFFER];
    if (strncmp(truePath, "/starter/", 9) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.mai", riildir, truePath);
    }
    else if (strncmp(truePath, "/metro/", 7) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.ccc", riildir, truePath);
    }
    else if (strncmp(truePath, "/dragon/", 8) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.rot", riildir, truePath);
    }
    else if (strncmp(truePath, "/blackrose/", 11) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.bin", riildir, truePath);
    }
    else if (strncmp(truePath, "/skystreet/", 11) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.gz", riildir, truePath);
    }
    else if (strncmp(truePath, "/heaven/", 8) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.enc", riildir, truePath);
    }
    else
    {
        return -ENOENT;
    }
    int fd;
    int res;
    (void)fi;

    fd = open(fullPath, O_RDONLY);
    if (fd == -1)
    {
        return -errno;
    }
    res = pread(fd, buf, size, offset);
    if (res == -1)
    {
        return -errno;
    }
    close(fd);
    return res;
}
static int mai_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    const char *truePath = path;
    if (strncmp(path, "/7sref/", 7) == 0)
    {
        char tmp[BUFFER];
        const char *whereUscore = path + 7;
        int delimiterLoc = strcspn(whereUscore, "_");
        snprintf(tmp, BUFFER, "/%.*s/%s", delimiterLoc, whereUscore, whereUscore + delimiterLoc + 1);
        truePath = tmp;
    }
    char fullPath[BUFFER];
    if (strncmp(truePath, "/starter/", 9) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.mai", riildir, truePath);
    }
    else if (strncmp(truePath, "/metro/", 7) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.ccc", riildir, truePath);
    }
    else if (strncmp(truePath, "/dragon/", 8) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.rot", riildir, truePath);
    }
    else if (strncmp(truePath, "/blackrose/", 11) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.bin", riildir, truePath);
    }
    else if (strncmp(truePath, "/skystreet/", 11) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.gz", riildir, truePath);
    }
    else if (strncmp(truePath, "/heaven/", 8) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.enc", riildir, truePath);
    }
    else
    {
        return -ENOENT;
    }
    int fd;
    int res;
    (void)fi;

    if (strncmp(truePath, "/starter/", 9) == 0)
    {
        fd = open(fullPath, O_WRONLY);
        if (fd == -1)
        {
            return -errno;
        }
        res = pwrite(fd, buf, size, offset);
        if (res == -1)
        {
            res = -errno;
        }
        close(fd);
        return res;
    }
    else if (strncmp(truePath, "/metro/", 7) == 0)
    {
        fd = open(fullPath, O_WRONLY);
        if (fd == -1)
        {
            return -errno;
        }
        char shifted[size];
        for (int i = 0; i < size; i++)
        {
            shifted[i] = buf[i] + ((offset + i) % 256);
        }
        res = pwrite(fd, shifted, size, offset);
        if (res == -1)
        {
            res = -errno;
        }
        close(fd);
        return res;
    }
    else if (strncmp(truePath, "/dragon/", 8) == 0)
    {
        fd = open(fullPath, O_WRONLY);
        if (fd == -1)
        {
            return -errno;
        }
        char rotated[size];
        for (int i = 0; i < size; i++)
        {
            if (buf[i] >= 'A' && buf[i] <= 'Z')
            {
                rotated[i] = ((buf[i] - 'A' + 13) % 26) + 'A';
            }
            else if (buf[i] >= 'a' && buf[i] <= 'z')
            {
                rotated[i] = ((buf[i] - 'a' + 13) % 26) + 'a';
            }
            else
            {
                rotated[i] = buf[i];
            }
        }
        res = pwrite(fd, rotated, size, offset);
        if (res == -1)
        {
            res = -errno;
        }
        close(fd);
        return res;
    }
    else if (strncmp(truePath, "/blackrose/", 11) == 0)
    {
        fd = open(fullPath, O_WRONLY);
        if (fd == -1)
        {
            return -errno;
        }
        res = pwrite(fd, buf, size, offset);
        if (res == -1)
        {
            res = -errno;
        }
        close(fd);
        return res;
    }
    else if (strncmp(truePath, "/skystreet/", 11) == 0)
    {
        fd = open(fullPath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd == -1)
        {
            return -errno;
        }
        int ret;
        unsigned have;
        z_stream strm;
        unsigned char out[CHUNK];

        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
        if (ret != Z_OK)
        {
            close(fd);
            return -errno;
        }
        strm.next_in = (Bytef *)buf;
        strm.avail_in = size;

        do
        {
            strm.next_out = out;
            strm.avail_out = CHUNK;
            ret = deflate(&strm, Z_FINISH);

            if (ret == Z_STREAM_ERROR)
            {
                (void)deflateEnd(&strm);
                close(fd);
                return -errno;
            }
            have = CHUNK - strm.avail_out;

            if (pwrite(fd, out, have, offset) != have)
            {
                (void)deflateEnd(&strm);
                close(fd);
                return -errno;
            }
        } while (strm.avail_out == 0);

        (void)deflateEnd(&strm);
        close(fd);

        return size;
    }
    else if (strncmp(truePath, "/heaven/", 8) == 0)
    {
        fd = open(fullPath, O_WRONLY);
        if (fd == -1)
        {
            return -errno;
        }
        unsigned char iv[16];
        RAND_bytes(iv, sizeof(iv));
        if (pwrite(fd, iv, sizeof(iv), 0) != sizeof(iv))
        {
            close(fd);
            return -errno;
        }
        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

        unsigned char key[32] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
                                 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
                                 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
                                 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31};

        unsigned char out[BUFFER + EVP_MAX_BLOCK_LENGTH];
        int len;
        EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);
        EVP_EncryptUpdate(ctx, out, &len, (const unsigned char *)buf, size);
        int cipherLen = len;
        EVP_EncryptFinal_ex(ctx, out + len, &len);
        cipherLen += len;
        if (pwrite(fd, out, cipherLen, sizeof(iv)) != cipherLen)
        {
            EVP_CIPHER_CTX_free(ctx);
            close(fd);
            return -errno;
        }

        EVP_CIPHER_CTX_free(ctx);
        close(fd);

        return size;
    }
}
static int mai_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    const char *truePath = path;
    if (strncmp(path, "/7sref/", 7) == 0)
    {
        char tmp[BUFFER];
        const char *whereUscore = path + 7;
        int delimiterLoc = strcspn(whereUscore, "_");
        snprintf(tmp, BUFFER, "/%.*s/%s", delimiterLoc, whereUscore, whereUscore + delimiterLoc + 1);
        truePath = tmp;
    }
    char fullPath[BUFFER];
    if (strncmp(truePath, "/starter/", 9) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.mai", riildir, truePath);
    }
    else if (strncmp(truePath, "/metro/", 7) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.ccc", riildir, truePath);
    }
    else if (strncmp(truePath, "/dragon/", 8) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.rot", riildir, truePath);
    }
    else if (strncmp(truePath, "/blackrose/", 11) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.bin", riildir, truePath);
    }
    else if (strncmp(truePath, "/skystreet/", 11) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.gz", riildir, truePath);
    }
    else if (strncmp(truePath, "/heaven/", 8) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.enc", riildir, truePath);
    }
    else
    {
        return -ENOENT;
    }
    int res;
    (void)fi;

    res = open(fullPath, O_CREAT | O_WRONLY, mode);
    if (res == -1)
    {
        return -errno;
    }
    close(res);
    return 0;
}
static int mai_unlink(const char *path)
{
    const char *truePath = path;
    if (strncmp(path, "/7sref/", 7) == 0)
    {
        char tmp[BUFFER];
        const char *whereUscore = path + 7;
        int delimiterLoc = strcspn(whereUscore, "_");
        snprintf(tmp, BUFFER, "/%.*s/%s", delimiterLoc, whereUscore, whereUscore + delimiterLoc + 1);
        truePath = tmp;
    }
    char fullPath[BUFFER];
    if (strncmp(truePath, "/starter/", 9) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.mai", riildir, truePath);
    }
    else if (strncmp(truePath, "/metro/", 7) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.ccc", riildir, truePath);
    }
    else if (strncmp(truePath, "/dragon/", 8) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.rot", riildir, truePath);
    }
    else if (strncmp(truePath, "/blackrose/", 11) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.bin", riildir, truePath);
    }
    else if (strncmp(truePath, "/skystreet/", 11) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.gz", riildir, truePath);
    }
    else if (strncmp(truePath, "/heaven/", 8) == 0)
    {
        snprintf(fullPath, BUFFER, "%s%s.enc", riildir, truePath);
    }
    else
    {
        return -ENOENT;
    }
    int res;

    res = unlink(fullPath);
    if (res == -1)
        return -errno;

    return 0;
}

static struct fuse_operations mai_ops = {
    .getattr = mai_getattr,
    .readdir = mai_readdir,
    .open = mai_open,
    .read = mai_read,
    .write = mai_write,
    .create = mai_create,
    .unlink = mai_unlink,
};

int main(int argc, char *argv[])
{
    umask(0);
    return fuse_main(argc, argv, &mai_ops, NULL);
}
