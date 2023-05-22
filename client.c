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

#define TAMANHO_MAXIMO_MENSAGEM 500

int criarSocket(char *enderecoIp, char *porta)
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

    sockfd = socket(endereco->sa_family, SOCK_STREAM, 0);

    if (sockfd == -1)
    {
        printf("socket error.");
        exit(1);
    }

    int conexao = connect(sockfd, endereco, tamanhoEndereco);

    if (conexao == -1)
    {
        printf("connection refused.");
        exit(1);
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

FILE *selecionarArquivo(char *nomeArquivo)
{
    int formatoValido = validarFormatoArquivo(nomeArquivo);

    if (formatoValido == 0)
    {
        return NULL;
    }

    int extensaoValida = validarExtensaoArquivo(nomeArquivo);

    if (extensaoValida == 0)
    {
        return NULL;
    }

    FILE *arquivo = fopen(nomeArquivo, "r");

    if (arquivo == NULL)
    {
        printf("%s do not exist\n", nomeArquivo);
        return NULL;
    }

    printf("%s selected\n", nomeArquivo);
    return arquivo;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("missing arguments.\n");
        exit(1);
    }

    int sockfd = criarSocket(argv[1], argv[2]);

    char *comandoExit = "exit", *comandoSelect = "select file", *comandoSend = "send file";
    FILE *arquivo = NULL;
    char *nomeArquivoRecebido;
    char *nomeArquivo;
    char conteudoArquivo[TAMANHO_MAXIMO_MENSAGEM];

    for (;;)
    {
        char buffer[TAMANHO_MAXIMO_MENSAGEM];

        fflush(stdin);
        scanf("%[^\n]%*c", buffer);

        if (strcmp(buffer, comandoExit) == 0)
        {
            write(sockfd, "exit", sizeof("exit"));
            close(sockfd);
            printf("connextion closed\n");
            exit(0);
        }

        if (strstr(buffer, comandoSelect) != NULL)
        {
            nomeArquivoRecebido = strrchr(buffer, ' ');
            nomeArquivoRecebido++;
            strcpy(nomeArquivo, nomeArquivoRecebido);
            strcpy(conteudoArquivo, "");
            arquivo = selecionarArquivo(nomeArquivoRecebido);
        }
        else if (strstr(buffer, comandoSend) != NULL)
        {
            if (arquivo == NULL)
            {
                printf("no file selected\n");
            }
            else
            {
                char mensagem[100];

                write(sockfd, nomeArquivo, sizeof(nomeArquivo) + 10);
                while (fgets(mensagem, 100, arquivo))
                {
                    strcat(conteudoArquivo, mensagem);
                }

                write(sockfd, conteudoArquivo, sizeof(conteudoArquivo));

                bzero(buffer, sizeof(buffer));

                read(sockfd, buffer, TAMANHO_MAXIMO_MENSAGEM);
                printf("%s", buffer);
            }
        }
        else
        {
            close(sockfd);
            exit(0);
        }
    }

    close(sockfd);

    return 1;
}