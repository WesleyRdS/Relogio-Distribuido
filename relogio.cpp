#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cstring>
#include <cstdlib> // Para getenv
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;
using namespace chrono;

struct Dados_horario
{
    int horas;
    int minutos;
    int segundos;
};

class Relogio
{
private:
    int horas;
    int minutos;
    int segundos;
    int drift;
    int portaE;
    int portaR;
    int portaC;
    string ip_local;
    bool lider;
    vector<string> listaIP;

public:
    Relogio(int h = 0, int m = 0, int s = 0, int d = 1, bool l = true, vector<string> lista  = {"127.0.0.1"})
        :  horas(h), minutos(m), segundos(s), drift(d), lider(l), listaIP(lista){
            char* ip_ambiente = getenv("IP");
            char* porta_ambienteR = getenv("PR");
            char* porta_ambienteE = getenv("PE");
            if(ip_ambiente != nullptr && porta_ambienteE != nullptr && porta_ambienteR != nullptr){
                ip_local = ip_ambiente;
                portaE = atoi(porta_ambienteE);
                portaR = atoi(porta_ambienteR);
            }else{
                cerr << "Variável de ambiente IP não definida. Usando endereço padrão 127.0.0.1.\n";
                ip_local = "127.0.0.1";
                portaE = 9985;
                portaR = 8885;
            }
        }
    
    int getHoras(){
        return horas;
    }

    int getMinutos(){
        return minutos;
    }

    int getSegundos(){
        return segundos;
    }


    void setDrift(int valor){
        drift = valor;
    }

    void setHorario(int h, int m, int s){
        horas = h;
        minutos = m;
        segundos = s;
    }
    
    void engrenagens(){
        segundos += drift;
        if(segundos >= 60) {
            segundos = 0;
            minutos++;
            if(minutos >= 60){
                minutos = 0;
                horas++;
            }
        }
    }

    void display(){
        cout << horas << ":" << minutos << ":" << segundos << endl;
    }

    void enviarHorario() {
        if(lider == true){
            int socket_envio;
            struct sockaddr_in end_servidor;
            display();
            engrenagens();

            if((socket_envio = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
                cerr << "Erro ao criar Socket UDP" << "\n";
                return;
            }

            //Preenche o bloco de memória apontado com o valor 0 até o número de bytes especificado.
            memset((char *) &end_servidor, 0, sizeof(end_servidor));
            end_servidor.sin_family = AF_INET;
            end_servidor.sin_port = htons(portaE);
            for(vector<string>::const_iterator iterador = listaIP.begin(); iterador != listaIP.end(); ++iterador){
                if(inet_aton((*iterador).c_str(), &end_servidor.sin_addr) == 0 ){
                    cerr << "Endereço não conectado a rede de relogios" << (*iterador) << endl;
                    close(socket_envio);
                    return;
                }

                Dados_horario horario_atual = {horas, minutos, segundos};
                char buffer[sizeof(Dados_horario)];
                memcpy(buffer, &horario_atual, sizeof(Dados_horario));

                if(sendto(socket_envio, buffer, sizeof(buffer), 0, (struct sockaddr *) &end_servidor, sizeof(end_servidor)) == -1){
                    cerr << "Erros ao enviar horario para " << (*iterador) << endl;   
                } else{
                    cout << "Relogio enviando horario para " << (*iterador) << ": ";
                    display();
                }
            }
        }

    }

    void receberHorario() {
        int socket_receptor;
        struct sockaddr_in end_servidor, end_cliente;
        socklen_t end_cliente_size = sizeof(end_cliente);
        char buffer[sizeof(Dados_horario)];
        if(lider == false){
            display();
            engrenagens();
        }
        if((socket_receptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
            cerr << "Erro ao criar Socket UDP" << endl;
            return;
        }

        memset((char *) &end_servidor, 0, sizeof(end_servidor));
        end_servidor.sin_family = AF_INET;
        //configuração para aceitar conexão com qualquer endereço local
        end_servidor.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY tem valor 0
        end_servidor.sin_port = htons(portaR);

        if(bind(socket_receptor, (struct sockaddr *) &end_servidor, sizeof(end_servidor)) == -1){
            cerr << "Erro ao se conectar" << endl;
            close(socket_receptor);
            return;
        }

        if(recvfrom(socket_receptor, buffer, sizeof(buffer), 0, (struct sockaddr *) &end_cliente, &end_cliente_size) == 0){
            lider = true;
        }else{

            Dados_horario horario_recebido;
            memcpy(&horario_recebido, buffer, sizeof(Dados_horario));

            if(horario_recebido.horas > horas || (horario_recebido.horas == horas && horario_recebido.minutos > minutos) 
            || (horario_recebido.horas == horas, horario_recebido.minutos == minutos && horario_recebido.segundos > segundos)){
                lider = false;
                horas = horario_recebido.horas;
                minutos = horario_recebido.minutos;
                segundos = horario_recebido.segundos;
                cout << "Relogio atualizado com horario recebido: " << endl;
                display();
            }else{
                lider = true;
            }
        }

        close(socket_receptor);
    }


    void processarComando() {
        char* porta_ambienteC = getenv("PC");
        int porta = atoi(porta_ambienteC);
        int socket_receptor;
        struct sockaddr_in end_servidor, end_cliente;
        socklen_t end_cliente_size = sizeof(end_cliente);
        char buffer[26];

        if ((socket_receptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
            cerr << "Erro ao criar Socket UDP" << endl;
            return;
        }

        memset((char *) &end_servidor, 0, sizeof(end_servidor));
        end_servidor.sin_family = AF_INET;
        end_servidor.sin_addr.s_addr = htonl(INADDR_ANY);
        end_servidor.sin_port = htons(porta);

        if (bind(socket_receptor, (struct sockaddr *) &end_servidor, sizeof(end_servidor)) == -1) {
            cerr << "Erro ao se conectar" << endl;
            close(socket_receptor);
            return;
        }

        cout << "Aguardando comando UDP na porta " << porta << "..." << endl;

        while (true) {
            if (recvfrom(socket_receptor, buffer, sizeof(buffer), 0, (struct sockaddr *) &end_cliente, &end_cliente_size) == -1) {
                cerr << "Erro ao receber comando UDP" << endl;
            } else {
                char tipoComando[20];
                int valor1, valor2, valor3;

                cout << "Comando recebido via UDP: " << buffer << endl;

                if ((sscanf(buffer, "%s %d %d %d", tipoComando, &valor1, &valor2, &valor3) == 4) || sscanf(buffer, "%s %d", tipoComando, &valor1) == 2) {
                    if (strcmp(tipoComando, "SET_DRIFT") == 0) {
                        setDrift(valor1);  
                    } else if (strcmp(tipoComando, "SET_HORARIO") == 0) {
                        setHorario(valor1, valor2, valor3);  
                    } else {
                        cout << "Comando inválido." << endl;
                    }
                } else {
                    cout << "Formato de comando inválido." << endl;
                }
                cout << "Comando processado: " << buffer << endl;
            }
        }

        close(socket_receptor);
    }



};


int main(){
    Relogio sincronismo;

    thread enviar_thread([&sincronismo](){
        while(true){
            sincronismo.enviarHorario();
            this_thread::sleep_for(seconds(1));
        }
    });

    thread receber_thread([&sincronismo](){
        while(true){
            sincronismo.receberHorario();
        }
    });

    thread comando_thread([&sincronismo](){
        while (true)
        {
            sincronismo.processarComando();
        }
        
    });
    
    enviar_thread.join();
    receber_thread.join();
    comando_thread.join();
    return 0;
}



