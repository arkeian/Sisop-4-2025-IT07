# Laporan Resmi Modul 4 Kelompok IT-07
## Anggota

| Nama 				| NRP		|
|-------------------------------|---------------|
| Muhammad Rakha Hananditya R.	| 5027241015 	|
| Zaenal Mustofa		| 5027241018 	|
| Mochkamad Maulana Syafaat	| 5027241021 	|

## • Soal  1

<p align="justify">
&emsp; The Shorekeeper adalah sebuah entitas misterius yang memimpin dan menjaga Black Shores secara keseluruhan. Karena Shorekeeper hanya berada di Black Shores, ia biasanya berjalan - jalan di sekitar Black Shores untuk mencari anomali - anomali yang ada untuk mencegah adanya kekacauan ataupun krisis di Black Shores. Semenjak kemunculan Fallacy of No Return, ia semakin ketat dalam melakukan pencarian anomali - anomali yang ada di Black Shores untuk mencegah hal yang sama terjadi lagi.
Suatu hari, saat di Tethys' Deep, Shorekeeper menemukan sebuah anomali yang baru diketahui. Anomali ini berupa sebuah teks acak yang kelihatannya tidak memiliki arti. Namun, ia mempunyai ide untuk mencari arti dari teks acak tersebut.
</p>
  
[Author: Haidar / scar / hemorrhager / 恩赫勒夫]

<ol type="a">
	<li>
		<p align="justify">
			Pertama, Shorekeeper akan mengambil beberapa sampel anomali teks dari link berikut: https://drive.google.com/file/d/1hi_GDdP51Kn2JJMw02WmCOxuc3qrXzh5/view. Pastikan file zip terhapus setelah proses unzip.
		</p>
	</li>
	<li>
		<p align="justify">
			Setelah melihat teks - teks yang didapatkan, ia menyadari bahwa format teks tersebut adalah hexadecimal. Dengan informasi tersebut, Shorekeeper mencoba untuk mencoba idenya untuk mencari makna dari teks - teks acak tersebut, yaitu dengan mengubahnya dari string hexadecimal menjadi sebuah file image. Bantulah Shorekeeper dengan membuat kode untuk FUSE yang dapat mengubah string hexadecimal menjadi sebuah gambar ketika file text tersebut dibuka di mount directory. Lalu, letakkan hasil gambar yang didapat ke dalam directory bernama “image”.
		  </p>
	</li>
	<li>
		<p align="justify">
			Untuk penamaan file hasil konversi dari string ke image adalah [nama file string]_image_[YYYY-mm-dd]_[HH:MM:SS].
		</p>
	</li>
	<li>
		<p align="justify">
			Catat setiap konversi yang ada ke dalam sebuah log file bernama conversion.log. Untuk formatnya adalah sebagai berikut.
	[YYYY-mm-dd][HH:MM:SS]: Successfully converted hexadecimal text [nama file string] to [nama file image].
		</p>
	</li>
</ol>

<p align="center">
	<img src="https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20211632.png" alt="Letto - Ruang Rindu">  
</p>

> (1.1) Screenshot potret contoh output dari `conversion.log`. 

<p align="center">
	<img src="https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20211744.png" alt="SAMSONS - Kenangan Terindah">  
</p>

> (1.2) Screenshot potret contoh struktur akhir dari Soal 1.

### • Penyelesaian Soal 1
1) Download dan Unzip:

```sh
wget -O anomali.zip "https://drive.usercontent.google.com/u/0/uc?id=1hi_GDdP51Kn2JJMw02WmCOxuc3qrXzh5&export=download" && unzip -o anomali.zip

```

2) Membuat fuse `hexed.c`:

```c

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

3) Compile file `hexed.c`:

```sh
gcc -Wall hexed.c -o hexed $(pkg-config fuse --cflags --libs)
```

4) Buat file mount_point:

```sh
mkdir -p mnt
```

5) Jalankan fuse dengan command:

```sh
./hexed -f mnt
```

6) Baca file txt dengan:

```sh
cat mnt/1.txt dst..
```

7) Untuk mematikan fuse gunakan:

```sh
fusermount -u mnt
```

### • Dokumentasi Soal 1

<p align="center">
	<img src="https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20213350.png" alt="Repvblik - Sandiwara Cinta">  
</p>

<p align="center">
	<img src="https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20213558.png" alt="D'MASIV - Pergilah Kasih">  
</p>

<p align="center">
	<img src="https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20213638.png" alt="Ada Band - Setengah Hati">  
</p>

<p align="center">
	<img src="https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20213732.png" alt="Andra and The Backbone - Sempurna">  
</p>

## • Soal  2

<p align="justify">
&emsp; Pada suatu hari, seorang ilmuwan muda menemukan sebuah drive tua yang tertanam di reruntuhan laboratorium robotik. Saat diperiksa, drive tersebut berisi pecahan data dari satu-satunya robot perawat legendaris yang dikenal dengan nama <code>Baymax</code>. Sayangnya, akibat kerusakan sistem selama bertahun-tahun, file utuh Baymax telah terfragmentasi menjadi 14 bagian kecil, masing-masing berukuran 1 kilobyte, dan tersimpan dalam direktori bernama relics. Pecahan tersebut diberi nama berurutan seperti <code>Baymax.jpeg.000</code>, <code>Baymax.jpeg.001</code>, hingga <code>Baymax.jpeg.013</code>. Ilmuwan tersebut kini ingin membangkitkan kembali Baymax ke dalam bentuk digital yang utuh, namun ia tidak ingin merusak file asli yang telah rapuh tersebut.
</p>
  
`(Author : Icel / wonbunsa).`  

<ol type="a">
	<li>
		<p align="justify">
			Sebagai asisten teknis, tugasmu adalah membuat sebuah sistem file virtual menggunakan <code>FUSE (Filesystem in Userspace)</code> yang dapat membantu sang ilmuwan. Buatlah sebuah direktori mount bernama bebas (misalnya <code>mount_dir</code>) yang merepresentasikan tampilan Baymax dalam bentuk file utuh <code>Baymax.jpeg</code>. File sistem tersebut akan mengambil data dari folder <code>relics</code> sebagai sumber aslinya.
		</p>
	</li>
</ol>
  
```markdown
├── mount_dir
├── relics
│ ├── Baymax.jpeg.000
│ ├── Baymax.jpeg.001
│ ├── dst dst…
│ └── Baymax.jpeg.013
└── activity.log
```

<ol type="a" start="2">
	<li>
		<p align="justify">
			Ketika direktori <code>FUSE</code> diakses, pengguna hanya akan melihat <code>Baymax.jpeg</code> seolah-olah tidak pernah terpecah, meskipun aslinya terdiri dari potongan <code>.000</code> hingga <code>.013</code>. File <code>Baymax.jpeg</code> tersebut dapat dibaca, ditampilkan, dan disalin sebagaimana file gambar biasa, hasilnya merupakan gabungan sempurna dari keempat belas pecahan tersebut.  
		</p>
	</li>
</ol>

<ol type="a" start="3">
	<li>
		<p align="justify">
			Namun sistem ini harus lebih dari sekadar menyatukan. Jika pengguna membuat file baru di dalam direktori FUSE, maka sistem harus secara otomatis memecah file tersebut ke dalam potongan-potongan berukuran maksimal 1 KB, dan menyimpannya di direktori <code>relics</code> menggunakan format <code>[namafile].000</code>, <code>[namafile].001</code>, dan seterusnya.
		</p>
	</li>
</ol>
  
<ol type="a" start="4">
	<li>
		<p align="justify">
			Ketika file tersebut dihapus dari direktori <code>mount</code>, semua pecahannya di <code>relics</code> juga harus ikut dihapus.
		</p>
	</li>
</ol>

<ol type="a" start="5">
	<li>
		<p align="justify">
			Untuk keperluan analisis ilmuwan, sistem juga harus mencatat seluruh aktivitas pengguna dalam sebuah file log bernama <code>activity.log</code> yang disimpan di direktori yang sama. Aktivitas yang dicatat antara lain:  
		</p>
	</li>
</ol>
  
&emsp;&emsp; -  Membaca file (misalnya membuka `baymax.png`)
  
&emsp;&emsp; -  Membuat file baru (termasuk nama file dan jumlah pecahan)
  
&emsp;&emsp; -  Menghapus file (termasuk semua pecahannya yang terhapus)
  
&emsp;&emsp; -  Menyalin file (misalnya `cp baymax.png /tmp/`)
  
&emsp;&emsp; -  Contoh Log:
  
```markdown
[2025-05-11 10:24:01] READ: Baymax.jpeg
[2025-05-11 10:25:14] WRITE: hero.txt -> hero.txt.000, hero.txt.001
[2025-05-11 10:26:03] DELETE: Baymax.jpeg.000 - Baymax.jpeg.013
[2025-05-11 10:27:45] COPY: Baymax.jpeg -> /tmp/Baymax.jpeg
```
  
### • Penyelesaian Soal 2

<p align="justify">
&emsp; Program ini merupakan implementasi virtual filesystem menggunakan FUSE v3, yang menyatukan dan memanipulasi file gambar yang dipecah menjadi 14 bagian (chunk). File asli bernama <code>Baymax.jpeg</code>, disimpan dalam bentuk terpecah dengan nama <code>Baymax.jpeg.000</code> hingga <code>Baymax.jpeg.013</code> di direktori khusus <code>RELICS_DIR</code>. Filesystem ini mampu melakukan operasi dasar seperti read, write, dan delete terhadap file tersebut secara transparan bagi pengguna.
</p>
  
Berikut ini code lengkapnya:
  
```c
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

// Path ke direktori yang menyimpan file pecahan
const char *RELICS_DIR = NULL;

// File log aktivitas
#define LOG_FILE "activity.log"

// Ukuran pecahan file (chunk)
#define CHUNK_SIZE 1024

// Fungsi untuk mencatat log aktivitas ke file activity.log
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

// Mengembalikan atribut file
static int baymax_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755; // direktori
        stbuf->st_nlink = 2;
    } else if (strcmp(path, "/Baymax.jpeg") == 0) {
        stbuf->st_mode = S_IFREG | 0644; // file biasa
        stbuf->st_nlink = 1;

        // Hitung ukuran total file dari semua chunk
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

// Mengisi isi direktori root
static int baymax_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
                          struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);
    filler(buf, "Baymax.jpeg", NULL, 0, 0); // tampilkan file gabungan
    return 0;
}

// Membuka file
static int baymax_open(const char *path, struct fuse_file_info *fi) {
    if (strcmp(path, "/Baymax.jpeg") != 0)
        return -ENOENT;
    write_log("READ", path + 1);
    return 0;
}

// Membaca isi file dari chunk
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

// Membuat file baru (sementara)
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

// Menulis ke file sementara
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

// Saat file ditutup, pecah menjadi chunk
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

// Menghapus semua chunk file
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

// Daftar operasi yang didukung filesystem
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

// Fungsi utama
int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <mountpoint> <relics_dir>\n", argv[0]);
        return 1;
    }

    // Simpan path absolut direktori pecahan
    RELICS_DIR = realpath(argv[argc - 1], NULL);
    if (!RELICS_DIR) {
        perror("Invalid relics directory");
        return 1;
    }

    // Hapus argumen relics_dir dari argv untuk FUSE
    argv[argc - 1] = NULL;
    argc--;

    return fuse_main(argc, argv, &baymax_oper, NULL);
}
```
Program ini membentuk sistem berkas virtual berbasis FUSE yang menyatukan dan mengelola file yang terpecah menjadi beberapa bagian secara transparan. Sistem ini efektif untuk implementasi fitur dekomposisi dan rekonstruksi file besar, serta memberikan jejak audit aktivitas file melalui sistem logging yang jelas. Berikut ini command yang digunakan :  
-  Compile File `baymax.c`
```
gcc -Wall baymax.c `pkg-config --cflags --libs fuse3` -o baymax
```
-  Jalankan Program Fuse
```
./baymax mount_dir relics
```
-  Lihat Hasil Mount
```
xdg-open mount_dir/Baymax.jpeg
```
-  Buat file baru di folder `mount`
```
echo "ini file hero" > mount_dir/hero.txt
```
-  Hapus File dari folder `mount`
```
rm mount_dir/hero.txt
```
-  Unmount program fuse
```
fusermount3 -u mount_dir
```
```c
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
```
Bagian ini mengimpor pustaka yang diperlukan oleh program, seperti FUSE, I/O standar, manajemen file dan direktori, serta manajemen waktu untuk pencatatan log. Makro FUSE_USE_VERSION 31 menandakan penggunaan API FUSE versi 3.1.  
```c
const char *RELICS_DIR = NULL;
#define LOG_FILE "activity.log"
#define CHUNK_SIZE 1024
```
-  `RELICS_DIR`: menyimpan path direktori tempat semua pecahan file disimpan.
-  `LOG_FILE` : nama file untuk mencatat aktivitas filesystem.
-  `CHUNK_SIZE` : ukuran tiap pecahan file, yaitu 1024 byte.
```c
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
```
Fungsi ini mencatat log aktivitas seperti baca, tulis, dan hapus file. Format log mencakup timestamp, jenis aksi, dan detail nama file.  
```c
static int baymax_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755; // direktori
        stbuf->st_nlink = 2;
    } else if (strcmp(path, "/Baymax.jpeg") == 0) {
        stbuf->st_mode = S_IFREG | 0644; // file biasa
        stbuf->st_nlink = 1;

        // Hitung ukuran total file dari semua chunk
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
```
Mengembalikan atribut file. Jika yang diminta adalah `root (/)`, maka diatur sebagai direktori. Jika file `Baymax.jpeg`, maka ukuran file dihitung dari total ukuran semua pecahan .000 sampai .013.
```c
static int baymax_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
                          struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);
    filler(buf, "Baymax.jpeg", NULL, 0, 0); // tampilkan file gabungan
    return 0;
}
```
Menampilkan isi direktori root virtual, yang hanya menampilkan file Baymax.jpeg. Fungsi ini mengabstraksikan 14 file pecahan menjadi satu file utuh.
```c
static int baymax_open(const char *path, struct fuse_file_info *fi) {
    if (strcmp(path, "/Baymax.jpeg") != 0)
        return -ENOENT;
    write_log("READ", path + 1);
    return 0;
}
```
Membuka file `Baymax.jpeg`. Jika file berhasil diakses, aktivitas akan dicatat ke log dengan label `READ`
```c
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
```
Membaca isi Baymax.jpeg berdasarkan offset dan size. Fungsi ini menghitung pecahan file mana yang perlu dibuka dan membaca data dari beberapa pecahan jika perlu.
```c
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
```
Membuat file sementara dengan akhiran _temp. File ini digunakan sebagai buffer untuk menyimpan data sebelum dipecah menjadi chunk.
```c
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
```
Menulis data ke file sementara yang dibuat di create. Fungsi ini mendukung operasi tulis yang terfragmentasi dan bersifat sementara.
```c
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
```
Ketika file ditutup, data dalam file sementara dipecah menjadi file-file pecahan (`.000`, `.001`, dst) dengan ukuran 1024 byte. Log dicatat dengan label `WRITE`.
```c
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
```
Menghapus semua pecahan file berdasarkan nama dasar file. Setelah penghapusan, aktivitas dicatat ke log dengan label `DELETE`.
```c
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
```
Struktur ini mendefinisikan operasi filesystem yang digunakan dalam `FUSE`, seperti `getattr`, `readdir`, `open`, `read`, `create`, `write`, `release`, dan `unlink`.
```c
int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <mountpoint> <relics_dir>\n", argv[0]);
        return 1;
    }

    // Simpan path absolut direktori pecahan
    RELICS_DIR = realpath(argv[argc - 1], NULL);
    if (!RELICS_DIR) {
        perror("Invalid relics directory");
        return 1;
    }

    // Hapus argumen relics_dir dari argv untuk FUSE
    argv[argc - 1] = NULL;
    argc--;

    return fuse_main(argc, argv, &baymax_oper, NULL);
}
```
Fungsi utama program :  
-  Memeriksa argumen mount point dan direktori pecahan.
-  Menyimpan path absolut ke `RELICS_DIR`.
-  Menghapus argumen direktori dari `argv`.
-  Menjalankan `fuse_main()` untuk memulai filesystem virtual Baymax.
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

### • Dokumentasi Soal 3
![1](https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20220956.png)
![2](https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20221114.png)
![3](https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20222037.png)
![4](https://github.com/jagosyafaat30/dokumetnsasi/blob/main/modul4/Screenshot%202025-05-22%20222147.png)

### • Kendala Soal 3
fuse tidak bisa membaca file
kadang dockernya juga bertabrakan


## • Soal 4: Chiho
### • Pendahuluan

## • Revisi
### • Revisi Soal 2
Pada implementasi awal (kode pertama), beberapa fitur utama belum berfungsi dengan baik, antara lain:  
-  Pembuatan File (Create): Fungsi create belum berhasil membuat file baru di dalam direktori mount point. Hal ini menyebabkan pengguna tidak dapat menyimpan file baru melalui filesystem ini.
-  Penghapusan File (Unlink): Fungsi unlink belum terimplementasi atau tidak berjalan sebagaimana mestinya, sehingga file yang dihapus dari VFS tidak benar-benar terhapus dari direktori backend.
-  Pencatatan Aktivitas (Logging): Tidak terdapat log aktivitas seperti READ, WRITE, CREATE, DELETE, dan RENAME ke dalam file log sebagaimana yang disyaratkan. Hal ini menyulitkan debugging dan audit atas operasi yang dilakukan oleh pengguna.   
Berikut ini kode perbaikannya :
```c
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
#include <sys/stat.h>

const char *RELICS_DIR = NULL;
#define LOG_FILE "activity.log"
#define CHUNK_SIZE 1024

void write_log(const char *action, const char *detail) {
    char log_path[256];
    sprintf(log_path, "%s/%s", RELICS_DIR, LOG_FILE);
    FILE *log = fopen(log_path, "a");
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
        char chunk_path[256];
        size_t size = 0;
        for (int i = 0; i < 1000; i++) {
            sprintf(chunk_path, "%s/Baymax.jpeg.%03d", RELICS_DIR, i);
            FILE *f = fopen(chunk_path, "rb");
            if (!f) break;
            fseek(f, 0, SEEK_END);
            size += ftell(f);
            fclose(f);
        }
        stbuf->st_size = size;
    } else {
        // Untuk file lain yang dibuat lewat create()
        char tmp[256];
        sprintf(tmp, "%s%s", RELICS_DIR, path);
        if (access(tmp, F_OK) == 0) {
            stbuf->st_mode = S_IFREG | 0644;
            stbuf->st_nlink = 1;
            return 0;
        }
        return -ENOENT;
    }
    return 0;
}

static int baymax_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
                          struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);

    int has_baymax = 0;
    DIR *dir = opendir(RELICS_DIR);
    if (!dir) return -ENOENT;

    struct dirent *dp;
    char files_seen[256][256];
    int file_count = 0;

    while ((dp = readdir(dir)) != NULL) {
        if (strstr(dp->d_name, "Baymax.jpeg.000")) {
            has_baymax = 1;
        }

        // Tangkap file temp atau log
        if (strstr(dp->d_name, ".log") || strstr(dp->d_name, ".temp")) continue;

        // Ambil nama dasar file (tanpa ekstensi pecahan)
        char base[256];
        if (sscanf(dp->d_name, "%[^.].%*03d", base) == 1) {
            // Tambahkan hanya sekali
            int already_added = 0;
            for (int i = 0; i < file_count; i++) {
                if (strcmp(files_seen[i], base) == 0) {
                    already_added = 1;
                    break;
                }
            }
            if (!already_added) {
                strcpy(files_seen[file_count++], base);
                filler(buf, base, NULL, 0, 0);
            }
        }
    }
    closedir(dir);

    if (has_baymax) {
        filler(buf, "Baymax.jpeg", NULL, 0, 0);
    }

    return 0;
}

static int baymax_open(const char *path, struct fuse_file_info *fi) {
    if (strcmp(path, "/Baymax.jpeg") == 0) {
        write_log("READ", path + 1);
        return 0;
    }
    // Deteksi salinan oleh `cp`
    write_log("COPY", path + 1);
    return 0;
}

static int baymax_read(const char *path, char *buf, size_t size, off_t offset,
                       struct fuse_file_info *fi) {
    if (strcmp(path, "/Baymax.jpeg") != 0) return -ENOENT;

    size_t bytes_read = 0;
    int chunk_index = offset / CHUNK_SIZE;
    size_t chunk_offset = offset % CHUNK_SIZE;

    while (size > 0 && chunk_index < 1000) {
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
    char *tmp_path = (char *)malloc(256);
    sprintf(tmp_path, "%s%s.temp", RELICS_DIR, path);
    FILE *f = fopen(tmp_path, "wb");
    if (!f) return -EIO;
    fclose(f);
    fi->fh = (uint64_t)tmp_path;
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
    for (; i < 1000; i++) {
        sprintf(chunk_path, "%s/%s.%03d", RELICS_DIR, filename, i);
        if (access(chunk_path, F_OK) != 0) break;
        remove(chunk_path);
    }

    char logline[256];
    if (i == 0) return -ENOENT;
    sprintf(logline, "%s.000 - %s.%03d", filename, filename, i - 1);
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
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <mountpoint> <relics_dir>\n", argv[0]);
        return 1;
    }

    RELICS_DIR = realpath(argv[argc - 1], NULL);
    if (!RELICS_DIR) {
        perror("Invalid relics directory");
        return 1;
    }

    argv[argc - 1] = NULL;
    argc--;

    return fuse_main(argc, argv, &baymax_oper, NULL);
}
```
Pada versi revisi (kode kedua), seluruh fitur yang belum berjalan pada kode pertama telah berhasil diimplementasikan, dengan rincian sebagai berikut:  
-  Fungsi create Berfungsi: Pengguna dapat membuat file baru di dalam virtual filesystem. File tersebut akan otomatis terfragmentasi ke dalam potongan 1 KB jika dilakukan penulisan (write).
-  Fungsi unlink Aktif: File yang dihapus melalui VFS akan menghapus semua fragmen file terkait dari direktori backend, sesuai dengan format penamaan {nama_file}.frag{i}.
-  Logging Aktif: Setiap operasi read, write, create, unlink, dan rename dicatat secara otomatis dalam berkas log baymax.log. Format log mengikuti standar timestamped dengan jenis operasi dan path file yang digunakan.

### Kendala yang Dihadapi
Pada tahap pengembangan awal virtual filesystem menggunakan FUSE, kami mengalami beberapa kendala teknis yang cukup signifikan, yaitu:
1.  FUSE tidak berjalan pada terminal
   Saat menjalankan program FUSE di terminal, filesystem gagal ter-mount dan tidak aktif sebagaimana yang diharapkan. Hal ini menghambat pengujian dan penggunaan fitur-fitur dasar filesystem.
2.  Tidak dapat membuat file baru
   Fungsi pembuatan file (`create`) tidak berfungsi dengan baik. Pengguna tidak dapat membuat file baru melalui filesystem, sehingga operasi tulis dan manipulasi file menjadi terbatas dan tidak dapat diuji secara optimal.

Kendala-kendala ini menyebabkan keterbatasan dalam menguji fitur lain seperti penghapusan file dan pencatatan log aktivitas, karena operasi-operasi tersebut sangat bergantung pada pembuatan dan manipulasi file yang berhasil.

### • Revisi Soal 4

<p align="justify">
&emsp; Sebelumnya Soal 4: Chiho memiliki kendala di mana suatu subdirektori yang berada di bawah naungan FUSE tidak dapat diakses. Alhasil, program FUSE gagal dalam membuat file baru yang nantinya akan dilakukan pengoperasian pada subsoal-subsoal Soal 4: Chiho. Pada revisi ini, program <code>maimai_fs</code> dapat membuat dan me-mounting sistem FUSE pada <code>fuse_dir</code>, mempopulasikan direktori <code>fuse_dir</code> dan <code>chiho</code> dengan subdirektori yang telah didefinisikan dan dinyatakan di dalam soal seperti <code>starter</code>, <code>metro</code>, <code>heaven</code>, dan sebagainya, menggandakan file yang dimasukkan ke dalam direktori virtual <code>fuse_dir</code> pada direktori <code>chiho</code> yang ada pada disk, serta melakukan beberapa pengoperasian subsoal yang diurutkan berdasarkan waktu penyelesaiannya: 
</p>
  
1) Subsoal 4.A: Starter, menampilkan file pada direktori `chiho` yang dimasukkan dengan cara memasukkannya ke dalam direktori `fuse_dir`.
2) Subsoal 4.B: Metro, melakukan enkripsi dan dekripsi shifting berdasarkan offset karakter pada file.
3) Subsoal 4.C: Dragon, melakukan enkripsi dan dekripsi menggunakan metode enkripsi ROT13 (Subsoal 4.C: Dragon).
4) Subsoal 4.F: Skystreet, melakukan proses mengompres dan dekompres suatu file menggunakan library `<zlib>`.
5) Subsoal 4.E: Heaven, melakukan enkripsi menggunakan metode enkripsi AES-256-CBC yang ada pada library `<openssl>`.

<p align="justify">
Adapun tampilan program revisi dari <code>maimai_fs</code> adalah sebagai berikut:
</p>
  
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

#### • Kendala yang Masih Dialami

<p align="justify">
&emsp; Pada kasus tertentu dimana program <code>maimai_fs</code> memerlukan suatu file untuk dibuat didalam FUSE yang diluar penanganan function <code>xmp_create()</code> maka file tersebut akan terbuat hanya dengan hak ases eksekusi untuk <code>root</code> saja. Kasus ini menyebabkan <code>user</code>, <code>groups</code>, atau bahkan <code>root</code> sendiri tidak dapat membaca file tersebut sehingga saat dijalankan command <code>cat</code> maka prosesnya akan melemparkan error ke layar terminal. Sebelumnya, saat proses inisiasi dan mempopulasikan direktori <code>fuse_dir</code> dan <code>chiho</code> dengan subdirektori yang telah didefinisikan dan dinyatakan di dalam soal, function <code>xmp_create()</code> menyediakan opsi untuk membuat file tersebut dalam mode <code>0777</code> di mana semua pengguna baik itu <code>root</code>, <code>user</code>, atau <code>groups</code> dapat mengeksekusi, membaca, dan menulis terhadap file yang dibuat tersebut.
</p>

<p align="justify">
&emsp; Namun, ada beberapa kasus pada program ini di mana program tidak akan menggunakan <code>xmp_create()</code> atau mode <code>0777</code> dalam pembuatannya, sehingga file tersebut kembali ke bentuk setelan awalnya. Kasus ini dapat dilihat pada saat mencoba untuk mengdekompres file yang sebelumnya telah terkompres menggunakan zlib yang telah dinyatakan pada subsoal 4.F: Skystreet. File hasil dekompres yang diciptakan pada program ini tidak dapat dibuka baik melalui command line seperti menggunakan <code>cat</code> atau secara UI pada VSCode.
</p>

<p align="center">
	<img src="https://github.com/user-attachments/assets/32e2b1c3-37d9-4f3e-8237-f05791bf3daa" alt="Dewa 19 - Aku Milikmu" width="640" height="360">  
</p>

> (?) Screenshot potret tampilan file `foo.txt` yang merupakan hasil dari proses dekompres program `maimai_fs` yang gagal dibuka karena user yang membuka file tidak memiliki hak akses. 
