/* Lab. Redes II - Prof. Fernando W. Cruz */
/* web80.c :Este eh um Web Server extremamente simples: */
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/uio.h>

#define TRUE 1

char **split_string(const char *str, const char *delimiter, int *token_count)
{
    char copy_str[100];
    strcpy(copy_str, str);

    char *token = strtok(copy_str, delimiter);
    int count = 0;

    while (token != NULL)
    {
        count++;
        token = strtok(NULL, delimiter);
    }

    char **tokens = malloc(count * sizeof(char *));
    if (tokens == NULL)
    {
        perror("Memory allocation failed");
        return NULL;
    }

    strcpy(copy_str, str);

    token = strtok(copy_str, delimiter);
    int i = 0;
    while (token != NULL)
    {
        tokens[i] = malloc((strlen(token) + 1) * sizeof(char));
        strcpy(tokens[i], token);
        i++;
        token = strtok(NULL, delimiter);
    }

    *token_count = count;
    return tokens;
}

void free_tokens(char **tokens, int token_count)
{
    for (int i = 0; i < token_count; i++)
    {
        free(tokens[i]);
    }
    free(tokens);
}

int main(int argc, char **argv)
{
    int z, sd;                           /* Descritor de Socket do Web Server     */
    int novo_sd;                         /* Descritor com dados do cliente        */
    int alen;                            /* Tamanho do Endereco                   */
    struct sockaddr_in end_web, end_cli; /* End.do Web Server e do Cliente*/
    int b = TRUE;                        /* Reutilizacao do ender.SO_REUSEADDR    */
    FILE *rx, *tx;                       /* Stream de Leitura e Escrita           */
    char getbuf[2048];                   /* GET buffer                            */
    time_t td;                           /* Data e hora corrente                  */

    sd = socket(AF_INET, SOCK_STREAM, 0);
    /* Web address on port 80: */
    memset(&end_web, 0, sizeof end_web);
    end_web.sin_family = AF_INET;
    end_web.sin_port = ntohs(80);
    end_web.sin_addr.s_addr = ntohl(INADDR_ANY);
    //  end_web.sin_addr.s_addr = inet_addr(argv[1]);
    //  end_web.sin_port = atoi(argv[2]);
    z = bind(sd, (struct sockaddr *)&end_web, sizeof end_web);
    /* Ativa a opcao SO_REUSEADDR :  */
    z = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &b, sizeof b);
    z = listen(sd, 10);
    /* Recebe uma msg e retorna um arquivo html */
    for (;;)
    {
        /* Wait for a connect from browser: */
        alen = sizeof end_cli;
        novo_sd = accept(sd, (struct sockaddr *)&end_cli, &alen);
        rx = fdopen(novo_sd, "r");      /* cria stream de leitura associada a novo_sd*/
        tx = fdopen(dup(novo_sd), "w"); /* cria stream de escrita associada a novo_sd */
        fgets(getbuf, sizeof getbuf, rx);
        printf("Msg de chegada = %s\n", getbuf);
        int token_count = 0;
        char **tokens = split_string(getbuf, " ", &token_count);
        char *path;
        if (tokens != NULL)
        {
            path = tokens[1];

            if (strcmp(path, "/favicon.ico") == 0)
            {
                continue;
            }
            if (strcmp(path, "/") == 0)
            {
                path = "/index";
            }
        }

        /* Resposta com o conteúdo do arquivo HTML */
        char temp[100];

        strcpy(temp, "html");
        strcat(temp, path);
        strcat(temp, ".html");

        printf("Path = %s\n", temp);

        FILE *html_file = fopen(temp, "r");
        free_tokens(tokens, token_count);

        if (html_file != NULL)
        {
            char buffer[2048];
            while (fgets(buffer, sizeof buffer, html_file) != NULL)
            {
                fputs(buffer, tx);
            }
            fclose(html_file);
        }
        else
        {
            char *not_found_response = "HTTP/1.1 404 Not Found\r\n\r\n"
                                       "<html><head><title>404 Not Found</title></head>"
                                       "<body><h1>404 Not Found</h1></body></html>";
            fputs(not_found_response, tx);
        }
        fclose(tx);
        fclose(rx);
    } /* fim-for */
    return 0;
} /* fim-main */