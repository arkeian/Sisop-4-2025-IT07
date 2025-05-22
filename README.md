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
![2](https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20213558.png)
![3](https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20213638.png)
![4](https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20213732.png)

## • Soal  2
## • Soal  3
Nafis dan Kimcun merupakan dua mahasiswa anomali😱 yang paling tidak tahu sopan santun dan sangat berbahaya di antara angkatan 24. Maka dari itu, Pujo sebagai komting yang baik hati dan penyayang😍, memutuskan untuk membuat sebuah sistem pendeteksi kenakalan bernama Anti Napis Kimcun (AntiNK) untuk melindungi file-file penting milik angkatan 24. Pujo pun kemudian bertanya kepada Asisten bagaimana cara membuat sistem yang benar, para asisten pun merespon (Author: Rafa / kookoon):

a.Pujo harus membuat sistem AntiNK menggunakan Docker yang menjalankan FUSE dalam container terisolasi. Sistem ini menggunakan docker-compose untuk mengelola container antink-server (FUSE Func.) dan antink-logger (Monitoring Real-Time Log). Asisten juga memberitahu bahwa docker-compose juga memiliki beberapa komponen lain yaitu
it24_host (Bind Mount -> Store Original File)
antink_mount (Mount Point)
antink-logs (Bind Mount -> Store Log)

b.Sistem harus mendeteksi file dengan kata kunci "nafis" atau "kimcun" dan membalikkan nama file tersebut saat ditampilkan. Saat file berbahaya (kimcun atau nafis) terdeteksi, sistem akan mencatat peringatan ke dalam log.
Ex: "docker exec [container-name] ls /antink_mount" 
Output: 
test.txt  vsc.sifan  txt.nucmik

c.Dikarenakan dua anomali tersebut terkenal dengan kelicikannya, Pujo mempunyai ide bahwa isi dari file teks normal akan di enkripsi menggunakan ROT13 saat dibaca, sedangkan file teks berbahaya tidak di enkripsi. 
Ex: "docker exec [container-name] cat /antink_mount/test.txt" 
Output: 
enkripsi teks asli

d.Semua aktivitas dicatat dengan ke dalam log file /var/log/it24.log yang dimonitor secara real-time oleh container logger.
![soal](https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20214414.png)

e.Semua perubahan file hanya terjadi di dalam container server jadi tidak akan berpengaruh di dalam direktori host. 

## penyelesaian
Buat dockerfile
```sh
FROM kalilinux/kali-rolling:latest

RUN apt-get update && apt-get install -y \
    fuse3 \
    libfuse3-dev \
    pkg-config \
    gcc \
    make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY antink.c /app/
RUN gcc -Wall antink.c -o antink -D_FILE_OFFSET_BITS=64 -lfuse3

RUN mkdir -p /antink_mount /it24_host /var/log/it24
VOLUME ["/it24_host", "/antink_mount", "/var/log/it24"]

CMD ["/app/antink", "/antink_mount", "-f"]
```
Buat docker-compse.yml
```sh
version: "3.9"

services:
  antink-server:
    build: .
    container_name: antink-server
    privileged: true
    volumes:
      - ./it24_host:/it24_host:ro
      - ./antink_mount:/antink_mount
      - ./antink-logs:/var/log/it24

  antink-logger:
    image: alpine:latest
    container_name: antink-logger
    command: sh -c "tail -F /var/log/it24/it24.log"
    volumes:
      - ./antink-logs:/var/log/it24
```
buat fuse antink.c
```sh

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
```
lakukan build up docker
```sh
docker-compose build
```
```sh
docker-compose up
```
setelah build up selesai lakukan
```sh
docker exec soal3_antink-server_1 ls -l /antink_moun
```
```sh
 docker exec -it soal3_antink-server_1 cat /antink_mount/test.txt
```

## dokumentasi soal4
![1](https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20220956.png)
![2](https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20221114.png)
![3](https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20222037.png)
![4](https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20222147.png)

## kendala soal4
fuse tidak bisa membaca file
kadang dockernya juga bertabrakan


## • Soal  4
## • Revisi
### • Revisi Soal 4

Sebelumnya Soal 4: Chiho memiliki kendala di mana suatu subdirektori yang berada di bawah naungan FUSE tidak dapat diakses. Alhasil, program FUSE gagal dalam membuat file baru yang nantinya akan dilakukan pengoperasian pada subsoal-subsoal Soal 4: Chiho. Pada revisi ini, program Soal 4: Chiho dapat membuat dan me-mounting sistem FUSE pada fuse_dir, mempopulasikan direktori fuse_dir dan chiho dengan subdirektori bawaan seperti starter, metro, heaven, dan sebagainya, menggandakan file yang dimasukkan ke dalam direktori virtual fuse_dir pada direktori chiho yang ada pada disk (Subsoal 4.A: Starter), serta melakukan beberapa pengoperasian subsoal yang diantaranya: 

1) Melakukan enkripsi dan dekripsi shifting berdasarkan offset pada file (Subsoal 4.B: Metro),
2) Melakukan enkripsi dan dekripsi menggunakan metode enkripsi ROT13 (Subsoal 4.C: Dragon),
3) Melakukan proses mengompres dan dekompres suatu file menggunakan zlib (Subsoal 4.F: Skystreet),
4) Melakukan enkripsi menggunakan metode enkripsi AES-256-CBC yang ada pada openssl (Subsoal 4.E: Heaven).

```c
#define FUSE_USE_VERSION 31
#define _GNU_SOURCE
#define _XOPEN_SOURCE 700
#define CHUNK 16384

#include <fuse.h>
#include <zlib.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

static char trueDir[PATH_MAX];

static void mai_true_path(const char *path, char *trpath) {
	snprintf(trpath, PATH_MAX, "%s%s", trueDir, path);
}

static void mai_huruf_berotasi(char *convbuf, size_t sz) {
	for (size_t i = 0; i < sz; i++) {
		if (convbuf[i] >= 'A' && convbuf[i] <= 'Z') {
			convbuf[i] = ((convbuf[i] - 'A' + 13) % 26) + 'A';
		}
		else if (convbuf[i] >= 'a' && convbuf[i] <= 'z') {
			convbuf[i] = ((convbuf[i] - 'a' + 13) % 26) + 'a';
		}
		else {
			convbuf[i] = convbuf[i];
		}
	}
}

static void mai_huruf_bergeser(char *convbuf, size_t sz, off_t ofst) {
	for (size_t i = 0; i < sz; i++) {
		if (convbuf[i] == '\0' || convbuf[i] == '\n') {
			continue;
		}
		convbuf[i] = convbuf[i] + ((ofst + i) % 256);
	}	
}

static void mai_huruf_anti_bergeser(char *convbuf, size_t sz, off_t ofst) {
	for (size_t i = 0; i < sz; i++) {
		if (convbuf[i] == '\0' || convbuf[i] == '\n') {
			continue;
		}
		convbuf[i] = convbuf[i] - ((ofst + i) % 256);
	}	
}

static int mai_berkas_berkompres(int fd, const char *buf, size_t sz, off_t ofst) {
	int ret;
    unsigned have;
    z_stream strm;
    unsigned char out[CHUNK];

	strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

	ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
	if (ret != Z_OK) {
		close(fd);
		return -errno;
	}

	strm.next_in = (Bytef *)buf;
    strm.avail_in = sz;

	do {
		strm.next_out = out;
		strm.avail_out = CHUNK;
		ret = deflate(&strm, Z_FINISH);

		if (ret == Z_STREAM_ERROR) {
			(void)deflateEnd(&strm);
			close(fd);
			return -errno;
		}
		have = CHUNK - strm.avail_out;

		if (pwrite(fd, out, have, ofst) != have) {
			(void)deflateEnd(&strm);
			close(fd);
			return -errno;
		}
	} while (strm.avail_out == 0);

	deflateEnd(&strm);
	return 0;
}

static int mai_berkas_anti_berkompres(int fd, const char *buf, size_t sz, off_t ofst) {
	int ret;
    unsigned have;
    z_stream strm;
    unsigned char out[CHUNK];

	strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
	strm.next_in = (Bytef *)buf;
    strm.avail_in = sz;

	ret = inflateInit2(&strm, 15 + 16);
	if (ret != Z_OK) {
		close(fd);
		return -errno;
	}


	do {
		strm.next_out = out;
		strm.avail_out = CHUNK;
		ret = inflate(&strm, Z_NO_FLUSH);

		if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
			(void)inflateEnd(&strm);
			close(fd);
			return -errno;
		}
		have = CHUNK - strm.avail_out;

		if (pwrite(fd, out, have, ofst) != have) {
			(void)inflateEnd(&strm);
			close(fd);
			return -errno;
		}
	} while (ret != Z_STREAM_END);

	inflateEnd(&strm);
	return 0;
}

static int mai_huruf_beraes(int fd, char *convbuf, size_t sz, off_t ofst) {
	unsigned char iv[16];
	RAND_bytes(iv, sizeof(iv));
	if (pwrite(fd, iv, sizeof(iv), ofst) != sizeof(iv)) {
		return -errno;
	}
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

	unsigned char key[32] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
								0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
								0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33,
								0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31};

	unsigned char out[sz + EVP_CIPHER_block_size(EVP_aes_256_cbc())];
	int len;
	EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);
	EVP_EncryptUpdate(ctx, out, &len, (const unsigned char *)convbuf, sz);
	int cipherLen = len;
	EVP_EncryptFinal_ex(ctx, out + len, &len);
	cipherLen += len;
	EVP_CIPHER_CTX_free(ctx);
	if (pwrite(fd, out, cipherLen, sizeof(iv) + ofst) != cipherLen) {
		return -errno;
	}

	return sz;
}

static const char *area[] = {
	"starter",
	"metro",
	"dragon",
	"blackrose",
	"skystreet",
	"heaven",
	"7sref",
	NULL
};

static int mknod_wrapper(int dirfd, const char *path, const char *link, int mode, dev_t rdev) {
	for (const char **d = area; *d; ++d) {
		char tmp[PATH_MAX];
		snprintf(tmp, sizeof(tmp), "/%s", *d);
		if (strncmp(path, tmp, strlen(tmp)) == 0 && (path[strlen(tmp)] == '/' || path[strlen(tmp)] == '\0')) {
			char trpath[PATH_MAX];
			mai_true_path(path, trpath);

			int res;

			if (S_ISREG(mode)) {
				res = openat(dirfd, trpath, O_CREAT | O_EXCL | O_WRONLY, mode & 0777);
				if (res >= 0) {
					res = close(res);
				}
			}
			else if (S_ISDIR(mode)) {
				res = mkdirat(dirfd, trpath, mode);
			}
			else if (S_ISLNK(mode) && link != NULL) {
				res = symlinkat(link, dirfd, trpath);
			}
			else if (S_ISFIFO(mode)) {
				res = mkfifoat(dirfd, trpath, mode);
			}
			else {
				res = mknodat(dirfd, trpath, mode, rdev);
			}

			return res;
		}
	}
	return -ENOENT;
}

static int fill_dir_plus = 0;

static void *xmp_init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
	(void) conn;
	cfg->use_ino = 1;
	cfg->parallel_direct_writes = 1;

	if (!cfg->auto_cache) {
		cfg->entry_timeout = 0;
		cfg->attr_timeout = 0;
		cfg->negative_timeout = 0;
	}

	return NULL;
}

static int xmp_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
	(void) fi;

	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		return 0;
	}

	for (const char **d = area; *d; ++d) {
		char tmp[PATH_MAX];
		snprintf(tmp, sizeof(tmp), "/%s", *d);
		if (strncmp(path, tmp, strlen(tmp)) == 0 && (path[strlen(tmp)] == '/' || path[strlen(tmp)] == '\0')) {
			char trpath[PATH_MAX];
			mai_true_path(path, trpath);
			
			int res;

			res = lstat(trpath, stbuf);
			if (res == -1) {
				return -errno;
			}

			return 0;
		}
	}
	return -ENOENT;
}

static int xmp_access(const char *path, int mask) {
	for (const char **d = area; *d; ++d) {
		char tmp[PATH_MAX];
		snprintf(tmp, sizeof(tmp), "/%s", *d);
		if (strncmp(path, tmp, strlen(tmp)) == 0 && (path[strlen(tmp)] == '/' || path[strlen(tmp)] == '\0')) {
			char trpath[PATH_MAX];
			mai_true_path(path, trpath);

			int res;

			res = access(trpath, mask);
			if (res == -1) {
				return -errno;
			}

			return 0;
		}
	}
	return -ENOENT;
}


static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
	(void) offset;
	(void) fi;
	(void) flags;

	char trpath[PATH_MAX];
	mai_true_path(path, trpath);

	DIR *dp;
	struct dirent *de;
	
	dp = opendir(trpath);
	if (dp == NULL) {
		return -errno;
	}

	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;

		filler(buf, de->d_name, &st, 0, 0);
	}
	closedir(dp);

	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev) {
	for (const char **d = area; *d; ++d) {
		char tmp[PATH_MAX];
		snprintf(tmp, sizeof(tmp), "/%s", *d);
		if (strncmp(path, tmp, strlen(tmp)) == 0 && (path[strlen(tmp)] == '/' || path[strlen(tmp)] == '\0')) {
			int res;

			res = mknod_wrapper(AT_FDCWD, path, NULL, mode, rdev);
			if (res == -1) {
				return -errno;
			}

			return 0;
		}
	}
	return -ENOENT;
}

static int xmp_mkdir(const char *path, mode_t mode) {
	for (const char **d = area; *d; ++d) {
		char tmp[PATH_MAX];
		snprintf(tmp, sizeof(tmp), "/%s", *d);
		if (strncmp(path, tmp, strlen(tmp)) == 0 && (path[strlen(tmp)] == '/' || path[strlen(tmp)] == '\0')) {
			char trpath[PATH_MAX];
			mai_true_path(path, trpath);

			int res;
			
			res = mkdir(trpath, mode);
			if (res == -1) {
				return -errno;
			}

			return 0;
		}
	}
	return -ENOENT;
}

static int xmp_rmdir(const char *path) {
	for (const char **d = area; *d; ++d) {
		char tmp[PATH_MAX];
		snprintf(tmp, sizeof(tmp), "/%s", *d);
		if (strncmp(path, tmp, strlen(tmp)) == 0 && (path[strlen(tmp)] == '/' || path[strlen(tmp)] == '\0')) {
			char trpath[PATH_MAX];
			mai_true_path(path, trpath);

			int res;

			res = rmdir(trpath);
			if (res == -1) {
				return -errno;
			}

			return 0;
		}
	}
	return -ENOENT;
}

static int xmp_utimens(const char *path, const struct timespec ts[2], struct fuse_file_info *fi) {
	(void) fi;

	for (const char **d = area; *d; ++d) {
		char tmp[PATH_MAX];
		snprintf(tmp, sizeof(tmp), "/%s", *d);
		if (strncmp(path, tmp, strlen(tmp)) == 0 && (path[strlen(tmp)] == '/' || path[strlen(tmp)] == '\0')) {
			char trpath[PATH_MAX];
			mai_true_path(path, trpath);

			int res;

			res = utimensat(0, trpath, ts, AT_SYMLINK_NOFOLLOW);
			if (res == -1) {
				return -errno;
			}

			return 0;
		}
	}
	return -ENOENT;
}

static int xmp_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	for (const char **d = area; *d; ++d) {
		char tmp[PATH_MAX];
		snprintf(tmp, sizeof(tmp), "/%s", *d);
		if (strncmp(path, tmp, strlen(tmp)) == 0 && (path[strlen(tmp)] == '/' || path[strlen(tmp)] == '\0')) {
			char trpath[PATH_MAX];
			mai_true_path(path, trpath);

			int res;

			res = open(trpath, fi->flags | O_CREAT, mode & 0777);

			if (res == -1) {
				return -errno;
			}

			if (fi->flags & O_DIRECT) {
				fi->direct_io = 1;
				fi->parallel_direct_writes = 1;
			}

			fi->fh = res;
			return 0;
		}
	}
	return -ENOENT;
}

static int xmp_open(const char *path, struct fuse_file_info *fi) {
	for (const char **d = area; *d; ++d) {
		char tmp[PATH_MAX];
		snprintf(tmp, sizeof(tmp), "/%s", *d);
		if (strncmp(path, tmp, strlen(tmp)) == 0 && (path[strlen(tmp)] == '/' || path[strlen(tmp)] == '\0')) {
			char trpath[PATH_MAX];
			mai_true_path(path, trpath);

			int res;

			res = open(trpath, fi->flags);
			if (res == -1) {
				return -errno;
			}

			if (fi->flags & O_DIRECT) {
				fi->direct_io = 1;
				fi->parallel_direct_writes = 1;
			}

			fi->fh = res;
			return 0;
		}
	}
	return -ENOENT;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	for (const char **d = area; *d; ++d) {
		char tmp[PATH_MAX];
		snprintf(tmp, sizeof(tmp), "/%s", *d);
		if (strncmp(path, tmp, strlen(tmp)) == 0 && (path[strlen(tmp)] == '/' || path[strlen(tmp)] == '\0')) {
			char trpath[PATH_MAX];
			mai_true_path(path, trpath);

			int fd;
			int res;

			if(fi == NULL) {
				fd = open(trpath, O_RDONLY);
			}
			else {
				fd = fi->fh;
			}
			
			if (fd == -1) {
				return -errno;
			}

			if (strstr(trpath, "skystreet")) {
				res = mai_berkas_anti_berkompres(fd, buf, size, 0);
			}
			else {
				res = pread(fd, buf, size, offset);
			}
			if (res == -1) {
				res = -errno;
			}
			else if (res != -1 && strstr(trpath, "dragon")) {
				mai_huruf_berotasi(buf, res);
			}
			else if (res != -1 && strstr(trpath, "metro")) {
				mai_huruf_anti_bergeser(buf, res, offset);
			}

			if(fi == NULL) {
				close(fd);
			}

			return res;
		}
	}
	return -ENOENT;
}

static int xmp_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	for (const char **d = area; *d; ++d) {
		char tmp[PATH_MAX];
		snprintf(tmp, sizeof(tmp), "/%s", *d);
		if (strncmp(path, tmp, strlen(tmp)) == 0 && (path[strlen(tmp)] == '/' || path[strlen(tmp)] == '\0')) {
			char trpath[PATH_MAX];
			mai_true_path(path, trpath);

			int fd;
			int res;

			(void) fi;
			if(fi == NULL) {
				fd = open(trpath, O_WRONLY);
			}
			else {
				fd = fi->fh;
			}
			if (fd == -1) {
				return -errno;
			}

			if (strstr(trpath, "dragon")) {
				char *convbuf = malloc(size);
				memcpy(convbuf, buf, size);
				mai_huruf_berotasi(convbuf, size);
				res = pwrite(fd, convbuf, size, offset);
				free(convbuf);
			}
			else if (strstr(trpath, "metro")) {
				char *convbuf = malloc(size);
				memcpy(convbuf, buf, size);
				mai_huruf_bergeser(convbuf, size, offset);
				res = pwrite(fd, convbuf, size, offset);
				free(convbuf);
			}
			else if (strstr(trpath, "skystreet")) {
				res = lseek(fd, 0, SEEK_END);
				
				if (res == -1) {
					res = -errno;
				}

				if (fi == NULL) {
					close(fd);
				}

				off_t end = res;

				res = mai_berkas_berkompres(fd, buf, size, end);
				if (res != 0) {
					res = -errno;
				}

				if (fi == NULL) {
					close(fd);
				}
				res = size;
			}
			else if (strstr(trpath, "heaven")) {
				char *convbuf = malloc(size);
				memcpy(convbuf, buf, size);
				res = mai_huruf_beraes(fd, convbuf, size, offset);
				free(convbuf);
			}
			else {
				res = pwrite(fd, buf, size, offset);
			}
			if (res == -1) {
				res = -errno;
			}

			if (fi == NULL) {
				close(fd);
			}
			return res;
		}
	}
	return -ENOENT;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf) {
	char trpath[PATH_MAX];
	mai_true_path(path, trpath);
	int res;

	res = statvfs(trpath, stbuf);
	if (res == -1) {
		return -errno;
	}

	return 0;
}

static int xmp_release(const char *path, struct fuse_file_info *fi) {
	(void) path;
	close(fi->fh);
	return 0;
}

static int xmp_fsync(const char *path, int isdatasync, struct fuse_file_info *fi) {
	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}

static off_t xmp_lseek(const char *path, off_t off, int whence, struct fuse_file_info *fi) {
	for (const char **d = area; *d; ++d) {
		char tmp[PATH_MAX];
		snprintf(tmp, sizeof(tmp), "/%s", *d);
		if (strncmp(path, tmp, strlen(tmp)) == 0 && (path[strlen(tmp)] == '/' || path[strlen(tmp)] == '\0')) {
			char trpath[PATH_MAX];
			mai_true_path(path, trpath);

			int fd;
			off_t res;

			if (fi == NULL) {
				fd = open(trpath, O_RDONLY);
			}
			else {
				fd = fi->fh;
			}

			if (fd == -1) {
				return -errno;
			}

			res = lseek(fd, off, whence);
			if (res == -1) {
				res = -errno;
			}

			if (fi == NULL) {
				close(fd);
			}
			return res;
		}
	}
	return -ENOENT;
}

static const struct fuse_operations xmp_oper = {
	.init       = xmp_init,
	.getattr	= xmp_getattr,
	.access		= xmp_access,
	.readdir	= xmp_readdir,
	.mknod		= xmp_mknod,
	.mkdir		= xmp_mkdir,
	.rmdir		= xmp_rmdir,
	.utimens	= xmp_utimens,
	.open		= xmp_open,
	.create 	= xmp_create,
	.read		= xmp_read,
	.write		= xmp_write,
	.statfs		= xmp_statfs,
	.release	= xmp_release,
	.fsync		= xmp_fsync,
	.lseek		= xmp_lseek,
};

void mai_filldir(int dirfd) {
	for (const char **d = area; *d; ++d) {
		if (mkdirat(dirfd, *d, 0755) == -1) {
			if (errno != EEXIST) {
				fprintf(stderr, "%s \n", strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
	}
}

int main(int argc, char *argv[]) {
	const char *mountpoint = argv[1];
	char cwd[PATH_MAX];

	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		return -errno;
	}

	int mnt = open(mountpoint, O_RDONLY | O_DIRECTORY);
	if (mnt == -1) {
		return -errno;
	}

	snprintf(trueDir, sizeof(trueDir), "%s/%s", cwd, "chiho");

	int trd = open(trueDir, O_RDONLY | O_DIRECTORY);
	if (trd == -1) {
		return -errno;
	}
	
	if (chdir(mountpoint) == -1) {
		return -errno;
	}

	mai_filldir(mnt);
	mai_filldir(trd);
	
	close(mnt);
	close(trd);

	argv[1] = ".";

	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}
```
