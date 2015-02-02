#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ne_session.h>
#include <ne_auth.h>
#include <ne_request.h>
#include <fcntl.h>
#include <sys/stat.h>

/*************************************************************************/
/* TYPEDEFS                                                              */
/*************************************************************************/

typedef struct
{
	char login[100];
    char password[100];
    char remote_dir[200];
    char local_dir[200];
} settings;

typedef struct
{
    char filename[200];
    char md5sum[33];
    char sha256sum[65];
    off_t length;
} y_file;    

/*************************************************************************/
/* GLOBAL VARIABLES                                                      */
/*************************************************************************/

settings yadd_config; 

/*************************************************************************/
/* FUNCTIONS                                                             */
/*************************************************************************/


static int define_auth(void *userdata,
                       const char *realm,
                       int attempts,
                       char *username,
                       char *password)
{
    strncpy(username, yadd_config.login, NE_ABUFSIZ);
    strncpy(password, yadd_config.password, NE_ABUFSIZ);
    printf("%s\n",realm);
    printf("Auth\n");
    return attempts;
}

off_t fsize(const char *filename)
{
    struct stat st;
    if (stat(filename, &st) == 0)
        return st.st_size;

    perror("Cannot determine size");
    return -1;
}

int send_file(ne_session *session, y_file *file_sp)
{
    char local_path[500];
    char remote_path[500];
    int res;

    strcpy(local_path, yadd_config.local_dir);
    strcat(local_path, file_sp->filename);

    strcpy(remote_path, yadd_config.remote_dir);
    strcat(remote_path, file_sp->filename);

    int fd = open(local_path, O_RDONLY);
    if (-1 == fd)
    {
        perror("open");
        return -1;
    }

    ne_request *req = ne_request_create(session, "PUT", remote_path);

    ne_set_request_body_fd(req, fd, 0, file_sp->length);
    ne_set_request_flag(req, NE_REQFLAG_EXPECT100,1);
    ne_add_request_header(req, "Etag", file_sp->md5sum);
    ne_add_request_header(req, "Sha256", file_sp->sha256sum);
    
    res = ne_request_dispatch(req);
    if (NE_OK != res)
    {
        printf("Request failed: %s\n", ne_get_error(session));
        ne_request_destroy(req);
        close(fd);
    }
    printf("Response status code was %s\n", ne_get_status(req)->reason_phrase);
    return res;
}

int main(int argc, char **argv)
{
    FILE *secret, *config;
    ne_session *yadd_session;

    int res = NE_OK;

    secret = fopen(".secret", "r");
    if (NULL == secret)
    {
        perror("Something was wrong with secret-file");
        exit(EXIT_FAILURE);
    }    
    
    config = fopen(".config", "r");
    if (NULL == config)
    {
        perror("Something was wrong with config-file");
        exit(EXIT_FAILURE);
    }    
    // %TODO: Rewrite this shit
    fscanf(secret, "%s", yadd_config.login);
    fscanf(secret, "%s", yadd_config.password);
    fscanf(config, "%s", yadd_config.remote_dir);
    fscanf(config, "%s", yadd_config.local_dir);
    
    ne_sock_init();

    yadd_session = ne_session_create("https","webdav.yandex.ru",443);
    ne_set_server_auth(yadd_session, define_auth, NULL);
    ne_ssl_trust_default_ca(yadd_session);

    y_file fl;
    strcpy(fl.filename,"42cc1b4bca1e.mp3");
    strcpy(fl.md5sum,"881416a02d11c92352f7874a8db2cc7f");
    strcpy(fl.sha256sum,"7f8f70736889884851fcd43e0c219c2482ee3f8f5a93337e874e7e34c478b15e");
    fl.length = fsize("/home/grayscale/Музыка/42cc1b4bca1e.mp3");

    send_file(yadd_session, &fl);
    
    printf("OK!\n");

    ne_session_destroy(yadd_session);
    ne_sock_exit();

    fclose(secret);
    fclose(config);
    return 0;
}
