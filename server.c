#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// TODO: validar
#define TAMANHO_MAXIMO_MENSAGEM 500

int conectar(const char *tipoIp, const char *porta, struct sockaddr **endereco)
{
    int dominio, tamanhoEndereco, serverfd;
    struct sockaddr_in enderecov4;
    struct sockaddr_in6 enderecov6;

    enderecov4.sin_family = AF_INET;
    enderecov4.sin_port = htons(atoi(porta));
    enderecov4.sin_addr.s_addr = htonl(INADDR_ANY);

    enderecov6.sin6_family = AF_INET6;
    enderecov6.sin6_port = htons(atoi(porta));
    enderecov6.sin6_addr = in6addr_any;

    if (strcmp(tipoIp, "v4") == 0)
    {
        dominio = AF_INET;
        tamanhoEndereco = sizeof(enderecov4);
        *endereco = (struct sockaddr *)&enderecov4;
    }
    else if (strcmp(tipoIp, "v6") == 0)
    {
        dominio = AF_INET6;
        tamanhoEndereco = sizeof(enderecov6);
        *endereco = (struct sockaddr *)&enderecov6;
    }

    serverfd = socket(dominio, SOCK_STREAM, IPPROTO_TCP);
    if (serverfd == 0)
    {
        printf("error creating socket.\n");
        exit(1);
    }

    int enable = 1;
    if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) != 0)
    {
        printf("error setsockopt.\n");
        exit(1);
    }

    if (bind(serverfd, *endereco, tamanhoEndereco) < 0)
    {
        printf("Could not bind port to socket\n");
        exit(1);
    }

    if (listen(serverfd, 5) < 0)
    {
        printf("Could not listen on designated address\n");
        exit(1);
    }

    return serverfd;
}

int aceitarNovaConexao(int sockfd, struct sockaddr *endereco)
{
    int connfd;
    int addrSize = sizeof(endereco);

    if ((connfd = accept(sockfd, (struct sockaddr *)&endereco, &addrSize)) < 0)
    {
        printf("Could not accept new connections in this server");
        exit(EXIT_FAILURE);
    }

    printf("connfd: %i", connfd);

    return connfd;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("missing arguments.\n");
        exit(1);
    }

    struct sockaddr *endereco;

    int sockfd = conectar(argv[1], argv[2], &endereco);

    char *nomeArquivo;
    int conexao = aceitarNovaConexao(sockfd, endereco);

    // for (;;)
    // {
    //     char buffer[TAMANHO_MAXIMO_MENSAGEM];

    //     fflush(stdin);
    //     read(conexao, buffer, sizeof(buffer));

    //     printf("%s", buffer);

    //     // if (strcmp(buffer, "exit") == 0)
    //     // {
    //     //     printf("connection closed\n");
    //     //     exit(0);
    //     // }

    //     // strcpy(nomeArquivo, buffer);

    //     // fflush(stdin);
    //     // read(conexao, buffer, sizeof(buffer));

    //     // if (buffer[0] == '\0')
    //     // {
    //     //     break;
    //     // }
    // }

    return 1;
}