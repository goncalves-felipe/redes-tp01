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

int conectar(char *enderecoIp, char *porta)
{
    int dominio, tamanhoEndereco, sockfd;

    struct sockaddr *endereco;
    struct sockaddr_in enderecov4;
    struct sockaddr_in6 enderecov6;

    enderecov4.sin_family = AF_INET;
    enderecov4.sin_port = htons(atoi(porta));

    enderecov6.sin6_family = AF_INET6;
    enderecov6.sin6_port = htons(atoi(porta));

    int eIpv4 = inet_pton(AF_INET, enderecoIp, &enderecov4.sin_addr);
    if (eIpv4 > 0)
    {
        dominio = AF_INET;
        tamanhoEndereco = sizeof(enderecov4);
        endereco = (struct sockaddr *)&enderecov4;
    }

    int eIpv6 = inet_pton(AF_INET6, enderecoIp, &enderecov6.sin6_addr);
    if (eIpv6 > 0)
    {
        dominio = AF_INET6;
        tamanhoEndereco = sizeof(enderecov6);
        endereco = (struct sockaddr *)&enderecov6;
    }

    sockfd = socket(dominio, SOCK_STREAM, IPPROTO_TCP);

    if (sockfd == 0)
    {
        perror("");
        exit(EXIT_FAILURE);
    }

    int conexao = connect(sockfd, endereco, tamanhoEndereco);

    if (conexao == -1)
    {
        printf("connection refused.");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

int validarFormatoArquivo(char *nomeArquivo)
{
    if (strstr(nomeArquivo, ".") == NULL)
    {
        printf("%s not valid!\n", nomeArquivo);
        return 0;
    }

    return 1;
}

int validarExtensaoArquivo(char *nomeArquivo)
{
    char copiaNome[TAMANHO_MAXIMO_MENSAGEM];
    strcpy(copiaNome, nomeArquivo);

    char *extensao = strrchr(copiaNome, '.');
    int extensaoValida = strcmp(extensao, ".txt") == 0 ||
                         strcmp(extensao, ".c") == 0 ||
                         strcmp(extensao, ".cpp") == 0 ||
                         strcmp(extensao, ".py") == 0 ||
                         strcmp(extensao, ".tex") == 0 ||
                         strcmp(extensao, ".java") == 0;

    if (!extensaoValida)
    {
        printf("%s not valid!\n", nomeArquivo);
        return 0;
    }

    return 1;
}

void selecionarArquivo(char *nomeArquivo)
{
    int formatoValido = validarFormatoArquivo(nomeArquivo);

    if (formatoValido == 0)
    {
        return;
    }

    int extensaoValida = validarExtensaoArquivo(nomeArquivo);

    if (extensaoValida == 0)
    {
        return;
    }

    FILE *arquivo = fopen(nomeArquivo, "r");

    if (arquivo == NULL)
    {
        printf("%s do not exist\n", nomeArquivo);
    }
}

int enviarArquivo() { return 0; }

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("missing arguments.");
        exit(EXIT_FAILURE);
    }

    // int sockfd = conectar(argv[1], argv[2]);

    char *comandoExit = "exit", *comandoSelect = "select file", *comandoSend = "send file";
    FILE *arquivo;

    for (;;)
    {
        char buffer[TAMANHO_MAXIMO_MENSAGEM];

        fflush(stdin);
        scanf("%[^\n]%*c", buffer);

        if (strcmp(buffer, comandoExit) == 0)
        {
            exit(1);
        }

        if (strstr(buffer, comandoSelect) != NULL)
        {
            char *nomeArquivo = strrchr(buffer, ' ');
            nomeArquivo++;
            selecionarArquivo(nomeArquivo);
        }

        if (strcmp(buffer, comandoSend) == 0)
        {
            enviarArquivo();
        }
    }

    return 1;
}