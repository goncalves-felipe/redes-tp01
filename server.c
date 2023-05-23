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

int inicializarEnderecoSocket(const char *tipoIp, const char *stringPorta, struct sockaddr_storage *storage)
{
    uint16_t porta = (uint16_t)atoi(stringPorta);

    if (porta == 0)
        return -1;

    porta = htons(porta);
    memset(storage, 0, sizeof(*storage));

    if (strcmp(tipoIp, "v4") == 0)
    {
        struct sockaddr_in *enderecov4 = (struct sockaddr_in *)storage;
        enderecov4->sin_family = AF_INET;
        enderecov4->sin_port = porta;
        enderecov4->sin_addr.s_addr = INADDR_ANY;
        return 0;
    }

    if (strcmp(tipoIp, "v6") == 0)
    {
        struct sockaddr_in6 *enderecov6 = (struct sockaddr_in6 *)storage;
        enderecov6->sin6_family = AF_INET6;
        enderecov6->sin6_port = porta;
        enderecov6->sin6_addr = in6addr_any;
        return 0;
    }

    return -1;
}

int iniciarSocket(const char *tipoIp, const char *porta)
{
    struct sockaddr_storage storage;

    if (inicializarEnderecoSocket(tipoIp, porta, &storage) != 0)
    {
        printf("missing arguments.\n");
        exit(1);
    }

    int socketServidor = socket(storage.ss_family, SOCK_STREAM, 0);
    if (socketServidor == -1)
    {
        printf("error creating socket.\n");
        exit(1);
    }

    int habilitado = 1;
    if (setsockopt(socketServidor, SOL_SOCKET, SO_REUSEADDR, &habilitado, sizeof(int)) != 0)
    {
        printf("error setsockopt\n");
        exit(1);
    }

    struct sockaddr *endereco = (struct sockaddr *)(&storage);

    if (bind(socketServidor, endereco, sizeof(storage)) != 0)
    {
        printf("error bind\n");
        exit(1);
    }

    if (listen(socketServidor, 1) != 0)
    {
        printf("error listen\n");
        exit(1);
    }

    return socketServidor;
}

int conectarComClient(int socketServidor)
{
    struct sockaddr_storage storage;
    struct sockaddr *enderecoCliente = (struct sockaddr *)&storage;
    socklen_t tamanhoEnderecoCliente = sizeof(storage);
    int socketClient = accept(socketServidor, enderecoCliente, &tamanhoEnderecoCliente);

    if (socketClient == -1)
    {
        printf("error accept\n");
        exit(1);
    }

    return socketClient;
}

int verificarSeArquivoExiste(char *nomeArquivo)
{
    FILE *arquivo = fopen(nomeArquivo, "rb");
    return arquivo == NULL ? 0 : 1;
}

int salvarArquivo(char *caminhoArquivo, char *dados)
{
    FILE *arquivo;
    arquivo = fopen(caminhoArquivo, "wb");
    fprintf(arquivo, "%s", dados);
    fclose(arquivo);
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("missing arguments.\n");
        exit(1);
    }

    int socketServidor = iniciarSocket(argv[1], argv[2]);

    int conexao = conectarComClient(socketServidor);
    char nomeArquivo[TAMANHO_MAXIMO_MENSAGEM];
    char buffer[TAMANHO_MAXIMO_MENSAGEM];
    char messagemRetorno[TAMANHO_MAXIMO_MENSAGEM];

    for (;;)
    {
        char caminhoArquivoServidor[TAMANHO_MAXIMO_MENSAGEM] = "./serverdata/";

        read(conexao, buffer, sizeof(buffer));
        strcpy(nomeArquivo, buffer);

        int result = read(conexao, buffer, sizeof(buffer));

        if (result == -1)
        {
            char mensagemErro[TAMANHO_MAXIMO_MENSAGEM] = "error receiving file ";
            strcat(mensagemErro, nomeArquivo);
            strcat(mensagemErro, "\n");
            write(conexao, mensagemErro, sizeof(mensagemErro));
            continue;
        }

        if (buffer[0] == '\0' || (strcmp("exit", buffer) == 0))
            break;

        strcat(caminhoArquivoServidor, nomeArquivo);
        strcpy(messagemRetorno, "file ");
        strcat(messagemRetorno, nomeArquivo);
        strcat(messagemRetorno, verificarSeArquivoExiste(caminhoArquivoServidor) == 0 ? " received\n" : " overwritten\n");

        salvarArquivo(caminhoArquivoServidor, buffer);

        write(conexao, messagemRetorno, sizeof(messagemRetorno));
    }

    close(socketServidor);
    close(conexao);

    return 1;
}