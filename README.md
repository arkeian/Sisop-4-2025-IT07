# Laporan Resmi Modul 4 Kelompok IT-07
## Anggota

| Nama 				| NRP		|
|-------------------------------|---------------|
| Muhammad Rakha Hananditya R.	| 5027241015 	|
| Zaenal Mustofa		| 5027241018 	|
| Mochkamad Maulana Syafaat	| 5027241021 	|

## • Soal  1

The Shorekeeper adalah sebuah entitas misterius yang memimpin dan menjaga Black Shores secara keseluruhan. Karena Shorekeeper hanya berada di Black Shores, ia biasanya berjalan - jalan di sekitar Black Shores untuk mencari anomali - anomali yang ada untuk mencegah adanya kekacauan ataupun krisis di Black Shores. Semenjak kemunculan Fallacy of No Return, ia semakin ketat dalam melakukan pencarian anomali - anomali yang ada di Black Shores untuk mencegah hal yang sama terjadi lagi.
Suatu hari, saat di Tethys' Deep, Shorekeeper menemukan sebuah anomali yang baru diketahui. Anomali ini berupa sebuah teks acak yang kelihatannya tidak memiliki arti. Namun, ia mempunyai ide untuk mencari arti dari teks acak tersebut. [Author: Haidar / scar / hemorrhager / 恩赫勒夫]


a.Pertama, Shorekeeper akan mengambil beberapa sampel anomali teks dari [link berikut](https://drive.google.com/file/d/1hi_GDdP51Kn2JJMw02WmCOxuc3qrXzh5/view). Pastikan file zip terhapus setelah proses unzip.


b.Setelah melihat teks - teks yang didapatkan, ia menyadari bahwa format teks tersebut adalah hexadecimal. Dengan informasi tersebut, Shorekeeper mencoba untuk mencoba idenya untuk mencari makna dari teks - teks acak tersebut, yaitu dengan mengubahnya dari string hexadecimal menjadi sebuah file image. Bantulah Shorekeeper dengan membuat kode untuk FUSE yang dapat mengubah string hexadecimal menjadi sebuah gambar ketika file text tersebut dibuka di mount directory. Lalu, letakkan hasil gambar yang didapat ke dalam directory bernama “image”.

c.Untuk penamaan file hasil konversi dari string ke image adalah [nama file string]_image_[YYYY-mm-dd]_[HH:MM:SS].
Contoh:
1_image_2025-05-11_18:35:26.png

d.Catat setiap konversi yang ada ke dalam sebuah log file bernama conversion.log. Untuk formatnya adalah sebagai berikut.
[YYYY-mm-dd][HH:MM:SS]: Successfully converted hexadecimal text [nama file string] to [nama file image].

![contoh](https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20211632.png)


![contoh](https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20211744.png)

## Penyelesaian
download dan unzip 
```sh
wget -O anomali.zip "https://drive.usercontent.google.com/u/0/uc?id=1hi_GDdP51Kn2JJMw02WmCOxuc3qrXzh5&export=download" && unzip -o anomali.zip

```

membuat fuse hexed.c
```sh

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

```
Compile file hexed.c
```sh
gcc -Wall hexed.c -o hexed $(pkg-config fuse --cflags --libs)

```
Buat file mount_point
```sh
mkdir -p mnt
```
jalankan fuse dengan command
```sh
./hexed -f mnt
```

baca file txt dengan
```sh
cat mnt/1.txt dst..
```

untuk mematikan fuse gunakan
```sh
fusermount -u mnt
```

## dokumentasi soal1
![1](https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20213350.png)
1[2](https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20213558.png)
![3](https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20213638.png)
1[4](https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20213732.png)

## • Soal  2
## • Soal  3
## • Soal  4
## • Revisi
