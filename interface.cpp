#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

void enviarComando(const char *ip_destino, int porta_destino, const char *comando) {
    int socket_envio;
    struct sockaddr_in end_destino;

    if ((socket_envio = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        cerr << "Erro ao criar Socket UDP" << endl;
        return;
    }

    memset((char *) &end_destino, 0, sizeof(end_destino));
    end_destino.sin_family = AF_INET;
    end_destino.sin_port = htons(porta_destino);

    if (inet_aton(ip_destino, &end_destino.sin_addr) == 0) {
        cerr << "Endereço de destino inválido." << endl;
        close(socket_envio);
        return;
    }

    if (sendto(socket_envio, comando, strlen(comando), 0, (struct sockaddr *) &end_destino, sizeof(end_destino)) == -1) {
        cerr << "Erro ao enviar comando para o relógio." << endl;
    } else {
        cout << "Comando enviado com sucesso para o relógio." << endl;
    }

    close(socket_envio);
}

int main() {
    char ip_destino[50];
    int porta_destino;
    char comando[100];

    while (true) {
        cout << "Digite o endereço IP do relógio: ";
        cin.getline(ip_destino, sizeof(ip_destino));

        cout << "Digite a porta de destino do relógio: ";
        cin >> porta_destino;

        cin.ignore(); // Limpar o buffer de entrada

        cout << "Digite o comando a ser enviado (ex. SET_DRIFT 5): ";
        cin.getline(comando, sizeof(comando));

        enviarComando(ip_destino, porta_destino, comando);
    }

    return 0;
}
